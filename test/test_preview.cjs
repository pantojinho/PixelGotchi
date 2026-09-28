// Executa os renderizadores reais e entradas da maquete, sem relógio ou USB reais.
const fs = require('node:fs');
const vm = require('node:vm');
const assert = require('node:assert/strict');
const path = require('node:path');
const root = path.resolve(__dirname, '..');
let now = 10000;
const elements = new Map();
const element = id => {
  if (!elements.has(id)) elements.set(id, { value:'', textContent:'', add(){}, getContext(){return {};}, setPointerCapture(){} });
  return elements.get(id);
};
const context = vm.createContext({
  window:{ addEventListener(){} }, document:{ getElementById:element, addEventListener(){} },
  performance:{now:()=>now}, location:{search:''}, URLSearchParams,
  Option:function(text,value){this.text=text;this.value=value;}, setTimeout:()=>1, clearTimeout(){}
});
vm.runInContext(fs.readFileSync(path.join(root,'preview/art.js'),'utf8'), context);
const html = fs.readFileSync(path.join(root,'preview/index.html'),'utf8');
const renderers = html.slice(html.indexOf('const A = window.ART;'),html.indexOf('const players = []'));
vm.runInContext(renderers+'\nconst players = [];', context);
vm.runInContext(fs.readFileSync(path.join(root,'preview/controls.js'),'utf8'), context);
const run = code => vm.runInContext(code, context);
const visible = frame => frame.some(p => p && p.some(v => v > 0));
const render = () => run(`players[0].fn(${now})`);
const choose = (id,value) => element(id).onchange({target:{value}});
const boot = () => element('boot').onclick({detail:0});

for (const pet of run('A.pets')) {
  choose('demo-pet',pet.id);
  for (let t=0;t<65000;t+=100) { now+=100; assert(visible(render()),`${pet.id}: idle apagou`); }
  choose('demo-need','normal'); boot();
  for(let t=0;t<3400;t+=100) { now+=100; assert(visible(render()),`${pet.id}: alimentação apagou`); }
  for(const kind of ['comendo','brincando','carinho','remedio']) {
    for(const t of [0,300,600,1000,1500,2500,2999]) {
      const result = run(`careFrame(A.pets.find(p=>p.id==='${pet.id}'),'${kind}',${t})`);
      assert(visible(result),`${pet.id}/${kind}/${t}`);
      const animation=kind==='comendo'?(t<600?'idle':t<2500?'eat':'happy'):
        kind==='remedio'?(t<800?'idle':'happy'):kind==='brincando'&&t<650?'idle':'happy';
      const bottom=kind==='brincando'&&t>=650&&t<2550?7-Math.floor(t/350)%2:7;
      const expected=run(`(()=>{const b=newBuf(), w=spriteW(animFrame('${pet.id}_idle',0));
        blitA(b,animFrame('${pet.id}_${animation}',${t}),Math.floor((8-w)/2)+Math.floor(w/2),${bottom});return b;})()`);
      expected.forEach((pixel,i)=>{if(pixel) assert.deepEqual(result[i],pixel,`${pet.id}/${kind}: efeito cobriu o pet`);});
    }
  }
  choose('demo-need','energy');
  now+=30001; assert(visible(render()));
  assert.match(element('demo-stats').textContent,/Dormindo/);
  const frames = new Set();
  for(let t=0;t<180000;t+=100) { now+=100; const f=render(); assert(visible(f)); frames.add(JSON.stringify(f)); }
  assert(frames.size>8,`${pet.id}: sonho não variou`);
  element('shake').onclick(); assert(visible(render()));
  assert.match(element('demo-stats').textContent,/Dormindo/);
  now+=9200; assert(visible(render()));
  boot(); render(); assert.doesNotMatch(element('demo-stats').textContent,/Dormindo/);
}
// A máscara de ambiente protege cada pixel existente, e não troca a pose base.
run(`const base = newBuf(); blitA(base,animFrame('capy_idle',0),3,7); const overlay=base.slice();
     for(let y=0;y<8;y++) for(let x=0;x<8;x++) setFree(overlay,x,y,[100,210,245]);`);
const [base,overlay] = run('[base,overlay]');
base.forEach((pixel,i)=>{if(pixel) assert.deepEqual(overlay[i],pixel);});
console.log('PASS: 6 pets, idle sem apagões, alimentação, cuidados, sonhos de 3 min, cochilo, movimento e BOOT');
