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
  const C = { hunger:[255,58,74], bored:[154,92,255], poop:[127,216,255], sick:[62,214,76], energy:[255,214,46], happy:[255,111,168], care:[62,214,76] };
  const STATUS = [['stat_hunger','hunger','Fome'], ['stat_happy','happy','Alegria'], ['stat_energy','energy','Energia'], ['stat_care','care','Saúde']];
  const AGE_TEXT = '0 DIAS', PAGE_MS = 2200, AGE_MS = (textWidth(AGE_TEXT) + N + 2) * 100;
  function nextStatusPage() {
    page++; pageAt = performance.now();
    if (page > STATUS.length) { screen = 'life'; say('De volta ao bichinho.'); }
    else say(page < STATUS.length ? `Status: ${STATUS[page][2]}` : 'Idade — maquete: 0 dias.');
  }
  function alertColor() {
    const s = state;
    if (!(s.hunger < 25 || s.happy < 25 || s.sick || s.poop || (!s.asleep && s.energy < 15))) return null;
    return s.sick ? C.sick : s.poop ? C.poop : s.hunger < 25 ? C.hunger : s.energy < 25 ? C.energy : C.bored;
  }
  function reset() {
    state = { hunger:80, happy:80, energy:90, poop:0, sick:false, asleep:false };
    screen = 'life'; action = ''; sleepByGesture = false; playedAt = -Infinity; cx = 3;
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
        if (state.asleep && state.energy < 50) add('happy', -5);
        state.asleep = !state.asleep;
        break;
      case 5: ok = !state.asleep; if (ok) { add('happy', 5); action = 'carinho'; } break;
      case 6: screen = 'status'; page = 0; pageAt = performance.now(); deadline = Infinity; say('Status: Fome'); return;
      case 7: say('De volta ao bichinho.'); return;
    }
    actionAt = performance.now();
    if (!ok) action = 'recusa';
    say(ok ? `${labels[index]}: ${index === 4 ? (state.asleep ? 'boa noite!' : 'acordou!') : 'feito.'}` : 'Agora não: o bichinho não precisa desse cuidado.');
  }
  function input(long) {
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
  el('demo-pet').onchange = e => { pet = A.pets.find(p => p.id === e.target.value); pose = scenes(pet); reset(); };
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
    if (screen === 'menu') { item = (item + dir + icons.length) % icons.length; deadline = performance.now() + 8000; say(labels[item]); }
    else {
      const w = spriteW(animFrame(pet.id + '_idle', 0));
      cx = dir < 0 ? Math.floor(w / 2) : N - 1 - (w - 1 - Math.floor(w / 2));
      say('O bichinho acompanha a inclinação.');
    }
  }
  el('tilt-left').onclick = () => tilt(-1);
  el('tilt-right').onclick = () => tilt(1);
  el('shake').onclick = () => {
    if (screen !== 'life' || state.asleep || action || heldAt !== null) return;
    const now = performance.now();
    if (now - playedAt < 5000) { say('Dê uma pausa de 5 s entre brincadeiras.'); return; }
    playedAt = now; care(1);
  };
  el('face-down').onclick = () => {
    if (screen !== 'life' || state.asleep) return;
    state.asleep = true; sleepByGesture = true; action = '';
    say('Boa noite! Na placa, mantenha virado por 1,5 s.');
  };
  el('face-up').onclick = () => {
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
    let buf = b;
    if (screen === 'menu') {
      const { s } = frameRef(icons[item]);
      blit(b, icons[item], Math.floor((N - s.w) / 2), Math.floor((N - 1 - s.h) / 2));
      for (let x = 0; x < 8; x++) b[56 + x] = x === item ? [255,255,255] : [40,40,52];
    } else if (screen === 'status') {
      const pt = now - pageAt;
      if (heldAt === null && pt > (page < STATUS.length ? PAGE_MS : AGE_MS)) nextStatusPage();
      if (page === STATUS.length) scrollText(b, AGE_TEXT, pt, 1, [255,255,255], 100);
      else if (page < STATUS.length) {
        const [spr, key] = STATUS[page], v = key === 'care' ? 100 : state[key], col = C[key];
        if (v >= 25 || Math.floor(now / 300) % 2) blit(b, spr, Math.floor((N - frameRef(spr).s.w) / 2), 0);
        const full = Math.floor((v * 8 + 50) / 100), len = pt < 400 ? Math.floor(full * pt / 400) : full;
        for (let x = 0; x < 8; x++) for (let y = 6; y < 8; y++) b[y*8+x] = x < len ? col : col.map(c => Math.floor(c*30/255));
      }
    } else if (state.asleep) buf = pose.dormindo(t);
    else if (action && pose[action]) buf = pose[action](now - actionAt);
    else if (action === 'recusa') blitA(b, animFrame(pet.id + '_sad', t), cx, 7);
    else if (action === 'limpar') { blitA(b, animFrame(pet.id + '_idle', t), cx, 7); blit(b, animFrame('fx_sparkle_anim', t), Math.floor((now-actionAt)/140)-2, 5); }
    else if (action === 'remedio' && now-actionAt < 800) blit(b, 'fx_pill', 2, 3);
    else if (action === 'remedio') blitA(b, animFrame(pet.id + '_happy', t), cx, 7);
    else if (state.sick) buf = pose.doente(t);
    else if (state.hunger < 25) buf = pose['com fome'](t);
    else if (state.happy < 25) buf = pose.triste(t);
    else blitA(b, animFrame(pet.id + ((t % 3800 < 150) ? '_blink' : '_idle'), t), cx, 7);
    if (screen === 'life' && state.poop && action !== 'limpar') blit(buf, 'fx_poop', 5, 6);
    const alert = screen === 'life' && !action && alertColor();
    if (alert && Math.floor(now / 700) % 2) buf[7] = alert;
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
