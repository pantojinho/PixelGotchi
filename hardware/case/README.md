# Case do PixelGochi

Case impressa em 3D para a Waveshare ESP32-S3-Matrix: **28,7 × 28,7 × 8,6 mm**
(11 mm na versão com difusor), com acesso ao USB-C, aos dois botões de
trás (BOOT e RESET) e argolinha de chaveiro.

![Explodida](images/exploded.png)

| Frente (aberta) | Verso |
|---|---|
| ![Frente](images/assembly_front.png) | ![Verso](images/assembly_back.png) |

## Peças

| Arquivo | Quantidade | Imprimir |
|---|---|---|
| `stl/front_open.stl` **ou** `stl/front_diffuser.stl` | 1 | face da frente na mesa |
| `stl/back.stl` | 1 | já vem com o lado de fora na mesa |
| `stl/pin.stl` | 2 (+1 de reserva) | já vem de cabeça pra baixo |

- **front_open**: janela aberta, os LEDs aparecem direto. Imprime em qualquer cor.
- **front_diffuser**: face fina (0,6 mm) na frente e grade 8×8 por dentro, cada
  LED vira um "pixel" quadradinho, estilo Tamagotchi. Imprima em **PLA branco ou
  natural** (a luz precisa atravessar).

## Como os botões funcionam

Cada botão tem um **pino solto** que atravessa a tampa e só encosta no botão da
placa. Apertar o pino aperta o botão. Como o pino pode deslizar pra fora, ele
**nunca fica apertando sozinho** — importante, porque BOOT apertado ao ligar
entra em modo de gravação e RESET apertado trava a placa. Uma aba no pino
impede que ele caia pra fora.

## Configuração na Ender-3 V3 KE (Creality Print)

| Ajuste | Case (front/back) | Pinos |
|---|---|---|
| Material | PLA | PLA |
| Camada | 0,16–0,2 mm | 0,12 mm |
| Paredes | 4 (1,6 mm) | 3 |
| Preenchimento | 20% | 100% |
| Suporte | **não** | **não** |
| Aderência | nenhuma/saia | **brim** (peça minúscula) |

Dicas:
- Imprima os pinos junto com a case (ou vários de uma vez) pra cada camada ter
  tempo de esfriar; sozinhos eles derretem.
- Na versão difusor, a face de 0,6 mm são 3–4 camadas: deixe o primeiro layer
  bem calibrado. Mais fina = mais luz; se ficar translúcida demais, suba
  `diff_t` pra 0,8.
- Se a tampa entrar muito justa ou muito solta, ajuste `plate_fit` (0,12 mm por
  lado) — cada impressora tem uma tolerância.

## Montagem

1. Coloque a placa **com os LEDs pra baixo** dentro da frente, com o USB-C
   alinhado ao rasgo de cima. Ela apoia na borda da janela.
2. Coloque os dois pinos em cima dos botões (ponta fina pra baixo). Em pé na
   tampa: o furo do **R** fica sobre o RESET e o do **B** sobre o BOOT.
3. Encaixe a tampa por trás, passando os pinos pelos furos, até clicar.
4. Pra abrir: unha ou chave fina no entalhe de baixo.

## Medidas e o que ainda é estimativa

Vieram do desenho cotado da Waveshare e de fotos da placa (usando o passo de
2,54 mm dos pinos como régua). Estão no topo de `pixelgochi_case.scad`:

| Medida | Valor | Confiança |
|---|---|---|
| Placa, cantos | 25,00 × 25,00 mm, R1 | alta (cotado) |
| USB-C | centralizado, passa ~0,8 mm da borda, 3,26 mm de altura | média |
| Botões | 4,6 mm das laterais, 3,6 mm do topo | média (±0,5 mm) — o pino tem ponta de 1,8 mm pra tolerar |
| Altura do botão | ~2,0 mm | baixa — o pino tolera de 1,5 a 2,5 mm |
| Espessura da placa | 1,6 mm | média — se ficar folgada, um pedacinho de fita resolve |
| Passo dos LEDs | ~2,7 mm | média — **só importa pra grade da versão difusor** |

Se algo não encaixar, meça com régua/paquímetro, ajuste o valor no `.scad` e
gere de novo.

## Gerar os STLs

Precisa do [OpenSCAD](https://openscad.org/downloads.html):

```bash
python tools/build_case.py
```

Ele confere **colisões** (uma placa simplificada não pode encostar em nenhuma
peça), gera os 4 STLs em `stl/` e as imagens em `images/`. Pra ver/ajustar ao
vivo, abra `pixelgochi_case.scad` no OpenSCAD e troque `PART`
(`"assembly"`, `"exploded"`, `"front"`, `"back"`, `"pin"`), `STYLE` e `KEYCHAIN`.
