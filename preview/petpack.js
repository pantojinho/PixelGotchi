// Formato do bichinho criado no editor. O mesmo pacote binário "PGP1" que o
// firmware recebe pelo USB (src/CustomPet.h); aqui ele é montado, validado e
// convertido para o simulador e para o formato .art do repositório.
(function (root) {
  'use strict';
  const ANIMS = ['idle', 'blink', 'walk', 'eat', 'sleep', 'happy', 'sad', 'hungry', 'tired'];
  const FOODS = ['food_melon', 'food_fish', 'food_fly', 'food_worm', 'food_carrot', 'food_shrimp'];
  const LIMITS = { bytes: 3072, colors: 15, frames: 40, animFrames: 12, name: 12, msMin: 40, msMax: 5000 };
  const N = 8;
  const hexToInt = h => parseInt(String(h).replace('#', ''), 16) >>> 0;
  const intToHex = v => '#' + (v >>> 0).toString(16).padStart(6, '0').toUpperCase();

  function crc32(bytes) {
    let crc = 0xFFFFFFFF;
    for (const b of bytes) {
      crc ^= b;
      for (let i = 0; i < 8; i++) crc = (crc >>> 1) ^ (0xEDB88320 & -(crc & 1));
    }
    return (~crc) >>> 0;
  }

  // Mesma conta de wildColor() em src/CustomPet.cpp.
  function wildColor(hex) {
    const c = hexToInt(hex), r = Math.floor(((c >> 16) & 255) * 3 / 5), g = Math.floor(((c >> 8) & 255) / 2), b = Math.floor((c & 255) * 2 / 5);
    return intToHex((r << 16) | (g << 8) | b);
  }

  // Ovo nas cores do bichinho: casca clara, pintas na cor principal, brilho e rachadura.
  function autoEgg(colors) {
    const main = hexToInt(colors[0] || '#C9A27A');
    const mix = (c, t, k) => [16, 8, 0].reduce((v, s) => v | (Math.round(((c >> s) & 255) * (1 - k) + t * k) << s), 0);
    return [intToHex(mix(main, 255, 0.82)), intToHex(main), '#FFFFFF', intToHex(mix(main, 0, 0.55))];
  }

  function blankProject() {
    const frame = () => ({ px: new Array(64).fill(0) });
    const anims = {};
    ANIMS.forEach((a, i) => { anims[a] = { ms: a === 'blink' ? 150 : 450, frames: [0] }; });
    return { format: 'pixelgotchi-pet', version: 1, name: 'Meu pet', side: false, food: 'food_fish',
      colors: ['#8DAAE2', '#FFF1D9', '#FF7FA6'], egg: null, frames: [frame()], anims };
  }

  // Projeto a partir de um bichinho de fábrica (window.ART), para servir de modelo.
  function fromArt(A, petId) {
    const pet = A.pets.find(p => p.id === petId);
    if (!pet) throw new Error('bichinho desconhecido: ' + petId);
    const colors = [], frames = [], anims = {};
    const colorIndex = hex => {
      const h = hex.toUpperCase();
      let i = colors.indexOf(h);
      if (i < 0) { colors.push(h); i = colors.length - 1; }
      return i + 1;
    };
    Object.values(A.palettes[A.sprites[A.anims[petId + '_idle'].frames[0].split('@')[0]].pal]).forEach(colorIndex);
    const idle = A.sprites[A.anims[petId + '_idle'].frames[0].split('@')[0]];
    const cx = Math.floor((N - idle.w) / 2) + Math.floor(idle.w / 2);
    const key = new Map();
    for (const a of ANIMS) {
      const src = A.anims[petId + '_' + a];
      anims[a] = { ms: src.ms, frames: src.frames.map(ref => {
        const [name, palName] = ref.split('@'), s = A.sprites[name], pal = A.palettes[palName || s.pal];
        const px = new Array(64).fill(0), left = cx - Math.floor(s.w / 2), top = N - s.h;
        s.rows.forEach((row, y) => [...row].forEach((ch, x) => {
          const X = left + x, Y = top + y;
          if (ch !== '.' && X >= 0 && X < N && Y >= 0 && Y < N) px[Y * N + X] = colorIndex(pal[ch]);
        }));
        const k = px.join(',');
        if (!key.has(k)) { key.set(k, frames.length); frames.push({ px }); }
        return key.get(k);
      }) };
    }
    const eggPal = A.palettes['egg_' + petId];
    return { format: 'pixelgotchi-pet', version: 1, name: pet.name, side: !!pet.side, food: pet.food,
      colors, egg: eggPal ? ['E', 'S', 'H', 'C'].map(k => eggPal[k].toUpperCase()) : null, frames, anims };
  }

  // Recorte usado no firmware: todas as poses dividem as mesmas colunas (o pet
  // não "pula" de lado entre poses) e cada uma vai do topo desenhado até o chão.
  function crop(project) {
    const used = [...new Set(ANIMS.flatMap(a => (project.anims[a] || { frames: [] }).frames))].sort((a, b) => a - b);
    let minX = N, maxX = -1;
    for (const f of used) {
      const px = (project.frames[f] || { px: [] }).px;
      for (let i = 0; i < 64; i++) if (px[i]) { minX = Math.min(minX, i % N); maxX = Math.max(maxX, i % N); }
    }
    if (maxX < 0) return null;
    const w = maxX - minX + 1, out = new Map();
    for (const f of used) {
      const px = project.frames[f].px;
      let top = N - 1;
      for (let i = 0; i < 64; i++) if (px[i] && i % N >= minX && i % N <= maxX) top = Math.min(top, Math.floor(i / N));
      const h = N - top, cells = [];
      for (let y = top; y < N; y++) for (let x = minX; x <= maxX; x++) cells.push(px[y * N + x]);
      out.set(f, { w, h, px: cells });
    }
    return { minX, maxX, w, frames: out };
  }

  // Diferença entre idle e comer: o jogo usa para achar a boca (Game.cpp mouthOf).
  function framesDiffer(project, a, b) {
    const fa = project.anims[a] && project.anims[a].frames, fb = project.anims[b] && project.anims[b].frames;
    if (!fa || !fb) return false;
    const idle = project.frames[fa[0]].px.join(',');
    return fb.some(f => project.frames[f].px.join(',') !== idle);
  }

  function validate(project) {
    const errors = [], warnings = [];
    const colors = project.colors || [];
    if (colors.length < 1) errors.push({ code: 'colors_min' });
    if (colors.length > LIMITS.colors) errors.push({ code: 'colors_max', n: LIMITS.colors });
    if (colors.some(c => !/^#[0-9a-fA-F]{6}$/.test(c))) errors.push({ code: 'color_format' });
    const name = project.name || '';
    if (name.length > LIMITS.name) errors.push({ code: 'name_long', n: LIMITS.name });
    if (/[^\x20-\x7E]/.test(name)) errors.push({ code: 'name_chars' });
    if (!FOODS.includes(project.food)) errors.push({ code: 'food' });
    for (const a of ANIMS) {
      const an = project.anims[a];
      if (!an || !an.frames.length) { errors.push({ code: 'anim_empty', anim: a }); continue; }
      if (an.frames.length > LIMITS.animFrames) errors.push({ code: 'anim_long', anim: a, n: LIMITS.animFrames });
      if (!(an.ms >= LIMITS.msMin && an.ms <= LIMITS.msMax)) errors.push({ code: 'anim_ms', anim: a, min: LIMITS.msMin, max: LIMITS.msMax });
      if (an.frames.some(f => !project.frames[f])) errors.push({ code: 'anim_ref', anim: a });
    }
    project.frames.forEach((f, i) => { if (f.px.some(v => v < 0 || v > colors.length)) errors.push({ code: 'frame_color', frame: i }); });
    if (errors.length) return { errors, warnings };
    const c = crop(project);
    if (!c) { errors.push({ code: 'empty' }); return { errors, warnings }; }
    if (c.frames.size > LIMITS.frames) errors.push({ code: 'frames_max', n: LIMITS.frames });
    if (c.w > 7) warnings.push({ code: 'wide' });
    if (!framesDiffer(project, 'idle', 'eat')) warnings.push({ code: 'eat_same' });
    const sleepH = Math.max(...project.anims.sleep.frames.map(f => c.frames.get(f).h));
    if (sleepH > 5) warnings.push({ code: 'sleep_tall' });
    try { const bytes = encodeUnchecked(project, c); if (bytes.length > LIMITS.bytes) errors.push({ code: 'bytes', n: LIMITS.bytes }); }
    catch (e) { errors.push({ code: 'encode', message: e.message }); }
    return { errors, warnings };
  }

  function encodeUnchecked(project, c) {
    const order = [...c.frames.keys()], slot = new Map(order.map((f, i) => [f, i]));
    const out = [];
    const put = (...b) => b.forEach(v => out.push(v & 255));
    const rgb = hex => { const v = hexToInt(hex); put(v >> 16, v >> 8, v); };
    put(0x50, 0x47, 0x50, 0x31); // "PGP1"
    put(project.side ? 1 : 0, FOODS.indexOf(project.food));
    const name = [...(project.name || '')].map(ch => ch.charCodeAt(0));
    put(name.length, ...name);
    put(project.colors.length); project.colors.forEach(rgb);
    (project.egg || autoEgg(project.colors)).forEach(rgb);
    put(order.length);
    for (const f of order) { const fr = c.frames.get(f); put(fr.w, fr.h, ...fr.px); }
    for (const a of ANIMS) {
      const an = project.anims[a];
      put(an.ms, an.ms >> 8, an.frames.length, ...an.frames.map(f => slot.get(f)));
    }
    const crc = crc32(out);
    put(crc, crc >> 8, crc >> 16, crc >> 24);
    return Uint8Array.from(out);
  }

  function encode(project) {
    const v = validate(project);
    if (v.errors.length) { const e = new Error('pacote inválido'); e.errors = v.errors; throw e; }
    return encodeUnchecked(project, crop(project));
  }

  const toHex = bytes => Array.from(bytes, b => b.toString(16).padStart(2, '0')).join('');

  // Estruturas no formato de window.ART, para o simulador mostrar o pet
  // exatamente como o firmware vai desenhar (mesmo recorte).
  function toArt(project, id = 'meu') {
    const c = crop(project);
    const chars = 'ABCDEFGHIJKLMNO';
    const palettes = {}, sprites = {}, anims = {};
    palettes[id] = {}; palettes[id + '_wild'] = {};
    project.colors.forEach((hex, i) => { palettes[id][chars[i]] = hex.toUpperCase(); palettes[id + '_wild'][chars[i]] = wildColor(hex); });
    const egg = project.egg || autoEgg(project.colors);
    palettes['egg_' + id] = { E: egg[0], S: egg[1], H: egg[2], C: egg[3] };
    for (const [f, fr] of c.frames) {
      const rows = [];
      for (let y = 0; y < fr.h; y++) rows.push(fr.px.slice(y * fr.w, (y + 1) * fr.w).map(v => v ? chars[v - 1] : '.').join(''));
      sprites[`${id}_f${f}`] = { pal: id, rows, w: fr.w, h: fr.h };
    }
    for (const a of ANIMS) anims[`${id}_${a}`] = { ms: project.anims[a].ms, frames: project.anims[a].frames.map(f => `${id}_f${f}`) };
    anims[`${id}_egg`] = { ms: 700, frames: ['egg0@egg_' + id] };
    return { palettes, sprites, anims, pet: { id, name: project.name || 'Meu pet', food: project.food, wild: id + '_wild', side: !!project.side } };
  }

  // Texto no formato art/*.art, para colar no repositório (caminho compilado).
  function toArtText(project, id = 'meu') {
    const art = toArt(project, id), lines = [`# ${project.name || id} — criado no editor do PixelGotchi`];
    const pal = (name, entries, base) => { lines.push(`palette ${name}${base ? ' : ' + base : ''}`); Object.entries(entries).forEach(([k, v]) => lines.push(`  ${k} ${v}`)); lines.push('end', ''); };
    pal(id, art.palettes[id]);
    pal(id + '_wild', art.palettes[id + '_wild'], id);
    pal('egg_' + id, art.palettes['egg_' + id]);
    for (const [name, s] of Object.entries(art.sprites)) { lines.push(`sprite ${name} ${id}`, ...s.rows, 'end', ''); }
    for (const [name, a] of Object.entries(art.anims)) lines.push(`anim ${name} ${a.ms} ${a.frames.join(' ')}`);
    lines.push('', `pet ${id} "${(project.name || id).replace(/"/g, '')}" food=${project.food} wild=${id}_wild${project.side ? ' side=1' : ''}`);
    return lines.join('\n') + '\n';
  }

  const api = { ANIMS, FOODS, LIMITS, crc32, wildColor, autoEgg, blankProject, fromArt, crop, validate, encode, toHex, toArt, toArtText };
  if (typeof module !== 'undefined' && module.exports) module.exports = api;
  else root.PetPack = api;
})(typeof window !== 'undefined' ? window : globalThis);
