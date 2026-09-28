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

---

# Versão com bateria

Mesma largura, mais alta: **28,7 × 47,7 × 18,8 mm**. A tela fica em cima;
embaixo dela, o carregador; atrás da placa, uma prateleira e depois a bateria,
com a chave liga/desliga numa faixa ao lado. Carrega pelo **mesmo USB-C da
placa**: ligou o cabo, carrega e roda.

![Explodida com bateria](images/bateria_exploded.png)

| Frente | Verso |
|---|---|
| ![Frente](images/bateria_assembly_front.png) | ![Verso](images/bateria_assembly_back.png) |

## Peças impressas

| Arquivo | Quantidade | Imprimir |
|---|---|---|
| `stl/bateria_front_open.stl` **ou** `stl/bateria_front_diffuser.stl` | 1 | face da frente na mesa |
| `stl/bateria_mid.stl` | 1 | como vem (lado liso na mesa) |
| `stl/bateria_lid.stl` | 1 | como vem (lado de fora na mesa) |
| `stl/bateria_pin.stl` | 2 (+1 de reserva) | como vem, com brim |

Mesmas configurações de impressão da versão sem bateria.

## Lista de compras

| Peça | O que procurar | Observação |
|---|---|---|
| Bateria | a sua **HC 801723, 160 mAh** | Confira se não está estufada e se tem mais de ~3,0 V |
| Carregador | "módulo carregador **TP4056 com proteção** micro USB" (~25 × 18 mm) | **Precisa trocar um resistor** (abaixo). O simples de 22 × 17 mm também cabe, mas só use se a bateria tiver proteção própria |
| Chave | "chave deslizante **SS12D00**" (mini, 2 posições) | Corpo ~8,7 × 3,7 × 3,6 mm |
| Diodo | "diodo Schottky **1N5819**" (ou SS14) | Impede a bateria de receber os 5 V do USB sem controle |
| Resistor | **10 kΩ SMD 0603** (ou 0805, conforme o módulo) | Troca da corrente de carga |
| Conector | contraparte do conector da bateria (ou corte e emende) | |
| Fios | fio fino (30 AWG / wire-wrap), fita dupla face fina | |

### Corrente de carga (importante)

Os módulos TP4056 vêm ajustados para **1 A**, seis vezes mais que uma bateria
de 160 mAh aguenta. Troque o resistor **R3** (costuma estar marcado `122`, que é
1,2 kΩ) por um de **10 kΩ** (`103`): a carga cai para ~120 mA. **Não ligue a
bateria sem fazer essa troca.**

## Ligação

A placa não tem carregador: o USB entra por um diodo (D1) na linha de 5 V, que
alimenta os LEDs e o regulador de 3,3 V. A bateria entra nessa mesma linha,
pelo pad **5V** do verso, através de um diodo — assim o USB nunca carrega a
bateria "na força" (só o TP4056 carrega) e a bateria não volta para o cabo.

```
Pad 5V da placa ──┬──────────────── TP4056 IN+   (carrega quando o USB está ligado)
                  │
                  └──|◄── chave ──── TP4056 OUT+
                     1N5819: a faixa (cátodo) fica do lado do pad 5V

Pad GND da placa ────────────────── TP4056 IN−   (no módulo com proteção, IN− = OUT−)
Bateria + ── TP4056 B+             Bateria − ── TP4056 B−
```

No módulo **simples** (sem proteção) não existe OUT+: a chave vai no **B+**.

- Pads **5V** e **GND** ficam no verso da placa, perto do botão RESET (ver foto
  do verso no README principal).
- Nunca ligue a bateria direto no pad 5V (sem diodo) nem no pino **3V3**: com o
  USB ligado ela receberia carga sem controle, e no 3V3 os 4,2 V dela queimariam
  o ESP32 e o sensor.
- Com a chave **desligada** a bateria ainda carrega pelo USB; a chave só corta a
  alimentação da placa pela bateria.
- Autonomia estimada: **~1,5 h** (os 64 LEDs consomem mesmo apagados).

## Montagem

1. Faça a ligação acima **fora da case** e teste: com a chave ligada e sem USB,
   o PixelGochi deve ligar; com USB, o LED do TP4056 deve indicar carga.
2. Coloque a placa com os LEDs pra baixo no corpo (USB no rasgo de cima) e o
   TP4056 no compartimento abaixo dela (fita dupla face). O diodo e os fios dos
   pads ficam no vão atrás da placa.
3. Encaixe a prateleira (postes pra baixo, sobre os cantos da placa), passando os
   fios da bateria e da chave pelas aberturas.
4. Bateria na área da esquerda (olhando o verso), com os fios pra baixo; chave na
   faixa da direita, com a alavanca saindo pela lateral.
5. Pinos nos furos da prateleira (ponta fina pra baixo) e feche com a tampa até clicar.

## Medidas da bateria

A HC 801723 é 8,0 × 17 × 23 mm só a célula; com a plaquinha/fita da ponta costuma
passar de 27 mm. Meça a sua e ajuste `bat_l`, `bat_w`, `bat_t` em
`pixelgochi_case_bateria.scad` (o arquivo avisa se não couber) e gere de novo com
`python tools/build_case.py bateria`.
