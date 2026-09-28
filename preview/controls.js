// Maquete de entrada humana. Arte e composição vêm da mesma galeria.
(() => {
  const el = id => document.getElementById(id);
  const labels = ['Alimentar', 'Brincar', 'Limpar', 'Remédio', 'Dormir / acordar', 'Carinho', 'Status', 'Voltar'];
  const icons = ['icon_food', 'icon_play', 'icon_clean', 'icon_medicine', 'icon_sleep', 'icon_pet', 'icon_status', 'icon_back'];
  let pet = A.pets.find(p => p.id === Q.get('pet')) || A.pets[0], pose = scenes(pet), state, screen = 'life', item = 0, page = 0;
  let action = '', actionAt = 0, deadline = 0, heldAt = null, resetSent = false;
  let holdResetTimer = null;
  let sleepByGesture = false, playedAt = -Infinity, pageAt = 0;
  const say = message => el('demo-message').textContent = message;
  // Mesmas cores de Game.cpp (C_HUNGER, C_HAPPY...): a cor do ícone que resolve.
  const C = { hunger:[255,58,74], bored:[154,92,255], poop:[127,216,255], sick:[62,214,76], energy:[255,138,26], happy:[255,111,168], care:[62,214,76] };
  const STATUS = [['stat_hunger','hunger','Comida','COMIDA'], ['stat_happy','happy','Alegria','ALEGRIA'], ['stat_energy','energy','Energia','ENERGIA'], ['stat_care','care','Saúde','SAUDE']];
  const STEP_MS = 150; // TEXT_STEP_MS do Config.h
  const AGE_TEXT = '0 DIAS', AGE_MS = (textWidth(AGE_TEXT) + N + 2) * STEP_MS;
  const ICON_MS = 1000;
  const statusLabel = p => p === 3 ? (state.sick ? 'DOENTE' : state.poop ? 'SUJO' : 'SAUDE') : STATUS[p][3];
  const pageMs = p => ICON_MS + (textWidth(statusLabel(p)) + N + 2) * STEP_MS;
  // Item do menu que resolve a necessidade mais urgente (espelha urgentNeed()).
  function urgentNeed() {
    const s = state;
    if (!(s.hunger < 25 || s.happy < 25 || s.sick || s.poop || (!s.asleep && s.energy < 15))) return -1;
    return s.sick ? 3 : s.poop ? 2 : s.hunger < 25 ? 0 : s.energy < 25 ? 4 : 1;
  }
  // Espelha Dna::name(): 2 ou 3 sílabas tiradas de um hash do DNA.
  function dnaName(bits) {
    let h = Math.imul(bits, 2654435761) >>> 0;
    h = (h ^ (h >>> 15)) >>> 0; h = Math.imul(h, 2246822519) >>> 0; h = (h ^ (h >>> 13)) >>> 0;
    const syl = 2 + (h & 1); h >>>= 1;
    let out = '';
    for (let s = 0; s < syl; s++) { out += 'BCDFGJKLMNPRSTVZ'[h & 15]; h >>>= 4; out += 'AEIOU'[(h & 7) % 5]; h >>>= 3; }
    return out;
  }
  let dna = (Math.random() * 2 ** 32) >>> 0;
  let lastActivityAt = performance.now();
  let sleepStartedAt = 0, sleepPetUntil = 0, sleepDreamVisible = false, sleepDreamViewAt = 0;
  // A maquete encurta a espera ociosa para dar para ver o efeito no navegador;
  // capítulos e bolha usam os mesmos tempos do firmware.
  const IDLE_DREAM_AFTER = 12000, IDLE_DREAM_CYCLE = 20000, IDLE_DREAM_SHOW = 7000;
  const SLEEP_DREAM_AFTER = 8000, SLEEP_PET_REVEAL = 8000, DREAM_STEP = 500;
  const DREAM_BUBBLE = 2400, BUBBLE_GROW = 900, DREAM_FADE = 1000;
  const CHAPTER_MIN = 20000, CHAPTER_MAX = 40000, STILL_STEPS = 6, OSC_MAX = 10000;
  function dreamHash(value) {
    value=Math.imul(value^(value>>>16),0x7FEB352D);
    value=Math.imul(value^(value>>>15),0x846CA68B);
    return (value^(value>>>16))>>>0;
  }
  // ---- Autômato: mesma implementação de src/Dream.cpp (bit a bit).
  const KIND = { gliders:0, pulse:1, soup:2 }, KIND_NAME = ['gliders atravessando', 'pulsação', 'grupos nascendo e se desfazendo'];
  const GLIDER=[[1,0],[2,1],[0,2],[1,2],[2,2]], BLINKER=[[1,0],[1,1],[1,2]];
  const TOAD=[[1,0],[2,0],[3,0],[0,1],[1,1],[2,1]], BEACON=[[0,0],[1,0],[0,1],[3,2],[2,3],[3,3]];
  class Life {
    constructor(rows) { this.rows = new Uint8Array(rows || 8); }
    clear() { this.rows.fill(0); }
    set(x, y) { this.rows[y & 7] |= 1 << (x & 7); }
    alive(x, y) { return x >= 0 && x < 8 && y >= 0 && y < 8 && ((this.rows[y] >> x) & 1) === 1; }
    pop() { let n = 0; for (const r of this.rows) for (let x = 0; x < 8; x++) n += (r >> x) & 1; return n; }
    sig() { return this.rows.join(','); }
    copy() { return new Life(this.rows); }
    step() {
      const next = new Uint8Array(8);
      for (let y=0;y<8;y++) for (let x=0;x<8;x++) {
        let n=0;
        for (let dy=-1;dy<=1;dy++) for (let dx=-1;dx<=1;dx++) if (dx||dy) n += this.alive((x+dx+8)%8,(y+dy+8)%8) ? 1 : 0;
        if (n===3 || (this.alive(x,y) && n===2)) next[y] |= 1 << x;
      }
      this.rows = next;
    }
    seedChapter(kind, seed) {
      this.clear(); seed >>>= 0;
      const flipX = seed & 1, flipY = seed & 2, swap = seed & 4, ox = (seed >>> 3) & 7, oy = (seed >>> 6) & 7;
      const place = (cells, dx, dy) => cells.forEach(([cx, cy]) => {
        let x = cx + dx, y = cy + dy;
        if (flipX) x = 7 - x;
        if (flipY) y = 7 - y;
        if (swap) [x, y] = [y, x];
        this.set(x + ox, y + oy);
      });
      if (kind === KIND.gliders) { place(GLIDER, 0, 0); if ((seed >>> 9) & 1) place(GLIDER, 4, 4); }
      else if (kind === KIND.pulse) {
        const v = (seed >>> 10) & 3;
        if (v === 0) place(BLINKER, 0, 0); else if (v === 1) place(TOAD, 0, 0);
        else if (v === 2) place(BEACON, 0, 0); else { place(BLINKER, 0, 0); place(BLINKER, 4, 4); }
      } else {
        let rng = (seed ^ 0x9E3779B9) >>> 0; if (!rng) rng = 0xA341316C;
        const next = () => { rng ^= rng << 13; rng >>>= 0; rng ^= rng >>> 17; rng ^= rng << 5; rng >>>= 0; return rng; };
        for (let attempt = 0; attempt < 6; attempt++) {
          this.clear();
          const density = 20 + next() % 13;
          for (let y = 0; y < 8; y++) for (let x = 0; x < 8; x++) if (next() % 100 < density) this.set(x, y);
          const probe = this.copy(); for (let i = 0; i < 8; i++) probe.step();
          if (probe.pop()) break;
        }
        if (!this.pop()) place(GLIDER, 0, 0);
      }
    }
  }
  // ---- Sonho: capítulos, troca por substituição de pixels e visitas acordado.
  const D = { grid:new Life(), prev:new Life(), seeded:false, ambient:false, place:false, calm:true,
    base:1, seed:1, chapter:0, chapterAt:0, chapterMs:CHAPTER_MIN, kind:KIND.gliders, lastStep:0,
    sig1:'', sig2:'', still:0, osc:0, fading:false, visit:-1,
    bright:[90,210,245], dim:[28,82,135], prevBright:[90,210,245], prevDim:[28,82,135] };
  function sleepKind(chapter) {
    const i = D.base % 4 + chapter;
    return D.calm ? [KIND.gliders, KIND.pulse, KIND.soup][i % 3] : [KIND.soup, KIND.gliders, KIND.soup, KIND.pulse][i % 4];
  }
  function startChapter(now, fade) {
    D.prev = D.grid.copy(); D.prevBright = D.bright; D.prevDim = D.dim;
    D.fading = fade && D.prev.pop() > 0;
    D.seed = dreamHash((D.base + Math.imul(D.chapter, 0x9E3779B9)) >>> 0);
    D.kind = sleepKind(D.chapter);
    D.grid.seedChapter(D.kind, D.seed);
    const p = D.chapter % 3;
    D.bright = (D.calm ? [[90,210,245],[120,235,110],[195,138,245]] : [[255,112,96],[255,155,45],[245,95,165]])[p];
    D.dim = (D.calm ? [[28,82,135],[28,90,45],[75,36,130]] : [[118,35,85],[105,42,15],[90,25,70]])[p];
    D.chapterAt = D.lastStep = now;
    D.chapterMs = CHAPTER_MIN + D.seed % (CHAPTER_MAX - CHAPTER_MIN + 1);
    D.still = D.osc = 0; D.sig1 = D.sig2 = '';
    if (state.asleep) say(`Sonho, capítulo ${D.chapter + 1}: ${KIND_NAME[D.kind]}.`);
  }
  function seedDream(now) {
    D.calm = !state.sick && !state.poop && state.hunger >= 50 && state.happy >= 50 && state.energy >= 50;
    D.base = (dna^(state.hunger<<24)^(state.energy<<16)^(state.happy<<8)^(state.poop<<4))>>>0;
    D.ambient = false; D.place = false; D.chapter = 0; D.grid.clear();
    startChapter(now, false); D.seeded = true;
  }
  // Visita acordada: glider (deslocamento) ou blinker (pulsação) + passarinho.
  function seedAmbient(now, visit) {
    D.base = dreamHash((dna ^ Math.imul(visit, 0x9E3779B9)) >>> 0);
    D.calm = true; D.ambient = true; D.place = true; D.chapter = visit; D.visit = visit;
    D.kind = visit % 2 ? KIND.pulse : KIND.gliders;
    D.seed = (D.base & ~(7 << 9)) >>> 0;
    D.grid.seedChapter(D.kind, D.seed);
    D.bright = [90,210,245]; D.dim = [28,82,135];
    D.chapterAt = D.lastStep = now; D.fading = false; D.seeded = true;
  }
  function advanceDream(now) {
    if (!D.seeded) seedDream(now);
    if (D.fading && now - D.chapterAt >= DREAM_FADE) D.fading = false;
    if (now - D.lastStep < DREAM_STEP) return;
    D.lastStep = now;
    const before = D.grid.sig(); D.grid.step();
    if (D.ambient) return;
    const after = D.grid.sig();
    D.still = after === before ? D.still + 1 : 0;
    D.osc = after !== before && (after === D.sig1 || after === D.sig2) ? D.osc + 1 : 0;
    D.sig2 = D.sig1; D.sig1 = before;
    const staleOsc = D.kind !== KIND.pulse && D.osc * DREAM_STEP >= OSC_MAX;
    if (!D.grid.pop() || D.still >= STILL_STEPS || staleOsc || now - D.chapterAt >= D.chapterMs) {
      D.chapter++; startChapter(now, true);
    }
  }
  function drawDreamWorld(buf, now) {
    const stage = D.fading ? 1 + Math.floor((now - D.chapterAt) * 4 / DREAM_FADE) : 4;
    for (let y=0;y<8;y++) for (let x=0;x<8;x++) {
      const fresh = (x + 2*y) % 4 < stage, g = fresh ? D.grid : D.prev;
      if (g.alive(x,y)) buf[y*8+x] = ((x+y)&1) ? (fresh ? D.bright : D.prevBright) : (fresh ? D.dim : D.prevDim);
    }
    return buf;
  }
  const ambientBird = () => D.ambient && D.kind === KIND.pulse;
  const birdX = now => 8 - Math.floor(now / 150) % 12;
  // Máscara: célula só em pixel livre que não encosta no pet (1 px de respiro).
  function ambientPixel(base, x, y) {
    const lit = (px, py) => px >= 0 && px < 8 && py >= 0 && py < 8 && !!base[py*8+px];
    return !lit(x,y) && !lit(x-1,y) && !lit(x+1,y) && !lit(x,y-1) && !lit(x,y+1);
  }
  function placeAmbient(base) {
    let best = -1, bestSeed = D.seed;
    for (let k = 0; k < 16; k++) {
      const candidate = ((D.seed & ~0xFFF) | ((D.seed + k * 37) & 0x1FF)) >>> 0, probe = new Life();
      probe.seedChapter(D.kind, candidate);
      let visible = 0;
      for (let g = 0; g < 6; g++) {
        for (let y=0;y<8;y++) for (let x=0;x<8;x++) if (probe.alive(x,y) && ambientPixel(base,x,y)) visible++;
        probe.step();
      }
      if (visible > best) { best = visible; bestSeed = candidate; }
    }
    D.seed = bestSeed; D.grid.seedChapter(D.kind, D.seed); D.place = false;
  }
  function drawAmbient(buf, now) {
    const base = buf.slice();
    if (D.place) placeAmbient(base);
    for (let y=0;y<8;y++) for (let x=0;x<8;x++)
      if (D.grid.alive(x,y) && ambientPixel(base,x,y)) buf[y*8+x] = ((x+y)&1) ? D.bright : D.dim;
    if (ambientBird()) {
      const x = birdX(now), y = 1 + Math.floor(now / 700) % 2, c = [205,228,255];
      setFree(buf,x,y+1,c); setFree(buf,x+1,y,c); setFree(buf,x+2,y+1,c);
    }
  }
  function ambientFocusX(now) {
    if (ambientBird()) return birdX(now) + 1;
    let sum = 0, n = 0;
    for (let y=0;y<8;y++) for (let x=0;x<8;x++) if (D.grid.alive(x,y)) { sum += x; n++; }
    return n ? Math.floor(sum / n) : B.x;
  }
  // Entrada: olhos fechados, bolhinhas, a bolha cresce e o sonho substitui o pet.
  function drawSleepingDream(buf, now, t) {
    const e = now - sleepDreamViewAt;
    if (e >= DREAM_BUBBLE) return drawDreamWorld(buf, now);
    const ref = animFrame(pet.id + '_sleep', t), s = frameRef(ref).s, flip = pet.side && !B.faceRight;
    blitA(buf, ref, B.x, 7, flip);
    const dir = B.faceRight ? 1 : -1, headTop = N - s.h;
    const bx = Math.max(1, Math.min(N - 2, B.x + dir * 2)), by = headTop > 3 ? 1 : 0, ring = [120,150,210];
    if (e < BUBBLE_GROW) {
      const night = dimBuf(buf, 110/255);
      setFree(night, B.x + dir, headTop - 1, ring);
      if (e >= 300) setFree(night, bx - dir, by + 1, ring);
      if (e >= 600) setFree(night, bx, by, D.bright);
      return night;
    }
    const world = drawDreamWorld(newBuf(), now), out = newBuf();
    const r = 1 + Math.floor((e - BUBBLE_GROW) * 10 / (DREAM_BUBBLE - BUBBLE_GROW)), r2 = r*r, inner2 = (r-1)*(r-1);
    for (let y=0;y<8;y++) for (let x=0;x<8;x++) {
      const d2 = (x-bx)**2 + (y-by)**2, i = y*8+x;
      out[i] = d2 < inner2 ? world[i] : d2 <= r2 ? (world[i] || ring) : (buf[i] && buf[i].map(v => Math.floor(v*110/255)));
    }
    return out;
  }
  // ---- Pequenas intenções acordado (espelha pickBehavior/updateBehavior).
  const BEH = ['pausa', 'passeio', 'farejando', 'olhando em volta', 'observando algo passar', 'pulinhos', 'se acomodando para cochilar', 'seguindo uma borboleta'];
  const B = { beh:0, at:0, until:0, x:3, target:3, faceRight:true, lastStep:0, blinkAt:0, bugX:0, bugY:1, bugDir:1, bugStep:0 };
  const NAP_SETTLE = 1500;
  const rnd = n => n ? Math.floor(Math.random() * n) : 0;
  const rndRange = (a, b) => a + rnd(b - a + 1);
  const idleW = () => idleSprite(pet).w;
  const minCx = () => Math.floor(idleW() / 2), maxCx = () => N - 1 - (idleW() - 1 - Math.floor(idleW() / 2));
  function pickBehavior(now) {
    const act = (dna >>> 12) & 255, cur = (dna >>> 20) & 255, s = state;
    const w = [30, 20 + (act >> 3), 6 + (cur >> 4), 6 + (cur >> 4), 4 + (cur >> 4),
      s.happy > 60 ? 3 + (act >> 5) : 0, s.energy < 40 ? 20 : 3, s.energy > 40 ? 3 + (act >> 5) : 0];
    if (pet.id === 'capy') { w[0] += 20; w[2] += 16; w[4] += 6; w[6] += 8; w[7] = 0; w[5] = w[5] >> 1; }
    else if (pet.id === 'cat') { w[3] += 6; w[4] += 14; w[7] += s.energy > 40 ? 14 : 0; }
    if (B.beh === 1) w[0] *= 2;
    if (B.beh === 0) w[1] = Math.floor(w[1] * 3 / 2);
    if (B.beh > 1) w[B.beh] = 0;
    let r = rnd(w.reduce((a, b) => a + b, 0)), i = 0;
    while (r >= w[i]) r -= w[i++];
    B.beh = i; B.at = now;
    if (i === 1) { B.target = rndRange(minCx(), maxCx()); if (B.target === B.x) B.beh = 0; B.until = now + 8000; }
    if (B.beh === 0) B.until = now + 1500 + (255 - act) * 12 + rnd(2500);
    if (i === 2) B.until = now + rndRange(1800, 3000);
    if (i === 3) B.until = now + rndRange(1500, 3000);
    if (i === 4) { B.bugDir = rnd(2) ? 1 : -1; B.bugX = B.bugDir > 0 ? -1 : N; B.bugY = rndRange(0, 1); B.bugStep = now; B.until = now + 8000; }
    if (i === 5) B.until = now + 1500;
    if (i === 6) B.until = now + NAP_SETTLE + rndRange(5000, 9000);
    if (i === 7) { B.bugX = rnd(N); B.bugY = rndRange(0, 2); B.until = now + 6000; }
    if (B.beh) say(`Acontecimento: ${BEH[B.beh]}.`);
  }
  function stepToward(target, now, ms) {
    if (now - B.lastStep < ms || B.x === target) return;
    B.lastStep = now; B.faceRight = target > B.x; B.x += B.faceRight ? 1 : -1;
  }
  function updateBehavior(now) {
    const s = state;
    if (s.hunger < 25 || s.sick || s.happy < 25) { stepToward(minCx(), now, 400); return; }
    if (s.energy < 25) return;
    if (B.beh === 1) { stepToward(B.target, now, 330); if (B.x === B.target) B.until = now; }
    else if (B.beh === 3) { if (Math.floor((now - B.at) / 700) % 2 === 1) { B.faceRight = !B.faceRight; B.at = now; } }
    else if (B.beh === 4) {
      if (now - B.bugStep > (pet.id === 'capy' ? 420 : 320)) { B.bugStep = now; B.bugX += B.bugDir; if (B.bugX < -1 || B.bugX > N) B.until = now; }
      B.faceRight = B.bugX >= B.x;
    } else if (B.beh === 7) {
      if (now - B.bugStep > 260) { B.bugStep = now; B.bugX = Math.max(0, Math.min(N - 1, B.bugX + rnd(3) - 1)); B.bugY = rndRange(0, 2); }
      stepToward(clampCx(pet, B.bugX), now, 270);
    }
    if (now >= B.until) pickBehavior(now);
  }
  // BOOT ou movimento interrompem o acontecimento ocioso, com uma piscada.
  function reactToInput(now) {
    if (B.beh <= 1) return;
    B.beh = 0; B.at = now; B.until = now + 2500; B.blinkAt = now;
    say('Ele percebeu você e parou o que fazia.');
  }
  function sniffSpot(b) {
    const flip = pet.side && !B.faceRight, [mx, my] = mouthAt(pet, B.x, flip);
    const dir = pet.side ? (B.faceRight ? 1 : -1) : (B.x <= N / 2 ? 1 : -1);
    for (let dx = 0; dx < 4; dx++) for (let y = N - 1; y >= my; y--) {
      const x = mx + dir * dx;
      if (x >= 0 && x < N && !b[y*8+x]) return [x, y];
    }
    return null;
  }
  function drawBehavior(b, now, t, watching) {
    const flip = pet.side && !B.faceRight, id = pet.id, beh = watching ? 0 : B.beh;
    if (beh === 1) blitA(b, animFrame(id + '_walk', t), B.x, 7, flip);
    else if (beh === 2) {
      blitA(b, animFrame(id + (Math.floor(t / 700) % 3 === 2 ? '_idle' : '_eat'), t), B.x, 7, flip);
      const spot = Math.floor(t / 450) % 2 === 0 && sniffSpot(b);
      if (spot) b[spot[1]*8+spot[0]] = [130,175,100];
    } else if (beh === 3) { B.faceRight = (Math.floor(t / 900) + dna) % 2 === 1; blitA(b, animFrame(id + '_idle', t), B.x, 7, pet.side && !B.faceRight); }
    else if (beh === 4) { blitA(b, animFrame(id + '_idle', t), B.x, 7, flip); setFree(b, B.bugX, B.bugY, [205,228,255]); }
    else if (beh === 5) blitA(b, animFrame(id + '_happy', t), B.x, 7 - Math.floor(t / 250) % 2, flip);
    else if (beh === 6) {
      if (t < NAP_SETTLE) blitA(b, animFrame(id + '_tired', t), B.x, 7, flip);
      else {
        blitA(b, animFrame(id + '_sleep', t), B.x, 7, flip);
        const z = frameRef('fx_z').s, p = (t - NAP_SETTLE) % 2400;
        blit(b, 'fx_z', 0, -Math.floor(p * (z.h + 1) / 2400));
      }
    } else if (beh === 7) {
      blitA(b, animFrame(id + '_walk', t), B.x, 7, flip);
      const open = Math.floor(now / 200) % 2;
      setFree(b, B.bugX, B.bugY, open ? [255,176,48] : [255,226,140]);
      if (open) setFree(b, B.bugX + 1, B.bugY, [255,176,48]);
    } else {
      if (now >= B.blinkAt + 150) B.blinkAt = now + rndRange(2500, 5000);
      const blinking = now >= B.blinkAt && now < B.blinkAt + 150;
      blitA(b, animFrame(id + (blinking ? '_blink' : '_idle'), t), B.x, 7, flip);
    }
    return b;
  }
  let meal = null;
  function markActivity(now=performance.now()) { lastActivityAt=now; if (!state.asleep && screen === 'life') reactToInput(now); }
  function startSleep(now=performance.now()) {
    state.asleep=true; sleepStartedAt=now; sleepPetUntil=now+SLEEP_DREAM_AFTER;
    sleepDreamVisible=false;
    seedDream(now);
  }
  // Espelha petColor(): 1ª cor da paleta do idle.
  const petColor = () => { const { pal } = frameRef(animFrame(pet.id + '_idle', 0)); return hexToRgb(Object.values(pal)[0]); };
  const nameMs = () => (textWidth(dnaName(dna)) + N + 2) * STEP_MS;
  function nextStatusPage() {
    page++; pageAt = performance.now();
    if (page > STATUS.length) { screen = 'life'; say('De volta ao bichinho.'); }
    else say(page < STATUS.length ? `Status: ${STATUS[page][2]}` : 'Idade — maquete: 0 dias.');
  }
  function reset() {
    state = { hunger:80, happy:80, energy:90, poop:0, sick:false, asleep:false };
    screen = 'life'; action = ''; sleepByGesture = false; playedAt = -Infinity;
    Object.assign(B, { beh:0, at:performance.now(), until:performance.now() + 3000, x:centerCx(idleW()), faceRight:true, blinkAt:0 });
    D.seeded = false; sleepDreamVisible=false;
    markActivity();
    el('demo-need').value = 'normal';
    say('Pronto: clique para alimentar ou segure e solte para o menu.');
  }
  function suggest() {
    if (state.asleep) return 4;
    if (state.sick) return 3;
    if (state.poop) return 2;
    if (state.hunger < 25) return 0;
    if (state.energy < 25) return 4;
    if (state.happy < 25) return 1;
    return 5;
  }
  function care(index) {
    markActivity();
    screen = 'life'; action = '';
    const add = (key, v) => state[key] = Math.max(0, Math.min(100, state[key] + v));
    let ok = true;
    switch (index) {
      case 0:
        ok = !state.asleep && state.hunger < 95;
        if (ok) { add('hunger', 30); action = 'comendo'; meal = planMeal(pet); } // ganho fixo por refeição
        break;
      case 1:
        ok = !state.asleep && state.energy >= 10;
        if (ok) { add('happy', 20); add('energy', -8); add('hunger', -3); action = 'brincando'; B.x = centerCx(idleW()); }
        break;
      case 2: ok = state.poop > 0; if (ok) { state.poop = 0; action = 'limpar'; } break;
      case 3: ok = state.sick; if (ok) { state.sick = false; action = 'remedio'; } break;
      case 4:
        sleepByGesture = false;
        if (state.asleep) {
          if (state.energy < 50) add('happy', -5);
          state.asleep = false;
        } else startSleep();
        break;
      case 5: ok = !state.asleep; if (ok) { add('happy', 5); action = 'carinho'; } break;
      case 6: screen = 'status'; page = -1; pageAt = performance.now(); deadline = Infinity; say(`Status: o nome dele é ${dnaName(dna)}`); return;
      case 7: say('De volta ao bichinho.'); return;
    }
    actionAt = performance.now();
    if (!ok) action = 'recusa';
    say(ok ? `${labels[index]}: ${index === 4 ? (state.asleep ? 'boa noite!' : 'acordou!') : 'feito.'}` : 'Agora não: o bichinho não precisa desse cuidado.');
  }
  function input(long) {
    markActivity();
    if (screen === 'menu') {
      if (long) care(item);
      else { item = (item + 1) % icons.length; deadline = performance.now() + 8000; say(labels[item]); }
    } else if (screen === 'status') {
      if (long) { screen = 'life'; say('De volta ao bichinho.'); }
      else nextStatusPage();
    } else if (long) {
      screen = 'menu'; item = suggest(); deadline = performance.now() + 8000;
      say(`Menu: ${labels[item]}. Clique para trocar; segure e solte para confirmar.`);
    } else if (state.asleep) care(4);
    else if (!action) care(0);
  }
  function press() {
    if (heldAt !== null) return;
    markActivity();
    heldAt = performance.now(); resetSent = false;
    holdResetTimer = setTimeout(requestReset, 8000);
  }
  function requestReset() {
    if (heldAt === null || resetSent) return;
    resetSent = true; reset();
    say('Reset da maquete. No firmware, volta à seleção e ao ovo.');
  }
  function release(cancel = false) {
    if (heldAt === null) return;
    const duration = performance.now() - heldAt;
    clearTimeout(holdResetTimer);
    if (!cancel && duration >= 8000) requestReset();
    heldAt = null;
    if (!cancel && !resetSent) input(duration >= 600);
  }
  A.pets.forEach(p => { const o = new Option(p.name, p.id); el('demo-pet').add(o); });
  el('demo-pet').value = pet.id;
  el('demo-pet').onchange = e => { pet = A.pets.find(p => p.id === e.target.value); pose = scenes(pet); dna = (Math.random() * 2 ** 32) >>> 0; reset(); };
  el('demo-need').onchange = e => {
    const need = e.target.value; reset(); el('demo-need').value = need;
    if (['hunger', 'happy', 'energy'].includes(need)) state[need] = 15;
    if (need === 'poop') state.poop = 2;
    if (need === 'sick') state.sick = true;
    say('Necessidade aplicada. Abra o menu para ver a sugestão.');
  };
  el('boot').onpointerdown = e => { e.preventDefault(); el('boot').setPointerCapture(e.pointerId); press(); };
  el('boot').onpointerup = () => release();
  el('boot').onpointercancel = () => release(true);
  el('boot').onclick = e => { if (e.detail === 0) input(false); }; // ativação assistiva
  function tilt(dir) {
    markActivity();
    if (screen === 'menu') { item = (item + dir + icons.length) % icons.length; deadline = performance.now() + 8000; say(labels[item]); }
    else if (state.asleep) { sleepPetUntil=performance.now()+SLEEP_PET_REVEAL; say('Ele se mexeu, mas continua dormindo.'); }
    else {
      B.x = dir < 0 ? minCx() : maxCx(); B.faceRight = dir > 0;
      say('O bichinho acompanha a inclinação.');
    }
  }
  el('tilt-left').onclick = () => tilt(-1);
  el('tilt-right').onclick = () => tilt(1);
  el('shake').onclick = () => {
    if (screen !== 'life' || action || heldAt !== null) return;
    const now = performance.now();
    markActivity(now);
    if (state.asleep) { sleepPetUntil=now+SLEEP_PET_REVEAL; say('Ele se mexeu, mas continua dormindo.'); return; }
    if (now - playedAt < 5000) { say('Dê uma pausa de 5 s entre brincadeiras.'); return; }
    playedAt = now; care(1);
  };
  el('face-down').onclick = () => {
    if (screen !== 'life' || state.asleep) return;
    startSleep(); sleepByGesture = true; action = '';
    say('Boa noite! Na placa, mantenha virado por 1,5 s.');
  };
  el('face-up').onclick = () => {
    markActivity();
    if (sleepByGesture && state.asleep) { care(4); say('Desvirou: o bichinho acordou.'); }
    sleepByGesture = false;
  };
  document.addEventListener('keydown', e => {
    if (['INPUT', 'SELECT'].includes(e.target.tagName)) return;
    if (e.code === 'Space') { e.preventDefault(); if (!e.repeat) press(); }
    if (e.code === 'ArrowLeft' || e.code === 'ArrowRight') { e.preventDefault(); if (!e.repeat) tilt(e.code === 'ArrowLeft' ? -1 : 1); }
  });
  document.addEventListener('keyup', e => { if (e.code === 'Space') { e.preventDefault(); release(); } });
  window.addEventListener('blur', () => release(true));
  reset();
  players.push({ ctx:el('demo-matrix').getContext('2d'), cellSize:30, label:'Interação', fn:t => {
    const now = performance.now(), b = newBuf();
    if (heldAt !== null && now - heldAt >= 8000 && !resetSent) {
      requestReset();
    }
    if (screen !== 'life' && now >= deadline && heldAt === null) { screen = 'life'; say('Menu fechado por inatividade.'); }
    const actionMs = { comendo:3000, brincando:3000, carinho:1600, limpar:1600, remedio:2000, recusa:900 };
    if (action && now - actionAt >= actionMs[action]) action = '';
    if(screen==='life'&&!state.asleep&&!action&&state.energy<25&&now-lastActivityAt>30000) {
      startSleep(now); say('Cansado e sozinho: cochilou. Ele também vai sonhar.');
    }
    let buf = b, fullDream = false;
    if (screen === 'menu') {
      const { s } = frameRef(icons[item]);
      if (item === 6) STATUS.forEach(([, key], i) => { // espelha drawStatusIcon()
        const v = key === 'care' ? 100 : state[key], h = Math.floor((v * 7 + 99) / 100);
        for (let y = 0; y < 7; y++) if (6 - y < h) b[y * 8 + 1 + i * 2] = C[key];
      });
      else blit(b, icons[item], Math.floor((N - s.w) / 2), Math.floor((N - 1 - s.h) / 2));
      for (let x = 0; x < 8; x++) b[56 + x] = x === item ? [255,255,255] : [40,40,52];
    } else if (screen === 'status') {
      const pt = now - pageAt;
      if (heldAt === null && pt > (page < 0 ? nameMs() : page < STATUS.length ? pageMs(page) : AGE_MS)) nextStatusPage();
      if (page < 0) scrollText(b, dnaName(dna), pt, 1, petColor(), STEP_MS);
      else if (page === STATUS.length) scrollText(b, AGE_TEXT, pt, 1, [255,255,255], STEP_MS);
      else if (page < STATUS.length) {
        const [spr, key] = STATUS[page], v = key === 'care' ? 100 : state[key], col = C[key];
        if (pt >= ICON_MS) scrollText(b, statusLabel(page), pt - ICON_MS, 0, col, STEP_MS);
        else if (v >= 25 || Math.floor(now / 300) % 2) blit(b, spr, Math.floor((N - frameRef(spr).s.w) / 2), 0);
        const full = Math.floor((v * 8 + 50) / 100), len = pt < 400 ? Math.floor(full * pt / 400) : full;
        for (let x = 0; x < 8; x++) for (let y = 6; y < 8; y++) b[y*8+x] = x < len ? col : col.map(c => Math.floor(c*30/255));
      }
    } else if (screen === 'life' && state.asleep) {
      const reveal = now - sleepStartedAt < SLEEP_DREAM_AFTER || now < sleepPetUntil;
      if (reveal) { // pet dormindo, com os Zzz e a luz de dormir
        sleepDreamVisible = false;
        blitA(b, animFrame(pet.id + '_sleep', t), B.x, 7, pet.side && !B.faceRight);
        blit(b, 'fx_z', 0, -Math.floor((t % 2400) / 2400 * 4));
        buf = dimBuf(b, 110 / 255);
      } else {
        if (!sleepDreamVisible) { sleepDreamVisible = true; sleepDreamViewAt = now; }
        if (now - sleepDreamViewAt >= DREAM_BUBBLE) advanceDream(now); else D.lastStep = now;
        buf = drawSleepingDream(b, now, t); fullDream = true;
      }
    } else if (screen === 'life' && action === 'comendo') {
      B.x = drawMeal(b, pet, meal, now - actionAt); B.faceRight = meal.dir > 0;
    } else if (screen === 'life' && action && pose[action]) buf = careFrame(pet, action, now - actionAt, B.x);
    else if (screen === 'life' && action === 'recusa') blitA(b, animFrame(pet.id + '_sad', t), B.x + (Math.floor((now - actionAt) / 120) % 2 ? 1 : -1), 7);
    else if (screen === 'life' && action === 'limpar') { blitA(b, animFrame(pet.id + '_idle', t), B.x, 7); blitFree(b, animFrame('fx_sparkle_anim', t), Math.floor((now-actionAt)/140)-2, 5); }
    else if (screen === 'life' && action === 'remedio') buf=careFrame(pet,'remedio',now-actionAt,B.x);
    else if (screen === 'life' && state.sick) buf = pose.doente(t);
    else if (screen === 'life' && state.hunger < 25) buf = pose['com fome'](t);
    else if (screen === 'life' && state.energy < 25) buf = pose.cansado(t);
    else if (screen === 'life' && state.happy < 25) buf = pose.triste(t);
    else if (screen === 'life') {
      // Visita de Conway (espelha updateDreamView): o pet para e observa.
      const idle = now - lastActivityAt;
      const visiting = urgentNeed() < 0 && idle >= IDLE_DREAM_AFTER && (idle - IDLE_DREAM_AFTER) % IDLE_DREAM_CYCLE < IDLE_DREAM_SHOW;
      if (visiting) {
        const visit = Math.floor((idle - IDLE_DREAM_AFTER) / IDLE_DREAM_CYCLE);
        if (!D.seeded || !D.ambient || visit !== D.visit) {
          seedAmbient(now, visit);
          say(visit % 2 ? 'Um blinker pulsa ao redor e um passarinho passa.' : 'Um glider atravessa o mundo ao redor.');
        }
        advanceDream(now);
        if (pet.side) B.faceRight = ambientFocusX(now) >= B.x;
      } else updateBehavior(now);
      drawBehavior(b, now, now - B.at, visiting);
      if (visiting) drawAmbient(b, now); // máscara só visual; o autômato continua completo
    }
    if (screen === 'life' && !fullDream && state.poop && action !== 'limpar') blitFree(buf, 'fx_poop', 5, 6);
    // Pedido de ajuda: ícone do menu que resolve a cada 4 s; senão, pontinho na cor dele.
    const need = screen === 'life' && !action && !fullDream ? urgentNeed() : -1;
    if (need >= 0 && now % 4000 < 900) {
      buf = newBuf();
      const { s } = frameRef(icons[need]);
      blit(buf, icons[need], Math.floor((N - s.w) / 2), Math.floor((N - s.h) / 2));
    } else if (need >= 0 && Math.floor(now / 700) % 2) buf[7] = [C.hunger, C.bored, C.poop, C.sick, C.energy][need];
    if (heldAt !== null) {
      const held = now - heldAt;
      if (held >= 3000) {
        buf = dimBuf(buf, 80/255);
        for (let x = 0; x < 8; x++) buf[56+x] = x < Math.floor((held-3000)*8/5000) ? [255,40,40] : [50,8,8];
      } else if (held >= 600) buf[7] = [62,214,76];
    }
    el('demo-stats').textContent = `Saciedade ${state.hunger} · Alegria ${state.happy} · Energia ${state.energy}${state.asleep ? ' · Dormindo' : ''}`;
    return buf;
  }});
})();
