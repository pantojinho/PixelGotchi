// Maquete de entrada humana. Arte e composição vêm da mesma galeria.
(() => {
  const el = id => document.getElementById(id);
  const labels = ['Alimentar', 'Brincar', 'Limpar', 'Remédio', 'Dormir / acordar', 'Carinho', 'Status', 'Voltar'];
  const icons = ['icon_food', 'icon_play', 'icon_clean', 'icon_medicine', 'icon_sleep', 'icon_pet', 'icon_status', 'icon_back'];
  let pet = A.pets[0], pose = scenes(pet), state, screen = 'life', item = 0, page = 0;
  let action = '', actionAt = 0, deadline = 0, heldAt = null, resetSent = false;
  let holdResetTimer = null;
  let sleepByGesture = false, playedAt = -Infinity, cx = 3, pageAt = 0;
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
  let lastActivityAt = performance.now(), dreamRows = new Uint8Array(8), dreamSeedValue = 1;
  let dreamGenerationAt = 0, dreamCalm = true, dreamBright = [90,210,245], dreamDim = [28,82,135];
  let sleepStartedAt = 0, sleepPetUntil = 0, idleDreamReady = false;
  let dreamBaseSeed=1, dreamChapter=0, dreamChapterAt=0, dreamStillSteps=0;
  let sleepDreamVisible=false, sleepDreamViewAt=0;
  // A maquete encurta os tempos para dar para ver o efeito sem esperar no navegador.
  const IDLE_DREAM_AFTER = 12000, IDLE_DREAM_CYCLE = 20000, IDLE_DREAM_SHOW = 7000;
  const SLEEP_DREAM_AFTER = 8000, SLEEP_PET_REVEAL = 8000, DREAM_STEP = 500;
  function dreamHash(value) {
    value=Math.imul(value^(value>>>16),0x7FEB352D);
    value=Math.imul(value^(value>>>15),0x846CA68B);
    return (value^(value>>>16))>>>0;
  }
  function seedDream(now) {
    dreamCalm = !state.sick && !state.poop && state.hunger >= 50 && state.happy >= 50 && state.energy >= 50;
    dreamBaseSeed=(dna^(state.hunger<<24)^(state.energy<<16)^(state.happy<<8)^(state.poop<<4))>>>0;
    dreamChapter=0; startDreamChapter(now);
  }
  function startDreamChapter(now) {
    dreamSeedValue=dreamHash((dreamBaseSeed+Math.imul(dreamChapter,0x9E3779B9))>>>0);
    const palette=dreamChapter%3;
    dreamBright=(dreamCalm?[[90,210,245],[120,235,110],[195,138,245]]:[[255,112,96],[255,155,45],[245,95,165]])[palette];
    dreamDim=(dreamCalm?[[28,82,135],[28,90,45],[75,36,130]]:[[118,35,85],[105,42,15],[90,25,70]])[palette];
    dreamRows.fill(0);
    if (dreamCalm) {
      const pattern = dreamSeedValue % 4;
      const put = (x,y) => { dreamRows[y] |= 1 << x; };
      if (pattern === 0) { put(2,3); put(3,3); put(4,3); }
      else if (pattern === 1) { [[3,2],[4,2],[5,2],[2,3],[3,3],[4,3]].forEach(([x,y])=>put(x,y)); }
      else if (pattern === 2) { [[3,3],[4,3],[3,4],[4,4]].forEach(([x,y])=>put(x,y)); }
      else { [[3,2],[4,3],[2,4],[3,4],[4,4]].forEach(([x,y])=>put(x,y)); }
      const transformed=new Uint8Array(8);
      for(let y=0;y<8;y++) for(let x=0;x<8;x++) if(dreamAlive(x,y)) {
        let nx=x,ny=y;
        for(let i=0;i<((dreamSeedValue>>>2)&3);i++) { const oldX=nx;nx=7-ny;ny=oldX; }
        nx=(nx+((dreamSeedValue>>>4)&7))%8;ny=(ny+((dreamSeedValue>>>7)&7))%8;
        transformed[ny]|=1<<nx;
      }
      dreamRows=transformed;
    } else {
      let rng = (dreamSeedValue ^ 0x9E3779B9) >>> 0;
      if (!rng) rng = 0xA341316C;
      for (let y=0;y<8;y++) for (let x=0;x<8;x++) {
        rng ^= rng << 13; rng ^= rng >>> 17; rng ^= rng << 5; rng >>>= 0;
        if (rng % 100 < 18) dreamRows[y] |= 1 << x;
      }
    }
    if(!dreamRows.some(Boolean)) dreamRows[3]=1<<3;
    dreamChapterAt=dreamGenerationAt=now; dreamStillSteps=0;
  }
  function dreamAlive(x,y) { return (dreamRows[y] & (1 << x)) !== 0; }
  function stepDream() {
    const next = new Uint8Array(8);
    for (let y=0;y<8;y++) for (let x=0;x<8;x++) {
      let n=0;
      for (let dy=-1;dy<=1;dy++) for (let dx=-1;dx<=1;dx++) if (dx||dy)
        n += Number(dreamAlive((x+dx+8)%8,(y+dy+8)%8));
      if (n===3 || (dreamAlive(x,y) && n===2)) next[y] |= 1 << x;
    }
    dreamRows = next;
  }
  function drawDream(buf, now, bird=false, freeOnly=false) {
    if(now-dreamGenerationAt>=DREAM_STEP) {
      dreamGenerationAt=now;
      const before=dreamRows.join(','); stepDream();
      dreamStillSteps=before===dreamRows.join(',')?dreamStillSteps+1:0;
      if(!dreamRows.some(Boolean)||now-dreamChapterAt>=30000||(dreamStillSteps>=8&&now-dreamChapterAt>=20000)) {
        dreamChapter++; startDreamChapter(now);
      }
    }
    for (let y=0;y<8;y++) for (let x=0;x<8;x++) if (dreamAlive(x,y))
      if(freeOnly) setFree(buf,x,y,((x+y)&1)?dreamBright:dreamDim);
      else buf[y*8+x] = ((x+y)&1) ? dreamBright : dreamDim;
    if (bird) {
      const x=8-Math.floor(now/150)%12, y=1+Math.floor(now/700)%3, c=[205,228,255];
      if(freeOnly) { setFree(buf,x,y+1,c);setFree(buf,x+1,y,c);setFree(buf,x+2,y+1,c); }
      else {
        if (x>=0&&x<8) buf[(y+1)*8+x]=c;
        if (x+1>=0&&x+1<8) buf[y*8+x+1]=c;
        if (x+2>=0&&x+2<8) buf[(y+1)*8+x+2]=c;
      }
    }
    return buf;
  }
  function markActivity(now=performance.now()) { lastActivityAt=now; idleDreamReady=false; }
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
    screen = 'life'; action = ''; sleepByGesture = false; playedAt = -Infinity; cx = 3;
    markActivity(); sleepDreamVisible=false;
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
        if (ok) { add('hunger', 30); action = 'comendo'; }
        break;
      case 1:
        ok = !state.asleep && state.energy >= 10;
        if (ok) { add('happy', 20); add('energy', -8); add('hunger', -3); action = 'brincando'; }
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
      const w = spriteW(animFrame(pet.id + '_idle', 0));
      cx = dir < 0 ? Math.floor(w / 2) : N - 1 - (w - 1 - Math.floor(w / 2));
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
      if (reveal) { buf = pose.dormindo(t); sleepDreamVisible=false; }
      else {
        if(!sleepDreamVisible) { sleepDreamVisible=true;sleepDreamViewAt=now; }
        const world=drawDream(b,now), elapsed=now-sleepDreamViewAt;
        buf=world; fullDream=true;
        if(elapsed<1200) {
          buf=pose.dormindo(t);
          const stage=Math.floor(elapsed*4/1200);
          setFree(buf,0,0,dreamBright);
          if(stage) { setFree(buf,1,0,dreamBright);setFree(buf,0,1,dreamDim); }
          for(let y=0;y<8;y++) for(let x=0;x<8;x++) if((x+2*y)%4<stage) buf[y*8+x]=world[y*8+x];
          if(!buf.some(Boolean)) buf=world;
        }
      }
    } else if (screen === 'life' && action && pose[action]) buf = pose[action](now - actionAt);
    else if (screen === 'life' && action === 'recusa') blitA(b, animFrame(pet.id + '_sad', t), cx, 7);
    else if (screen === 'life' && action === 'limpar') { blitA(b, animFrame(pet.id + '_idle', t), cx, 7); blitFree(b, animFrame('fx_sparkle_anim', t), Math.floor((now-actionAt)/140)-2, 5); }
    else if (screen === 'life' && action === 'remedio') buf=careFrame(pet,'remedio',now-actionAt);
    else if (screen === 'life' && state.sick) buf = pose.doente(t);
    else if (screen === 'life' && state.hunger < 25) buf = pose['com fome'](t);
    else if (screen === 'life' && state.energy < 25) buf = pose.cansado(t);
    else if (screen === 'life' && state.happy < 25) buf = pose.triste(t);
    else if (screen === 'life') {
      const idle=now-lastActivityAt, behavior=(Math.floor(idle/7000)+(dna&7))%5;
      if(idle<3000) blitA(b,animFrame(pet.id+((t%3800<150)?'_blink':'_idle'),t),cx,7);
      else if(pet.id==='capy'&&behavior===1) {
        blitA(b,animFrame('capy_eat',t),cx,7);if(Math.floor(t/450)%2===0) setFree(b,cx+2,1,[130,175,100]);
      } else if(behavior===2) {
        blitA(b,animFrame(pet.id+'_idle',t),cx,7,pet.side&&Math.floor(t/900)%2===1);
        if(pet.id==='cat') setFree(b,Math.floor(now/350)%8,0,[205,228,255]);
      } else buf=pose.passeando(idle);
    }
    if(screen==='life'&&!state.asleep&&!action&&state.energy>=25&&urgentNeed()<0) {
      const idle=now-lastActivityAt;
      if(idle>=IDLE_DREAM_AFTER&&(idle-IDLE_DREAM_AFTER)%IDLE_DREAM_CYCLE<IDLE_DREAM_SHOW) {
        if(!idleDreamReady) { seedDream(now);idleDreamReady=true; }
        const bird=Math.floor((idle-IDLE_DREAM_AFTER)/IDLE_DREAM_CYCLE)%2===1;
        drawDream(buf,now,bird,true);
      }
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
