// Pacote do editor (preview/petpack.js): formato, validação, simulador e .art.
// As fixtures em test/fixtures/ também são lidas pelo teste C++ do firmware,
// garantindo que navegador e placa entendem os mesmos bytes.
//   node test/test_petpack.cjs            (confere)
//   node test/test_petpack.cjs --update   (regrava as fixtures após mudar o formato)
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const vm = require('node:vm');
const assert = require('node:assert/strict');
const { execFileSync } = require('node:child_process');
const root = path.resolve(__dirname, '..');
const PetPack = require('../preview/petpack.js');
const ctx = { window: {} };
vm.createContext(ctx);
vm.runInContext(fs.readFileSync(path.join(root, 'preview/art.js'), 'utf8'), ctx);
const A = ctx.window.ART;

assert.equal(PetPack.crc32(Buffer.from('123456789')), 0xCBF43926); // vetor padrão do CRC-32

// Todos os bichinhos de fábrica servem de modelo e cabem no pacote.
for (const pet of A.pets) {
  const project = PetPack.fromArt(A, pet.id);
  const v = PetPack.validate(project);
  assert.deepEqual(v.errors, [], `${pet.id}: ${JSON.stringify(v.errors)}`);
  const bytes = PetPack.encode(project);
  assert(bytes.length <= PetPack.LIMITS.bytes, `${pet.id}: ${bytes.length} bytes`);
  assert.equal(Buffer.from(bytes.slice(0, 4)).toString(), 'PGP1');
}

// Fixtures lidas pelo firmware (test/test_controls.cpp).
const fixtures = path.join(root, 'test/fixtures');
for (const id of ['capy', 'cat']) {
  const hex = PetPack.toHex(PetPack.encode(PetPack.fromArt(A, id))) + '\n';
  const file = path.join(fixtures, `${id}.pgp.hex`);
  if (process.argv.includes('--update')) { fs.mkdirSync(fixtures, { recursive: true }); fs.writeFileSync(file, hex); }
  assert.equal(fs.readFileSync(file, 'utf8'), hex, `${id}: formato mudou; rode com --update e confira o teste C++`);
}

// No simulador, o pet convertido desenha como o de fábrica (capivara e gato
// têm todas as poses com a mesma largura, então o recorte não muda nada).
const rowsOf = (art, ref) => { const [name] = ref.split('@'); const s = art.sprites[name]; return { w: s.w, rows: s.rows, pal: art.palettes[s.pal] }; };
// Ancorado no chão: linhas vazias no topo não mudam o desenho.
const paint = ({ w, rows, pal }) => {
  const drawn = rows.slice(rows.findIndex(r => /[^.]/.test(r)));
  return drawn.map(r => [...r].map(ch => ch === '.' ? '.' : pal[ch]).join('|')).join('/') + `:${w}`;
};
for (const id of ['capy', 'cat']) {
  const art = PetPack.toArt(PetPack.fromArt(A, id), 'meu');
  for (const a of PetPack.ANIMS) {
    const ours = art.anims['meu_' + a], theirs = A.anims[id + '_' + a];
    assert.equal(ours.ms, theirs.ms);
    ours.frames.forEach((ref, i) => assert.equal(paint(rowsOf(art, ref)), paint(rowsOf(A, theirs.frames[i])), `${id}/${a}/${i}`));
  }
  assert.equal(art.pet.side, !!A.pets.find(p => p.id === id).side);
}

// Validação: cada problema tem um código que o editor traduz.
const codes = p => PetPack.validate(p).errors.map(e => e.code);
const blank = PetPack.blankProject();
assert.deepEqual(codes(blank), ['empty']);
const many = PetPack.fromArt(A, 'capy'); many.colors = new Array(16).fill('#112233');
assert(codes(many).includes('colors_max'));
const longName = PetPack.fromArt(A, 'capy'); longName.name = 'Capivarinha da Silva';
assert.deepEqual(codes(longName), ['name_long']);
const accent = PetPack.fromArt(A, 'capy'); accent.name = 'Capivará';
assert.deepEqual(codes(accent), ['name_chars']);
const noSleep = PetPack.fromArt(A, 'capy'); noSleep.anims.sleep.frames = [];
assert.deepEqual(codes(noSleep), ['anim_empty']);
const same = PetPack.fromArt(A, 'capy'); same.anims.eat.frames = [same.anims.idle.frames[0]];
assert(PetPack.validate(same).warnings.some(w => w.code === 'eat_same'));
assert.throws(() => PetPack.encode(blank), e => e.errors[0].code === 'empty');

// Exportação .art: o gerador real do repositório aceita o texto, numa cópia temporária.
const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'pixelgotchi-art-'));
try {
  for (const dir of ['art', 'tools', 'src/art', 'preview']) fs.cpSync(path.join(root, dir), path.join(tmp, dir), { recursive: true });
  const text = PetPack.toArtText(PetPack.fromArt(A, 'frog'), 'meusapo');
  fs.appendFileSync(path.join(tmp, 'art/pets.art'), '\n' + text);
  const out = execFileSync(process.env.PYTHON || 'python3', [path.join(tmp, 'tools/gen_art.py')], { encoding: 'utf8' });
  assert.match(out, /7 pets/);
} finally {
  fs.rmSync(tmp, { recursive: true, force: true });
}

console.log('PASS: pacote PGP1 dos 6 modelos, fixtures do firmware, simulador, validação e exportação .art');
