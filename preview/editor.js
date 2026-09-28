// Editor de bichinhos: desenho 8×8, animações, validação, arquivos e envio
// pela USB (Web Serial) no protocolo de src/CustomPet.h.
(() => {
  'use strict';
  const A = window.ART, P = window.PetPack, N = 8;
  const KEY = 'pixelgotchi.project';
  const $ = id => document.getElementById(id);
  const ANIM_LABEL = { idle: L('Descanso', 'Idle'), blink: L('Piscar', 'Blink'), walk: L('Andar', 'Walk'), eat: L('Comer', 'Eat'),
    sleep: L('Dormir', 'Sleep'), happy: L('Feliz', 'Happy'), sad: L('Triste', 'Sad'), hungry: L('Com fome', 'Hungry'), tired: L('Cansado', 'Tired') };
  const FOOD_LABEL = { food_melon: L('Melancia', 'Watermelon'), food_fish: L('Peixe', 'Fish'), food_fly: L('Mosquinha', 'Fly'),
    food_worm: L('Minhoca', 'Worm'), food_carrot: L('Cenoura', 'Carrot'), food_shrimp: L('Camarão', 'Shrimp') };
  const PET_LABEL = { capy: L('Capivara', 'Capybara'), cat: L('Gato', 'Cat'), frog: L('Sapo', 'Frog'), chick: L('Pintinho', 'Chick'),
    bunny: L('Coelho', 'Bunny'), axo: L('Axolote', 'Axolotl') };
  const frameLetter = i => i < 26 ? String.fromCharCode(65 + i) : 'F' + (i + 1);

  let project, anim = 'idle', step = 0, color = 1, tool = 'pen';
  let undoStack = [], redoStack = [];

  // ---------------------------------------------------------------- estado
  function normalize(p) {
    const base = P.blankProject();
    const out = Object.assign(base, p);
    out.colors = (p.colors || base.colors).slice(0, P.LIMITS.colors).map(c => String(c).toUpperCase());
    out.frames = (p.frames && p.frames.length ? p.frames : base.frames).map(f => ({ px: Array.from({ length: 64 }, (_, i) => Math.max(0, Math.min(out.colors.length, f.px[i] | 0))) }));
    for (const a of P.ANIMS) {
      const an = (p.anims || {})[a] || base.anims[a];
      out.anims[a] = { ms: Math.max(P.LIMITS.msMin, Math.min(P.LIMITS.msMax, an.ms | 0 || 450)), frames: an.frames.filter(f => f >= 0 && f < out.frames.length) };
      if (!out.anims[a].frames.length) out.anims[a].frames = [0];
    }
    if (!P.FOODS.includes(out.food)) out.food = 'food_fish';
    return out;
  }
  function load() {
    try {
      const saved = JSON.parse(localStorage.getItem(KEY));
      if (saved && saved.format === 'pixelgotchi-pet') return normalize(saved);
    } catch (_) {}
    const p = P.fromArt(A, 'cat');
    p.name = L('Meu gato', 'My cat');
    return p;
  }
  function persist() { try { localStorage.setItem(KEY, JSON.stringify(project)); } catch (_) {} }
  function snapshot() {
    undoStack.push(JSON.stringify(project));
    if (undoStack.length > 100) undoStack.shift();
    redoStack = [];
  }
  function change(fn) { snapshot(); fn(); persist(); render(); }
  const frameIndex = () => project.anims[anim].frames[step];
  const frame = () => project.frames[frameIndex()];

  // Remove frames que nenhuma animação usa e renumera as referências.
  function collectGarbage() {
    const used = new Set(P.ANIMS.flatMap(a => project.anims[a].frames));
    const map = new Map(); const frames = [];
    project.frames.forEach((f, i) => { if (used.has(i)) { map.set(i, frames.length); frames.push(f); } });
    project.frames = frames;
    for (const a of P.ANIMS) project.anims[a].frames = project.anims[a].frames.map(f => map.get(f));
  }

  // ---------------------------------------------------------------- cores e LEDs
  const hexRgb = h => [1, 3, 5].map(i => parseInt(h.slice(i, i + 2), 16));
  const colorOf = idx => idx ? hexRgb(project.colors[idx - 1]) : null;
  // Mesma aproximação do modo LED do simulador (LedProfile::color + brilho do perfil).
  function ledColor(rgb) {
    const LP = A.ledProfile;
    let [r, g, b] = [LP.gammaLut[rgb[0]], LP.gammaLut[rgb[1]], LP.gammaLut[rgb[2]] * Math.round(LP.blueGain * 255) / 255];
    const sum = r + g + b;
    if (sum > LP.glareCap) { const k = LP.glareCap / sum; r *= k; g *= k; b *= k; }
    const peak = Math.max(r, g, b), floorV = Math.floor((LP.minPeak * 256 + LP.brightness) / (LP.brightness + 1));
    if (peak > 0 && peak < floorV) { const k = floorV / peak; r *= k; g *= k; b *= k; }
    const out = [r, g, b].map(v => Math.floor(Math.min(255, Math.floor(v + 0.5)) * (LP.brightness + 1) / 256));
    if (out.every(v => v === 0)) return null;
    return out.map(v => Math.round(255 * Math.pow(Math.min(1, v / LP.brightness), 0.55)));
  }

  // ---------------------------------------------------------------- desenho da grade
  const grid = $('grid'), gctx = grid.getContext('2d');
  function drawGrid() {
    const cell = grid.width / N, px = frame().px;
    gctx.fillStyle = '#050506'; gctx.fillRect(0, 0, grid.width, grid.height);
    const c = P.crop(project);
    for (let i = 0; i < 64; i++) {
      const x = i % N, y = Math.floor(i / N), rgb = colorOf(px[i]);
      gctx.fillStyle = rgb ? `rgb(${rgb})` : ((x + y) & 1 ? '#15171b' : '#1b1d22');
      gctx.fillRect(x * cell + 1, y * cell + 1, cell - 2, cell - 2);
    }
    if (c) { // colunas que o firmware usa: todas as poses dividem essa largura
      gctx.strokeStyle = 'rgba(198,243,107,.55)'; gctx.setLineDash([6, 5]); gctx.lineWidth = 2;
      gctx.strokeRect(c.minX * cell + 1, 1, (c.maxX - c.minX + 1) * cell - 2, grid.height - 2);
      gctx.setLineDash([]);
    }
  }
  function drawThumb(canvas, px) {
    const ctx = canvas.getContext('2d'), s = canvas.width / N;
    ctx.fillStyle = '#050506'; ctx.fillRect(0, 0, canvas.width, canvas.height);
    for (let i = 0; i < 64; i++) {
      const rgb = colorOf(px[i]);
      if (rgb) { ctx.fillStyle = `rgb(${rgb})`; ctx.fillRect((i % N) * s, Math.floor(i / N) * s, s, s); }
    }
  }

  function cellAt(e) {
    const r = grid.getBoundingClientRect();
    const x = Math.floor((e.clientX - r.left) / r.width * N), y = Math.floor((e.clientY - r.top) / r.height * N);
    return x >= 0 && x < N && y >= 0 && y < N ? [x, y] : null;
  }
  function fill(px, x, y, value) {
    const target = px[y * N + x];
    if (target === value) return;
    const stack = [[x, y]];
    while (stack.length) {
      const [cx, cy] = stack.pop();
      if (cx < 0 || cx >= N || cy < 0 || cy >= N || px[cy * N + cx] !== target) continue;
      px[cy * N + cx] = value;
      stack.push([cx + 1, cy], [cx - 1, cy], [cx, cy + 1], [cx, cy - 1]);
    }
  }
  let painting = null;
  grid.addEventListener('contextmenu', e => e.preventDefault());
  grid.addEventListener('pointerdown', e => {
    const at = cellAt(e); if (!at) return;
    e.preventDefault(); grid.setPointerCapture(e.pointerId);
    snapshot();
    const value = e.button === 2 ? 0 : color;
    if (tool === 'fill') { fill(frame().px, at[0], at[1], value); persist(); render(); return; }
    painting = value;
    frame().px[at[1] * N + at[0]] = value; drawGrid();
  });
  grid.addEventListener('pointermove', e => {
    if (painting === null) return;
    const at = cellAt(e); if (!at) return;
    const i = at[1] * N + at[0];
    if (frame().px[i] !== painting) { frame().px[i] = painting; drawGrid(); }
  });
  const endStroke = () => { if (painting === null) return; painting = null; persist(); render(); };
  grid.addEventListener('pointerup', endStroke);
  grid.addEventListener('pointercancel', endStroke);

  // ---------------------------------------------------------------- painéis
  function renderPalette() {
    const box = $('palette'); box.innerHTML = '';
    const make = (idx, label) => {
      const b = document.createElement('button');
      b.className = 'sw' + (idx ? '' : ' erase');
      if (idx) b.style.background = project.colors[idx - 1];
      b.title = label; b.setAttribute('aria-label', label);
      b.setAttribute('aria-pressed', String(idx === color));
      b.onclick = () => { color = idx; render(); };
      box.append(b);
    };
    make(0, L('Borracha (LED apagado)', 'Eraser (LED off)'));
    project.colors.forEach((c, i) => make(i + 1, L('Cor ', 'Color ') + (i + 1) + ' ' + c));
    $('color-edit').disabled = color === 0;
    if (color) $('color-edit').value = project.colors[color - 1].toLowerCase();
    $('add-color').disabled = project.colors.length >= P.LIMITS.colors;
    $('remove-color').disabled = color === 0 || project.colors.length <= 1;
  }
  function renderAnims() {
    const box = $('anims'); box.innerHTML = '';
    for (const a of P.ANIMS) {
      const b = document.createElement('button');
      b.textContent = ANIM_LABEL[a];
      b.setAttribute('aria-pressed', String(a === anim));
      b.onclick = () => { anim = a; step = 0; render(); };
      box.append(b);
    }
    const tl = $('timeline'); tl.innerHTML = '';
    project.anims[anim].frames.forEach((f, i) => {
      const b = document.createElement('button');
      b.className = 'step';
      b.setAttribute('aria-pressed', String(i === step));
      b.setAttribute('aria-label', L(`Passo ${i + 1}: frame ${frameLetter(f)}`, `Step ${i + 1}: frame ${frameLetter(f)}`));
      const cv = document.createElement('canvas'); cv.width = cv.height = 44;
      drawThumb(cv, project.frames[f].px);
      b.append(cv);
      b.onclick = () => { step = i; render(); };
      tl.append(b);
    });
    const uses = P.ANIMS.map(a => [a, project.anims[a].frames.filter(f => f === frameIndex()).length]).filter(([, n]) => n);
    $('frame-use').textContent = L('Frame ', 'Frame ') + frameLetter(frameIndex()) + ' · ' + L('usado em: ', 'used in: ') +
      uses.map(([a, n]) => ANIM_LABEL[a] + (n > 1 ? ' ×' + n : '')).join(', ') +
      (uses.length > 1 || uses[0][1] > 1 ? L(' — editar muda todos esses lugares.', ' — editing changes all of them.') : '');
    $('frame-title').textContent = ANIM_LABEL[anim] + ' · ' + L('passo ', 'step ') + (step + 1) + '/' + project.anims[anim].frames.length;
    $('ms').value = project.anims[anim].ms;
    const n = project.anims[anim].frames.length;
    $('step-left').disabled = step === 0;
    $('step-right').disabled = step >= n - 1;
    $('step-remove').disabled = n <= 1;
    $('step-new').disabled = $('step-repeat').disabled = n >= P.LIMITS.animFrames;
  }
  const MESSAGES = {
    colors_min: () => L('Adicione ao menos uma cor.', 'Add at least one color.'),
    colors_max: e => L(`Use no máximo ${e.n} cores.`, `Use at most ${e.n} colors.`),
    color_format: () => L('Há uma cor inválida na paleta.', 'There is an invalid color in the palette.'),
    name_long: e => L(`O nome pode ter até ${e.n} letras.`, `The name can have up to ${e.n} letters.`),
    name_chars: () => L('No nome, use letras sem acento, números e espaço (a placa não tem acentos).', 'Use plain letters, digits and spaces in the name.'),
    food: () => L('Escolha uma comida.', 'Choose a food.'),
    anim_empty: e => L(`A animação "${ANIM_LABEL[e.anim]}" está vazia.`, `The "${ANIM_LABEL[e.anim]}" animation is empty.`),
    anim_long: e => L(`"${ANIM_LABEL[e.anim]}" tem mais de ${e.n} passos.`, `"${ANIM_LABEL[e.anim]}" has more than ${e.n} steps.`),
    anim_ms: e => L(`O tempo de "${ANIM_LABEL[e.anim]}" deve ficar entre ${e.min} e ${e.max} ms.`, `"${ANIM_LABEL[e.anim]}" time must be between ${e.min} and ${e.max} ms.`),
    anim_ref: () => L('Uma animação aponta para um frame que não existe.', 'An animation points to a missing frame.'),
    frame_color: () => L('Um frame usa uma cor removida.', 'A frame uses a removed color.'),
    empty: () => L('Desenhe o bichinho: todos os frames estão vazios.', 'Draw your pet: every frame is empty.'),
    frames_max: e => L(`Use no máximo ${e.n} frames diferentes.`, `Use at most ${e.n} different frames.`),
    bytes: e => L(`O bichinho passou de ${e.n} bytes; use menos frames.`, `The pet is over ${e.n} bytes; use fewer frames.`),
    encode: e => e.message,
    wide: () => L('Com 8 colunas de largura ele não consegue andar pela tela.', 'At 8 columns wide it cannot walk around the screen.'),
    eat_same: () => L('"Comer" é igual a "Descanso": a comida aparece, mas não dá para ver a mastigação.', '"Eat" is the same as "Idle": the food shows up but chewing is not visible.'),
    sleep_tall: () => L('Dormindo com mais de 5 linhas sobra pouco espaço para os Zzz e a bolha do sonho.', 'Sleeping taller than 5 rows leaves little room for the Zzz and the dream bubble.'),
  };
  let validation = { errors: [], warnings: [] };
  function renderIssues() {
    validation = P.validate(project);
    const ul = $('issues'); ul.innerHTML = '';
    const add = (cls, text) => { const li = document.createElement('li'); li.className = cls; li.textContent = text; ul.append(li); };
    validation.errors.forEach(e => add('err', (MESSAGES[e.code] || (() => e.code))(e)));
    validation.warnings.forEach(w => add('warn', MESSAGES[w.code](w)));
    if (!validation.errors.length) {
      const size = P.encode(project).length;
      add('ok', L(`Pronto para enviar: ${size} de ${P.LIMITS.bytes} bytes.`, `Ready to send: ${size} of ${P.LIMITS.bytes} bytes.`));
    }
    updateSend();
  }
  function render() {
    drawGrid(); renderPalette(); renderAnims(); renderIssues();
    $('name').value = project.name || '';
    $('food').value = project.food;
    $('side').checked = !!project.side;
    $('undo').disabled = !undoStack.length;
    $('redo').disabled = !redoStack.length;
  }

  // ---------------------------------------------------------------- prévia animada
  const pv = $('preview'), pctx = pv.getContext('2d');
  function drawPreview(now) {
    const c = P.crop(project), an = project.anims[anim];
    const cell = pv.width / N;
    pctx.fillStyle = '#050506'; pctx.fillRect(0, 0, pv.width, pv.height);
    const buf = new Array(64).fill(null);
    if (c) {
      const fr = c.frames.get(an.frames[Math.floor(now / an.ms) % an.frames.length]);
      const left = Math.floor((N - fr.w) / 2), top = N - fr.h; // ancorado como no jogo
      fr.px.forEach((v, i) => { if (v) buf[(top + Math.floor(i / fr.w)) * N + left + (i % fr.w)] = colorOf(v); });
    }
    buf.forEach((rgb, i) => {
      const led = rgb && ledColor(rgb), x = (i % N) * cell, y = Math.floor(i / N) * cell;
      pctx.fillStyle = led ? `rgb(${led})` : '#1b1b1f';
      pctx.beginPath(); pctx.roundRect(x + 2, y + 2, cell - 4, cell - 4, 3); pctx.fill();
    });
    requestAnimationFrame(drawPreview);
  }

  // ---------------------------------------------------------------- controles
  function setup() {
    const tpl = $('template');
    A.pets.forEach(p => tpl.add(new Option(PET_LABEL[p.id] || p.name, p.id)));
    tpl.add(new Option(L('Em branco', 'Blank'), ''));
    P.FOODS.forEach(f => $('food').add(new Option(FOOD_LABEL[f], f)));
    $('use-template').onclick = () => {
      if (!confirm(L('Substituir o projeto atual pelo modelo? Você pode desfazer depois.', 'Replace the current project with this template? You can undo it.'))) return;
      change(() => {
        const id = tpl.value;
        project = id ? P.fromArt(A, id) : P.blankProject();
        project.name = id ? (PET_LABEL[id] || id).slice(0, P.LIMITS.name) : L('Meu pet', 'My pet');
        anim = 'idle'; step = 0; color = 1;
      });
    };
    $('name').addEventListener('input', e => { project.name = e.target.value; persist(); renderIssues(); });
    $('name').addEventListener('focus', snapshot);
    $('food').onchange = e => change(() => { project.food = e.target.value; });
    $('side').onchange = e => change(() => { project.side = e.target.checked; });
    $('color-edit').addEventListener('focus', snapshot);
    $('color-edit').addEventListener('input', e => { if (color) { project.colors[color - 1] = e.target.value.toUpperCase(); persist(); drawGrid(); renderPalette(); renderAnims(); } });
    $('color-edit').addEventListener('change', renderIssues);
    $('add-color').onclick = () => change(() => { project.colors.push('#FFFFFF'); color = project.colors.length; });
    $('remove-color').onclick = () => {
      const used = project.frames.some(f => f.px.includes(color));
      if (used && !confirm(L('Essa cor está em uso. Os pixels dela ficam apagados. Continuar?', 'This color is in use. Its pixels will be cleared. Continue?'))) return;
      change(() => {
        project.frames.forEach(f => { f.px = f.px.map(v => v === color ? 0 : v > color ? v - 1 : v); });
        project.colors.splice(color - 1, 1);
        color = Math.min(color, project.colors.length);
      });
    };
    const setTool = t => { tool = t; $('tool-pen').setAttribute('aria-pressed', String(t === 'pen')); $('tool-fill').setAttribute('aria-pressed', String(t === 'fill')); };
    $('tool-pen').onclick = () => setTool('pen');
    $('tool-fill').onclick = () => setTool('fill');
    $('flip').onclick = () => change(() => {
      const c = P.crop(project), lo = c ? c.minX : 0, hi = c ? c.maxX : N - 1, px = frame().px, out = px.slice();
      for (let y = 0; y < N; y++) for (let x = lo; x <= hi; x++) out[y * N + x] = px[y * N + (lo + hi - x)];
      frame().px = out;
    });
    const shift = (dx, dy) => {
      const px = frame().px;
      for (let i = 0; i < 64; i++) {
        const x = i % N + dx, y = Math.floor(i / N) + dy;
        if (px[i] && (x < 0 || x >= N || y < 0 || y >= N)) return; // não deixa o desenho sair da grade
      }
      change(() => {
        const out = new Array(64).fill(0);
        px.forEach((v, i) => { if (v) out[(Math.floor(i / N) + dy) * N + i % N + dx] = v; });
        frame().px = out;
      });
    };
    $('shift-left').onclick = () => shift(-1, 0);
    $('shift-right').onclick = () => shift(1, 0);
    $('shift-up').onclick = () => shift(0, -1);
    $('shift-down').onclick = () => shift(0, 1);
    $('clear').onclick = () => change(() => { frame().px = new Array(64).fill(0); });
    $('step-new').onclick = () => change(() => {
      project.frames.push({ px: frame().px.slice() });
      project.anims[anim].frames.splice(step + 1, 0, project.frames.length - 1); step++;
    });
    $('step-repeat').onclick = () => change(() => { project.anims[anim].frames.splice(step + 1, 0, frameIndex()); step++; });
    const move = d => change(() => { const f = project.anims[anim].frames; [f[step], f[step + d]] = [f[step + d], f[step]]; step += d; });
    $('step-left').onclick = () => move(-1);
    $('step-right').onclick = () => move(1);
    $('step-remove').onclick = () => change(() => {
      project.anims[anim].frames.splice(step, 1);
      step = Math.min(step, project.anims[anim].frames.length - 1);
      collectGarbage();
    });
    $('ms').onchange = e => change(() => { project.anims[anim].ms = Math.max(P.LIMITS.msMin, Math.min(P.LIMITS.msMax, +e.target.value || 450)); });
    $('undo').onclick = () => { if (!undoStack.length) return; redoStack.push(JSON.stringify(project)); project = JSON.parse(undoStack.pop()); clampView(); persist(); render(); };
    $('redo').onclick = () => { if (!redoStack.length) return; undoStack.push(JSON.stringify(project)); project = JSON.parse(redoStack.pop()); clampView(); persist(); render(); };
    document.addEventListener('keydown', e => {
      if (['INPUT', 'SELECT', 'TEXTAREA'].includes(e.target.tagName)) return;
      if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'z') { e.preventDefault(); (e.shiftKey ? $('redo') : $('undo')).click(); }
      if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'y') { e.preventDefault(); $('redo').click(); }
    });
    const slug = () => (project.name || 'meupet').normalize('NFD').replace(/[^a-zA-Z]/g, '').toLowerCase().slice(0, 12) || 'meupet';
    const download = (name, text, type) => {
      const a = document.createElement('a');
      a.href = URL.createObjectURL(new Blob([text], { type }));
      a.download = name; a.click();
      setTimeout(() => URL.revokeObjectURL(a.href), 1000);
    };
    $('save-file').onclick = () => download(slug() + '.pixelgotchi.json', JSON.stringify(project, null, 1), 'application/json');
    $('open-file').onclick = () => $('file-input').click();
    $('file-input').onchange = async e => {
      const file = e.target.files[0]; e.target.value = '';
      if (!file) return;
      try {
        const data = JSON.parse(await file.text());
        if (data.format !== 'pixelgotchi-pet') throw new Error('format');
        change(() => { project = normalize(data); anim = 'idle'; step = 0; color = 1; });
      } catch (_) { alert(L('Esse arquivo não é um projeto do editor do PixelGotchi.', 'This file is not a PixelGotchi editor project.')); }
    };
    $('export-art').onclick = () => {
      if (validation.errors.length) { alert(L('Corrija os erros da conferência antes de exportar.', 'Fix the check errors before exporting.')); return; }
      let id = slug();
      if (A.pets.some(p => p.id === id)) id += 'x';
      download(id + '.art', P.toArtText(project, id), 'text/plain');
    };
    $('open-sim').onclick = () => { persist(); window.open('index.html?pet=meu', '_blank'); };
    $('connect').onclick = connect;
    $('send').onclick = send;
  }
  function clampView() {
    step = Math.min(step, project.anims[anim].frames.length - 1);
    color = Math.min(color, project.colors.length);
  }

  // ---------------------------------------------------------------- USB (Web Serial)
  const board = { port: null, hello: null, waiters: [], busy: false };
  const status = (text, kind = '') => { const s = $('board-status'); s.textContent = text; s.className = kind; };
  const BOARD_ERR = {
    crc: L('os dados chegaram corrompidos', 'the data arrived corrupted'), size: L('tamanho inválido', 'invalid size'),
    storage: L('não coube na memória da placa', 'it did not fit in the board memory'), incomplete: L('o envio ficou incompleto', 'the transfer was incomplete'),
    name: L('nome inválido', 'invalid name'), command: L('a placa não entendeu o comando', 'the board did not understand the command'),
  };
  function updateSend() {
    $('send').disabled = !board.hello || board.busy || validation.errors.length > 0;
    $('connect').textContent = board.port ? L('Desconectar', 'Disconnect') : L('Conectar placa', 'Connect board');
  }
  function nextReply(timeout) {
    return new Promise((resolve, reject) => {
      const w = { resolve, timer: setTimeout(() => { board.waiters = board.waiters.filter(x => x !== w); reject(new Error('timeout')); }, timeout) };
      board.waiters.push(w);
    });
  }
  async function write(text) {
    const writer = board.port.writable.getWriter();
    try { await writer.write(new TextEncoder().encode(text)); } finally { writer.releaseLock(); }
  }
  async function cmd(line, timeout = 2500) {
    const reply = nextReply(timeout);
    await write(line + '\n');
    return reply;
  }
  async function readLoop(port) {
    let buffer = '';
    const decoder = new TextDecoder();
    while (port.readable && board.port === port) {
      const reader = port.readable.getReader();
      board.reader = reader;
      try {
        for (;;) {
          const { value, done } = await reader.read();
          if (done) break;
          buffer += decoder.decode(value, { stream: true });
          let nl;
          while ((nl = buffer.indexOf('\n')) >= 0) {
            const line = buffer.slice(0, nl).trim(); buffer = buffer.slice(nl + 1);
            if (line.startsWith('PG ') && board.waiters.length) { const w = board.waiters.shift(); clearTimeout(w.timer); w.resolve(line); }
          }
        }
      } catch (_) { break; } finally { reader.releaseLock(); }
    }
  }
  async function disconnect() {
    const port = board.port;
    board.port = null; board.hello = null;
    try { if (board.reader) await board.reader.cancel(); } catch (_) {}
    try { if (port) await port.close(); } catch (_) {}
    updateSend();
  }
  async function connect() {
    if (board.port) { await disconnect(); status(L('Desconectado.', 'Disconnected.')); return; }
    if (!('serial' in navigator)) {
      status(L('Este navegador não acessa a USB. Use Chrome ou Edge em um computador.', 'This browser cannot access USB. Use Chrome or Edge on a computer.'), 'bad');
      return;
    }
    let port;
    try { port = await navigator.serial.requestPort(); await port.open({ baudRate: 115200 }); }
    catch (e) { status(L('Nenhuma porta aberta. Feche outros programas que usam a placa e tente de novo.', 'No port opened. Close other programs using the board and try again.'), 'bad'); return; }
    board.port = port; updateSend();
    readLoop(port);
    status(L('Conectado. Conversando com a placa…', 'Connected. Talking to the board…'));
    // A placa pode reiniciar ao abrir a porta: tenta por alguns segundos.
    for (let i = 0; i < 8 && board.port === port; i++) {
      try {
        const r = await cmd('PG?', 1000);
        if (r.startsWith('PG HELLO')) {
          const parts = r.split(' ');
          board.hello = { protocol: +parts[2], max: +parts[3], has: parts[4] === '1', active: parts[5] === '1', name: parts.slice(6).join(' ') };
          break;
        }
      } catch (_) { await new Promise(res => setTimeout(res, 400)); }
    }
    if (!board.hello) {
      status(L('A placa não respondeu. Ela precisa do firmware atual: atualize pelo instalador (isso apaga o pet salvo) e conecte de novo.',
        'The board did not answer. It needs the current firmware: update it with the installer (this erases the saved pet) and connect again.'), 'bad');
      await disconnect();
      return;
    }
    if (board.hello.protocol !== 1) {
      status(L('Firmware com outra versão do protocolo. Atualize pelo instalador.', 'Firmware with a different protocol version. Update it with the installer.'), 'bad');
      board.hello = null; updateSend(); return;
    }
    const current = board.hello.has ? `"${board.hello.name}"` : L('nenhum', 'none');
    status(L(`Placa pronta. Bichinho do editor gravado nela: ${current}.`, `Board ready. Editor pet stored on it: ${current}.`), 'good');
    updateSend();
  }
  async function send() {
    if (!board.hello || board.busy) return;
    const adopt = $('adopt').checked;
    if (adopt && !confirm(L('O bichinho atual da placa será substituído por um ovo deste. Continuar?', 'The pet currently on the board will be replaced by an egg of this one. Continue?'))) return;
    let bytes;
    try { bytes = P.encode(project); } catch (_) { renderIssues(); return; }
    if (bytes.length > board.hello.max) { status(L('O bichinho não cabe nesta placa.', 'The pet does not fit on this board.'), 'bad'); return; }
    board.busy = true; updateSend();
    const bar = $('progress'); bar.hidden = false; bar.value = 0;
    try {
      let r = await cmd('PGPUT ' + bytes.length);
      if (r !== 'PG READY') throw new Error(r);
      for (let at = 0; at < bytes.length; at += 64) {
        const chunk = bytes.slice(at, at + 64);
        r = await cmd('PGD ' + P.toHex(chunk));
        if (r !== 'PG ACK ' + (at + chunk.length)) throw new Error(r);
        bar.value = (at + chunk.length) / bytes.length;
      }
      r = await cmd('PGEND', 6000);
      if (!r.startsWith('PG SAVED')) throw new Error(r);
      if (adopt) {
        r = await cmd('PGADOPT');
        if (r !== 'PG ADOPTED') throw new Error(r);
        status(L('Pronto! Um ovo do seu bichinho está na placa: mexa com calma para chocar.', 'Done! An egg of your pet is on the board: move it gently to hatch.'), 'good');
      } else if (board.hello.active) {
        status(L('Pronto! O desenho novo já aparece na placa.', 'Done! The new drawing is already on the board.'), 'good');
      } else {
        status(L('Pronto! Ele está salvo na placa. Para usá-lo agora, marque "Trocar" e envie de novo, ou recomece o jogo (segure BOOT por 8 s) e escolha a 7ª espécie.',
          'Done! It is saved on the board. To use it now, check "Replace" and send again, or restart the game (hold BOOT for 8 s) and pick the 7th species.'), 'good');
      }
      board.hello.has = true; board.hello.name = project.name;
      if (adopt) board.hello.active = true; // o pet da placa agora é este
    } catch (e) {
      const code = String(e.message).startsWith('PG ERR ') ? e.message.slice(7) : '';
      const why = code ? (BOARD_ERR[code] || code) : L('a placa parou de responder', 'the board stopped answering');
      status(L(`Não foi possível enviar: ${why}. O bichinho que já estava na placa continua valendo.`, `Could not send: ${why}. The pet already on the board is kept.`), 'bad');
    } finally {
      board.busy = false; bar.hidden = true; updateSend();
    }
  }
  if ('serial' in navigator) navigator.serial.addEventListener('disconnect', e => {
    if (e.target === board.port) { disconnect(); status(L('A placa foi desconectada.', 'The board was disconnected.'), 'bad'); }
  });

  project = load();
  setup();
  render();
  requestAnimationFrame(drawPreview);
})();
