# Montar o seu PixelGotchi em casa

[README](../README.md) · [Montar o seu](GUIA-MONTAGEM.md) · [Como jogar](JOGO.md) · [Editor](EDITOR.md) · [Hardware](HARDWARE.md) · [Desenvolvimento](DESENVOLVIMENTO.md) · [Prompts para IA](PROMPTS-IA.md) · [English](en/BUILD-GUIDE.md)

Do zero até o bichinho chocando na sua mão. A versão básica **não precisa de
solda, fios nem programação**: é uma plaquinha pronta, um cabo e o navegador.

| Etapa | Precisa de | Tempo |
|---|---|---|
| 0. Experimentar antes de comprar | Navegador | 5 min |
| 1. Comprar a placa | — | prazo da loja |
| 2. Gravar o jogo | Computador com Chrome/Edge e cabo USB-C de dados | 5 min |
| 3. Chocar o ovo | Mexer a placa com calma | 5 min |
| 4. Case impressa (opcional) | Impressora 3D ou serviço de impressão | ~1 h de impressão |
| 5. Bateria (opcional, avançado) | Solda e alguns componentes | 1–2 h |
| 6. Bichinho seu (opcional) | O editor no navegador | 15 min ou mais |

## 0. Experimente antes de comprar

Abra o [simulador](https://pantojinho.github.io/PixelGotchi/). Ele usa os
mesmos desenhos do firmware: escolha o bichinho, clique em **BOOT** para
alimentar, segure e solte para abrir o menu, use os botões de inclinar,
chacoalhar e virar. Os tempos são encurtados para você ver sonhos e
acontecimentos sem esperar.

## 1. O que comprar

| Item | Obrigatório? | Onde encontrar |
|---|---|---|
| **Waveshare ESP32-S3-Matrix** (placa 25 × 25 mm com matriz 8×8 RGB, botão BOOT e sensor QMI8658) | Sim | [Loja Waveshare](https://www.waveshare.com/esp32-s3-matrix.htm) · [AliExpress](https://www.aliexpress.com/w/wholesale-waveshare-esp32%2Ds3%2Dmatrix.html) · [Mercado Livre](https://lista.mercadolivre.com.br/esp32-s3-matrix) |
| **Cabo USB-C com dados** | Sim | Muitos cabos só carregam. Se o computador não "ver" a placa, troque o cabo primeiro. [Mercado Livre](https://lista.mercadolivre.com.br/cabo-usb-c-dados) |
| Carregador USB de celular ou power bank | Para usar longe do computador | O que você já tiver |
| Case impressa em 3D | Não | Arquivos prontos no projeto (etapa 4) |
| Peças da versão com bateria | Não | [Lista na documentação da case](../hardware/case/README.md#lista-de-compras) |

**Confira o modelo antes de pagar.** Existem várias placas "ESP32 + matriz
8×8" com pinagem diferente. O anúncio precisa dizer **Waveshare
ESP32-S3-Matrix** e as fotos precisam bater com a frente e o verso abaixo.
Os links de AliExpress e Mercado Livre são buscas: escolha o vendedor.

![Frente e verso da Waveshare ESP32-S3-Matrix, com matriz, USB-C, BOOT e RESET](images/hardware-components.png)

Mais detalhes (pinos, dimensões, brilho): [Hardware](HARDWARE.md) e a
[documentação oficial da Waveshare](https://docs.waveshare.com/ESP32-S3-Matrix).

## 2. Gravar o jogo pelo navegador

1. No computador, abra o [instalador](https://pantojinho.github.io/PixelGotchi/install.html)
   no **Chrome ou Edge** (Firefox, Safari e celular não acessam USB).
2. Conecte a placa com o cabo de dados.
3. Marque o aviso (a gravação apaga o que estiver na placa), clique no botão,
   escolha a porta da placa e depois **Install**.
4. Quando terminar, toque em **RESET** na placa.

Não apareceu nenhuma porta? Troque o cabo, feche programas que usam portas
seriais e tente o modo de gravação: segure **BOOT**, toque em **RESET**,
solte **BOOT** e escolha a porta de novo. Mais opções em [Instalar](INSTALAR.md).

## 3. Primeira vez: escolher e chocar

1. A placa mostra um bichinho. **Clique** no BOOT (ou incline) para trocar
   entre capivara, gato, sapo, pintinho, coelho e axolote.
2. **Segure o BOOT por 0,6 s e solte** para escolher. Aparece um ovo.
3. **Mexa a placa com calma**: o ovo choca com 5 minutos de movimento
   acumulado. Parar só pausa; clique no BOOT para ver a barra de progresso.
4. O bichinho nasce e diz o próprio nome, tirado do DNA dele.

Daí em diante: **clique = comida**, **segure e solte = menu**, **chacoalhar =
brincar**, **virar a tela para baixo = dormir**. Tudo em [Como jogar](JOGO.md).
O pet fica salvo mesmo se a placa desligar; o relógio dele só anda ligado.

## 4. Case impressa em 3D (opcional)

Uma case de **28,7 × 28,7 × 8,6 mm** com rasgo para o USB-C, pinos que apertam
o BOOT e o RESET por trás e argolinha de chaveiro. Imprime **sem suporte**.

![Case explodida](../hardware/case/images/exploded.png)

**Sem impressora?** Serviços de impressão 3D, makerspaces e escolas técnicas
imprimem peças pequenas assim; envie os arquivos `.stl` abaixo e peça PLA.

| Arquivo | Quantas | Observação |
|---|---|---|
| [`front_open.stl`](../hardware/case/stl/front_open.stl) **ou** [`front_diffuser.stl`](../hardware/case/stl/front_diffuser.stl) | 1 | Janela aberta (qualquer cor) ou difusor com grade 8×8 (PLA branco/natural, efeito "pixel") |
| [`back.stl`](../hardware/case/stl/back.stl) | 1 | Tampa de trás |
| [`pin.stl`](../hardware/case/stl/pin.stl) | 2 (+1 reserva) | Pinos dos botões; imprima junto com a case e use brim |

Configuração sugerida em qualquer fatiador ([PrusaSlicer](https://www.prusa3d.com/page/prusaslicer_424/),
[OrcaSlicer](https://www.orcaslicer.com/), Cura, Creality Print): PLA,
camada 0,16–0,2 mm (pinos 0,12 mm), 4 paredes, 20% de preenchimento (pinos
100%), sem suporte. Valores testados na Ender-3 V3 KE e dicas de ajuste:
[documentação da case](../hardware/case/README.md).

### Montagem

1. Placa **com os LEDs para baixo** dentro da frente, USB-C alinhado ao rasgo.
2. Os dois pinos sobre os botões, ponta fina para baixo: **R** sobre RESET,
   **B** sobre BOOT.
3. Encaixe a tampa por trás até clicar. Para abrir: unha no entalhe de baixo.
4. Ligue e confira: a placa **não** pode reiniciar nem entrar em modo de
   gravação sozinha. Se entrar, um pino está comprido demais: lixe a ponta ou
   aumente `sw_h` em `hardware/case/placa.scad` (encurta o pino) e gere de novo.

As medidas vieram do desenho da Waveshare e de fotos; a primeira impressão é
um teste de encaixe. Para ajustar folgas e gerar novos STLs com o OpenSCAD,
veja a [documentação da case](../hardware/case/README.md#gerar-os-stls).

## 5. Versão com bateria (opcional, avançado)

Case de 28,7 × 47,7 × 18,8 mm com bateria LiPo de 160 mAh, carregador TP4056
e chave liga/desliga, carregando pelo mesmo USB-C. **Exige solda nos pads da
placa e a troca de um resistor do carregador**; ligar errado pode danificar a
placa ou a bateria. Autonomia estimada: ~1,5 h. Siga a
[lista de compras, ligação e montagem](../hardware/case/README.md#versão-com-bateria)
e teste tudo fora da case antes de fechar.

## 6. Crie o seu bichinho (opcional)

No [editor do site](https://pantojinho.github.io/PixelGotchi/editor.html)
você parte de um dos seis bichinhos, redesenha as poses, testa no simulador
e envia para a placa pelo mesmo cabo USB, sem compilar. Ele vira a 7ª
espécie ou substitui o pet atual por um ovo dele. Passo a passo em
[Editor](EDITOR.md).

## Deu problema?

| Sintoma | O que fazer |
|---|---|
| O navegador não mostra porta | Cabo de dados, outra porta USB, fechar monitores seriais, BOOT + RESET |
| Gravou mas a matriz não acende | Toque em RESET com BOOT solto; confira se a placa é mesmo a ESP32-S3-Matrix |
| Imagem de ponta-cabeça ou inclinação invertida | Outra revisão da placa: ajuste em [Hardware](HARDWARE.md) |
| Gestos não funcionam, BOOT sim | O sensor QMI8658 não respondeu; confira o modelo exato |
| Muito claro ou cores misturadas | O brilho já é baixo (5/255); veja o perfil de LEDs em [Hardware](HARDWARE.md) |
| A placa esquenta | Desconecte. O firmware limita brilho e corrente; não aumente esses limites |

Não resolveu? Use o [prompt de instalação assistida](PROMPTS-IA.md#1-instalar-na-placa-com-um-agente-local)
com um agente de IA no seu computador, ou abra uma
[issue](https://github.com/pantojinho/PixelGotchi/issues) com fotos da placa
e a mensagem de erro.
