# Rodar localmente, testar e publicar

[README](../README.md) · [Montar o seu](GUIA-MONTAGEM.md) · [Como jogar](JOGO.md) · [Editor](EDITOR.md) · [Hardware](HARDWARE.md) · [Desenvolvimento](DESENVOLVIMENTO.md) · [Prompts para IA](PROMPTS-IA.md) · [English](en/DEVELOPMENT.md)

Este guia é para quem quer rodar o site no próprio computador, mudar a arte
ou o código, rodar os testes ou publicar uma cópia do projeto.

## Site no ar ou cópia local?

São **os mesmos arquivos** (`preview/`). O GitHub Actions publica essa pasta em
[pantojinho.github.io/PixelGotchi](https://pantojinho.github.io/PixelGotchi/)
a cada push no `master`, com o firmware já compilado para o instalador.

| Você quer… | Use |
|---|---|
| Ver o simulador, jogar e gravar a placa com o jogo oficial | O **site no ar**: nada para instalar |
| Mudar desenhos, cores, regras ou código e ver o resultado na hora | A **cópia local** (abaixo) |
| Gravar na placa uma versão que você modificou | Cópia local + PlatformIO ([Instalar](INSTALAR.md#instalação-manual-via-usb)), ou o seu fork publicado (abaixo) |

O instalador USB funciona nos dois casos porque o navegador libera o Web
Serial em páginas HTTPS **e** em `localhost`.

## Rodar a página no seu computador

Precisa só de [Python 3](https://www.python.org/downloads/) (no Windows, marque
**Add Python to PATH** na instalação). Baixe o projeto com
`git clone https://github.com/pantojinho/PixelGotchi.git` ou, sem Git, em
**Code → Download ZIP** no GitHub, e extraia.

**Jeito fácil:** dê dois cliques em `iniciar.bat` (Windows) ou rode
`./iniciar.sh` no terminal (macOS/Linux). O navegador abre na página
**Comece aqui** do simulador.

**Pelo terminal**, na pasta do projeto:

```sh
python tools/preview.py
```

Isso serve `preview/` em `http://localhost:8765/`, abre o navegador e
**regenera e recarrega a página quando você salva um arquivo em `art/`**.
Outra porta: `python tools/preview.py 9000`. Pare com `Ctrl+C`.

- **Maquete interativa:** escolha a espécie e experimente BOOT, inclinação,
  chacoalhada e sono. **Espaço** funciona como BOOT; as **setas** inclinam.
  O seletor de necessidade permite experimentar fome, tristeza, cansaço,
  sujeira e doença.
- **Galeria:** cenas e todas as poses animadas; clique na matriz para ampliar.
- **Modo LED:** aproxima a aparência com brilho baixo; modo de design
  mostra a paleta original. O brilho do preview não altera o firmware.
- Filtros: `?only=cat`, `?only=capy`, `?view=scenes`, `?view=sprites`.

A maquete usa os mesmos sprites e reproduz os controles dos cuidados.
Ela não executa o firmware nem simula seu relógio, DNA, nascimento,
descuido ou persistência. A aparência óptica real depende dos LEDs.

### Instalador USB local, com o seu firmware

O site local não tem firmware compilado (binários não entram no Git). Para
gravar a sua versão pelo navegador, compile primeiro. Precisa do PlatformIO
([como instalar](INSTALAR.md#1-baixe-o-projeto-e-as-ferramentas)):

```sh
python tools/build_web_installer.py
python tools/preview.py
```

Abra `http://localhost:8765/install.html`. O primeiro comando gera binário,
manifesto e informações do firmware em `preview/firmware/`.

## Publicar a sua cópia (fork) com site próprio

1. No GitHub, clique em **Fork**.
2. No seu fork: **Settings → Pages → Source: GitHub Actions**.
3. **Actions → Publish web preview and USB installer → Run workflow**.

O site fica em `https://SEU-USUARIO.github.io/PixelGotchi/`. Daqui em diante,
cada push no `master` do fork gera a arte, compila o firmware **na nuvem** e
publica o simulador e o instalador com as suas mudanças. Assim dá para editar
`art/*.art` pelo próprio site do GitHub (ícone de lápis) e gravar na placa sem
instalar compilador nenhum. O teste `verify` vai apontar que os arquivos
gerados (`src/art/ArtData.*`, `preview/art.js`) ficaram desatualizados no
repositório; isso não impede a publicação, mas para manter o fork limpo rode
`python tools/gen_art.py` e faça commit das saídas. Se o Pages estiver
desativado, o workflow falha avisando a configuração que falta.

## Testes sem placa

Mesmos comandos da CI (`.github/workflows/verify.yml`), na raiz do projeto:

```sh
python tools/gen_art.py
git diff --exit-code -- src/art/ArtData.h src/art/ArtData.cpp src/art/LedProfile.h preview/art.js
python tools/test_controls.py
node test/test_preview.cjs
node test/test_petpack.cjs
node --experimental-vm-modules test/test_installer.cjs
python -m unittest discover -s test -p test_web_installer.py
python -m platformio run -e esp32-s3-matrix
```

| Precisa de | Para |
|---|---|
| Python 3.12 | gerador de arte, servidor local, teste do instalador |
| `g++` ou `clang++` (ou `CXX`; ou `ZIG_BINARY` com o Zig) | testes C++ (`test_controls.py`) |
| Node.js 22 | testes da maquete, do pacote do editor e do instalador |
| PlatformIO 6.2.0 | build do firmware ([instalação](INSTALAR.md#1-baixe-o-projeto-e-as-ferramentas)) |

No Windows, o compilador C++ mais simples é o do
[MSYS2](https://www.msys2.org/) (`pacman -S mingw-w64-ucrt-x86_64-gcc`) ou o
Zig. No macOS, `xcode-select --install`. No Linux, o pacote `g++`.

- **`test_controls.py`** executa o C++ real de Game, Input, Imu, PetSim,
  Canvas, Display, Dream e arte, trocando só relógio, GPIO, sensor, NVS e
  saída LED. Cobre brilho/gamma, contraste da capivara nos 64 tons de DNA,
  BOOT (debounce, segurar, reset), IMU (histerese, gestos), sono, menu, ovo,
  poses sem corte, refeição nas seis espécies, capítulos do sonho, Conway
  e o bichinho do editor (protocolo USB, erros, NVS, adoção e troca ao vivo).
- **`test_preview.cjs`** roda os renderizadores reais da maquete com relógio
  simulado: idle sem apagão, refeição, cuidados, 3 min de sonho, movimento e BOOT.
- **`test_petpack.cjs`** confere o pacote do editor para os seis modelos,
  a validação, a conversão para o simulador e a exportação `.art` (rodando o
  gerador real numa cópia temporária). Ele também confere as fixtures de
  `test/fixtures/`, que o teste C++ carrega no firmware: se mudar o formato,
  rode `node test/test_petpack.cjs --update` e revise o teste C++.
- **`test_installer.cjs`** e **`test_web_installer.py`** conferem o pacote do
  instalador, o SHA-256 e o bloqueio quando algo não bate.

O GitHub Actions roda tudo isso a cada push e pull request. Resultados e o
roteiro de teste na placa física estão em [TESTES.md](TESTES.md).

## Editar a arte

Para desenhar sem mexer em código, use o [editor](EDITOR.md); o botão
**Exportar .art** gera o texto abaixo para incluir o bichinho no repositório.

Edite **`art/*.art`** e execute `python tools/gen_art.py`. Não edite
`src/art/ArtData.*` ou `preview/art.js` manualmente. Veja o formato e os
critérios para LEDs em [art/README.md](../art/README.md).

```text
art/             sprites, paletas, efeitos, fonte e perfil de cor dos LEDs
tools/           gerador da arte, servidor local, instalador, release e testes
preview/         site: guia, simulador, editor e instalador (i18n.js, petpack.js)
src/Game.*       cenas, entrada e animações
src/PetSim.*     estado e regras
src/Dream.*      autômato de Conway dos sonhos
src/CustomPet.*  bichinho do editor: pacote PGP1 e protocolo USB
src/Input.*      botão BOOT com debounce
src/Imu.*        leitura e gestos do acelerômetro
src/Canvas.*     composição em 8×8
src/Display.*    saída FastLED, orientação e limites
src/Storage.*    persistência NVS
test/            testes C++, JS e Python, mocks de hardware e fixtures
```

## Pacote de release

`python tools/package_release.py v0.1.0` gera `dist/PixelGotchi-v0.1.0.zip`
com imagem única, binários com offsets, STLs da case e `SHA256SUMS.txt`.
Veja as pendências em [PROXIMA-SPRINT.md](PROXIMA-SPRINT.md#pendências-case-3d-e-primeira-release).
