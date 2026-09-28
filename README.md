# PixelGotchi

[![verify](https://github.com/pantojinho/PixelGotchi/actions/workflows/verify.yml/badge.svg)](https://github.com/pantojinho/PixelGotchi/actions/workflows/verify.yml)

Um bichinho virtual de **64 pixels** que cabe num chaveiro. Roda na
**Waveshare ESP32-S3-Matrix** (25 × 25 mm, matriz RGB 8×8, botão BOOT e
acelerômetro) e funciona sozinho, sem Wi-Fi nem celular.
Você alimenta com um clique, brinca chacoalhando e põe para dormir virando a
placa. Ele tem nome próprio, personalidade tirada do DNA e sonha com o
[Jogo da Vida de Conway](https://pt.wikipedia.org/wiki/Jogo_da_vida).

**▶ [Experimente agora no navegador](https://pantojinho.github.io/PixelGotchi/guia.html)**:
simulador com os mesmos desenhos do firmware e instalador USB para a placa.

| Capivara | Gato |
|---|---|
| ![Capivara de perfil no simulador, com controles de BOOT e movimento](docs/images/system-capybara.jpg) | ![Gato sentado no simulador, com orelhas rosa, peito claro e cauda](docs/images/system-cat.jpg) |

## Escolha o seu caminho

| Eu quero… | Vá para |
|---|---|
| **Só ver e brincar**, sem comprar nada | [Simulador no navegador](https://pantojinho.github.io/PixelGotchi/) |
| **Montar o meu** do zero: o que comprar, gravar, chocar o ovo | [Guia de montagem](docs/GUIA-MONTAGEM.md) |
| **Já tenho a placa** e quero gravar o jogo | [Instalador pelo navegador](https://pantojinho.github.io/PixelGotchi/install.html) (Chrome/Edge + cabo USB-C) |
| **Imprimir a case** (chaveiro, difusor, versão com bateria) | [Case 3D](docs/GUIA-MONTAGEM.md#4-case-impressa-em-3d-opcional) · [arquivos e detalhes](hardware/case/README.md) |
| **Criar o meu próprio bichinho** | [Criar o seu bichinho](#criar-o-seu-bichinho) |
| **Rodar no meu computador**, mexer no código, rodar os testes | [Desenvolvimento](docs/DESENVOLVIMENTO.md) |
| **Pedir ajuda a uma IA** para instalar, desenhar ou montar | [Prompts prontos](docs/PROMPTS-IA.md) |
| Entender as regras, o menu e os sonhos | [Como jogar](docs/JOGO.md) |

## O que ele faz

- **Seis espécies**: capivara, gato, sapo, pintinho, coelho e axolote, cada
  uma com descanso, caminhada, comer, dormir, feliz, triste, fome e cansaço.
- **Nasce de um ovo** que choca com 5 minutos de movimento, e se apresenta
  com um **nome próprio** (KALU, MOBITE…) tirado do DNA dele.
- **Vive de verdade**: fome, alegria e energia caem com o tempo, ele fica
  sujo e doente se for esquecido e pode virar **selvagem**. Bem cuidado,
  vive para sempre. Tudo fica salvo mesmo desligando.
- **Tem vontades próprias**: fareja, observa o que passa, segue borboletas,
  cochila. Cuidados e cliques sempre têm prioridade.
- **Sonha**: dormindo, uma bolha sai da cabeça e vira um mundo de Conway que
  muda de capítulo a cada 20–40 s. Acordado e sozinho, gliders e blinkers
  aparecem ao redor dele.
- **Cuida dos olhos e da placa**: brilho limitado a 5/255, corrente a 400 mA
  e cores ajustadas para ficarem distintas nos LEDs.

## Do que você precisa

| Para | Precisa de |
|---|---|
| Simulador | Qualquer navegador moderno |
| Placa funcionando | [Waveshare ESP32-S3-Matrix](https://www.waveshare.com/esp32-s3-matrix.htm) ([AliExpress](https://www.aliexpress.com/w/wholesale-waveshare-esp32%2Ds3%2Dmatrix.html), [Mercado Livre](https://lista.mercadolivre.com.br/esp32-s3-matrix)), cabo USB-C **de dados** e um computador com Chrome ou Edge |
| Case | Impressora 3D (PLA, sem suporte) ou um serviço de impressão |
| Versão com bateria | LiPo 160 mAh, TP4056, chave e diodo, **mais solda**: [lista completa](hardware/case/README.md#lista-de-compras) |

Não precisa soldar nada nem ligar botões extras na versão básica. Confira o
modelo exato antes de comprar: há outras placas "ESP32 + matriz 8×8" com
pinagem diferente ([fotos e detalhes](docs/HARDWARE.md)).

## Como jogar, em 30 segundos

| Ação | O que acontece |
|---|---|
| **Clique** no BOOT | Dá comida (ou acorda, se estiver dormindo) |
| **Segure 0,6 s e solte** | Abre o menu já no cuidado mais urgente; no menu, clique troca e segurar confirma |
| **Chacoalhe** | Brinca |
| **Incline** | Ele vai para o lado mais baixo; no menu, troca o ícone |
| **Vire a tela para baixo** por 1,5 s | Dorme; desvirar acorda |
| **Segure 8 s** | Recomeça do zero (barra vermelha a partir de 3 s) |

Quando ele precisa de algo, o ícone do cuidado aparece de tempos em tempos e
um pontinho pisca na cor dele. Tabela completa, menu e status:
[Como jogar](docs/JOGO.md).

## Site no ar ou no seu computador?

Os dois usam **os mesmos arquivos** da pasta `preview/`:

- **[Site no ar](https://pantojinho.github.io/PixelGotchi/guia.html)**: não
  instala nada. Simulador, galeria de animações e instalador USB com o
  firmware mais recente, publicados automaticamente a cada atualização.
- **No seu computador**: baixe o projeto (`git clone` ou **Code → Download
  ZIP**), instale o [Python 3](https://www.python.org/downloads/) e dê dois
  cliques em `iniciar.bat` (Windows) ou rode `./iniciar.sh` (macOS/Linux).
  Abre a mesma página em `localhost`, que se atualiza sozinha quando você
  edita os desenhos. É o caminho para quem quer modificar o bichinho.

O navegador só libera o USB (Web Serial) em HTTPS ou `localhost`, então o
instalador funciona nos dois. Detalhes, publicação do seu próprio fork com
site e todos os testes: [Desenvolvimento](docs/DESENVOLVIMENTO.md).

## Criar o seu bichinho

**Hoje** os bichinhos são desenhos em texto (`art/*.art`), um caractere por
LED. O caminho é:

1. Desenhe à mão ou peça a uma IA com o
   [prompt de bichinho novo](docs/PROMPTS-IA.md#2-desenhar-um-bichinho-novo-chat-comum).
2. Cole no fim de `art/pets.art` e veja no simulador local: ele recarrega ao
   salvar e o gerador aponta qualquer erro de formato.
3. Grave na placa compilando no seu computador ([Instalar](docs/INSTALAR.md#instalação-manual-via-usb)),
   **ou** sem instalar compilador: faça um fork, ative o GitHub Pages e o seu
   site próprio compila e publica o instalador com o seu bichinho
   ([como](docs/DESENVOLVIMENTO.md#publicar-a-sua-cópia-fork-com-site-próprio)).

**Planejado:** um editor dentro do simulador, com modelo para começar,
desenho pixel a pixel, animações, salvar/abrir o projeto e **envio direto
para a placa pelo cabo USB**, sem compilar nada. O envio direto exige um
firmware que receba e guarde pacotes de arte; o plano está em
[Próxima sprint](docs/PROXIMA-SPRINT.md).

## Documentação

| Documento | Conteúdo |
|---|---|
| [Guia de montagem](docs/GUIA-MONTAGEM.md) | Compra, gravação, primeiro uso, case, bateria e problemas comuns |
| [Instalar](docs/INSTALAR.md) | Instalador web e instalação manual com PlatformIO, atualização |
| [Como jogar](docs/JOGO.md) | Espécies, controles, menu, status, sonhos, vida e persistência |
| [Hardware](docs/HARDWARE.md) | Placa, pinos, brilho e cor dos LEDs, calibração do sensor |
| [Case 3D](hardware/case/README.md) | STLs, impressão, montagem, versão com bateria |
| [Desenvolvimento](docs/DESENVOLVIMENTO.md) | Rodar local, editar arte, testes, publicar o fork, release |
| [Prompts para IA](docs/PROMPTS-IA.md) | Instalar, desenhar um bichinho, montar, mexer no código |
| [Arte para 64 LEDs](art/README.md) | Formato `.art` e critérios de desenho |
| [Testes](docs/TESTES.md) | Resultados e roteiros de teste na placa |
| [Animações e Conway](docs/ANIMACOES-E-CONWAY.md) · [Próxima sprint](docs/PROXIMA-SPRINT.md) | Planos de evolução |

## Estado do projeto

Verificado em software a cada push: testes do firmware com hardware
simulado, testes do simulador e do instalador, e build do ESP32-S3
([CI](https://github.com/pantojinho/PixelGotchi/actions)). **Ainda falta
ensaiar na placa física** a gravação pelo navegador, os gestos, a aparência
dos LEDs e a case; os roteiros estão em [Testes](docs/TESTES.md). Relatos de
quem montar são muito bem-vindos em
[issues](https://github.com/pantojinho/PixelGotchi/issues).

Os desenhos são originais, feitos diretamente na grade de pixels.
Referências de forma: [perfil de capivara (WWF)](https://www.wwf.or.jp/staffblog/news/5510.html)
e [silhueta de gato sentado](https://freesvg.org/black-cat-vector-image).

## Licença

MIT — [LICENSE](LICENSE).
