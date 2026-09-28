# PixelGotchi

[Português](README.md) · **English**

[![verify](https://github.com/pantojinho/PixelGotchi/actions/workflows/verify.yml/badge.svg)](https://github.com/pantojinho/PixelGotchi/actions/workflows/verify.yml)

A **64-pixel** virtual pet that fits on a keychain. It runs on the
**Waveshare ESP32-S3-Matrix** (25 × 25 mm, 8×8 RGB matrix, BOOT button and
accelerometer) and works on its own, with no Wi-Fi or phone. You feed it with
a click, play by shaking it and put it to sleep by turning the board over. It
has its own name, a DNA-based personality, and dreams in
[Conway's Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life).
Made in Brazil 🇧🇷; the website and docs are in English and Portuguese.

**▶ [Try it now in your browser](https://pantojinho.github.io/PixelGotchi/guia.html?lang=en)**:
a simulator with the same drawings as the firmware, a pet editor and a USB
installer for the board.

| Capybara | Cat |
|---|---|
| ![Capybara in profile in the simulator, with BOOT and motion controls](docs/images/system-capybara.jpg) | ![Sitting cat in the simulator, with pink ears, light chest and tail](docs/images/system-cat.jpg) |

## Pick your path

| I want to… | Go to |
|---|---|
| **Just look and play**, without buying anything | [Simulator in the browser](https://pantojinho.github.io/PixelGotchi/?lang=en) |
| **Build my own** from scratch: what to buy, flash, hatch the egg | [Build guide](docs/en/BUILD-GUIDE.md) |
| **I already have the board** and want to flash the game | [Browser installer](https://pantojinho.github.io/PixelGotchi/install.html?lang=en) (Chrome/Edge + USB-C cable) · [release files](https://github.com/pantojinho/PixelGotchi/releases) |
| **Print the case** (keychain, diffuser, battery version) | [3D case](docs/en/BUILD-GUIDE.md#4-3d-printed-case-optional) · [files and details](hardware/case/README.en.md) |
| **Create my own pet** and send it to the board | [Pet editor](https://pantojinho.github.io/PixelGotchi/editor.html?lang=en) · [how it works](#create-your-own-pet) |
| **Run it on my computer**, change the code, run the tests | [Development](docs/en/DEVELOPMENT.md) |
| **Ask an AI for help** installing, drawing or building | [Ready-made prompts](docs/en/AI-PROMPTS.md) |
| **Contribute** to the project | [CONTRIBUTING](CONTRIBUTING.md#english) |
| Understand the rules, the menu and the dreams | [How to play](docs/en/GAME.md) |

## What it does

- **Six species**: capybara, cat, frog, chick, bunny and axolotl, each with
  idle, walking, eating, sleeping, happy, sad, hungry and tired animations.
  Plus **your own**, drawn in the editor.
- **Hatches from an egg** after 5 minutes of movement, and introduces itself
  with its **own name** (KALU, MOBITE…) derived from its DNA.
- **Really lives**: hunger, joy and energy drop over time, it gets dirty and
  sick if forgotten and can go **wild**. Well cared for, it lives forever.
  Everything stays saved even when powered off.
- **Has a will of its own**: sniffs, watches things go by, follows
  butterflies, naps. Care and clicks always come first.
- **Dreams**: asleep, a bubble rises from its head and becomes a Conway world
  that changes chapter every 20–40 s. Awake and alone, gliders and blinkers
  appear around it.
- **Easy on the eyes and on the board**: brightness capped at 5/255, current
  at 400 mA and colors tuned to stay distinct on the LEDs.

## What you need

| For | You need |
|---|---|
| Simulator and editor | Any modern browser |
| A working board | [Waveshare ESP32-S3-Matrix](https://www.waveshare.com/esp32-s3-matrix.htm) ([AliExpress](https://www.aliexpress.com/w/wholesale-waveshare-esp32%2Ds3%2Dmatrix.html), [Mercado Livre](https://lista.mercadolivre.com.br/esp32-s3-matrix)), a USB-C **data** cable and a computer with Chrome or Edge |
| Case | A 3D printer (PLA, no supports) or a printing service |
| Battery version | 160 mAh LiPo, TP4056, switch and diode, **plus soldering**: [full list](hardware/case/README.en.md#shopping-list) |

The basic version needs no soldering and no extra buttons. Check the exact
model before buying: other "ESP32 + 8×8 matrix" boards have different
pinouts ([photos and details](docs/en/HARDWARE.md)).

## How to play, in 30 seconds

| Action | What happens |
|---|---|
| **Click** BOOT | Feeds it (or wakes it up if asleep) |
| **Hold 0.6 s and release** | Opens the menu on the most urgent care; in the menu, click changes and hold confirms |
| **Shake** | Plays |
| **Tilt** | It walks to the lower side; in the menu, changes the icon |
| **Turn the screen face down** for 1.5 s | Sleeps; turning it back wakes it |
| **Hold 8 s** | Starts over (red bar from 3 s) |

When it needs something, the icon of the needed care shows up from time to
time and a small dot blinks in its color. Full table, menu and status:
[How to play](docs/en/GAME.md).

## Live website or on your computer?

Both use **the same files** from the `preview/` folder:

- **[Live website](https://pantojinho.github.io/PixelGotchi/guia.html?lang=en)**:
  nothing to install. Simulator, animation gallery, pet editor and USB
  installer with the latest firmware, published automatically on every update.
- **On your computer**: get the project (`git clone` or **Code → Download
  ZIP**), install [Python 3](https://www.python.org/downloads/) and
  double-click `iniciar.bat` (Windows) or run `./iniciar.sh` (macOS/Linux). The
  same pages open on `localhost` and reload by themselves when you edit the
  drawings. This is the path for changing the code.

Browsers only allow USB (Web Serial) on HTTPS or `localhost`, so the installer
and the editor work in both. Details, publishing your own fork with a site and
all the tests: [Development](docs/en/DEVELOPMENT.md).

## Create your own pet

Open the **[pet editor](https://pantojinho.github.io/PixelGotchi/editor.html?lang=en)**
in the browser, nothing to install:

1. Start from one of the six pets (or from scratch) and draw each pose on the
   real 8×8 grid: idle, blink, walk, eat, sleep, happy, sad, hungry and tired.
   Choose the colors, the favorite food and each animation's timing.
2. The check tells you what the board would not accept (too many colors,
   empty animation, accented name) and what will look odd (too wide to walk).
3. **Test in the simulator** shows the pet in the game, with the same controls.
4. **Send to the board** (Chrome/Edge + USB cable): it becomes the 7th species
   in the selection, or you can replace the current pet with an egg of it. No
   compiling; the board only needs the current firmware from the installer.

The project is saved in the browser and can be downloaded as a file. To add
the pet to the repository code, **Export .art** produces the text in the
`art/*.art` format ([how to build](docs/en/DEVELOPMENT.md#editing-the-art)).
You can also ask an AI for a drawing with the
[new pet prompt](docs/en/AI-PROMPTS.md#2-draw-a-new-pet-regular-chat).
Details in [Editor](docs/en/EDITOR.md).

## Documentation

| Document | Contents |
|---|---|
| [Build guide](docs/en/BUILD-GUIDE.md) | Buying, flashing, first use, case, battery and common problems |
| [Install](docs/en/INSTALL.md) | Web installer and manual install with PlatformIO, updating |
| [How to play](docs/en/GAME.md) | Species, controls, menu, status, dreams, life and saving |
| [Editor](docs/en/EDITOR.md) | Create a pet, test it and send it over USB; format and protocol |
| [Hardware](docs/en/HARDWARE.md) | Board, pins, LED brightness and color, sensor calibration |
| [3D case](hardware/case/README.en.md) | STLs, printing, assembly, battery version |
| [Development](docs/en/DEVELOPMENT.md) | Run locally, edit art, tests, publish your fork, release |
| [AI prompts](docs/en/AI-PROMPTS.md) | Install, draw a pet, build, change the code |
| [Art for 64 LEDs](art/README.en.md) | The `.art` format and drawing guidelines |
| [Contributing](CONTRIBUTING.md#english) | How to help, what can change and the checklist |
| [Tests](docs/TESTES.md) · [Animations and Conway](docs/ANIMACOES-E-CONWAY.md) · [Next sprint](docs/PROXIMA-SPRINT.md) | Test logs and plans (Portuguese) |

## Project status

**v0.1.0 — first release.** Verified in software on every push: firmware
tests with simulated hardware, simulator, editor package and installer tests,
and the ESP32-S3 build ([CI](https://github.com/pantojinho/PixelGotchi/actions)).
**Still to be tried on a physical board**: browser flashing, sending a pet over
USB, gestures, the look of the LEDs and the case; the test scripts are in
[Tests](docs/TESTES.md) (Portuguese). Reports from people who build one are
very welcome in [issues](https://github.com/pantojinho/PixelGotchi/issues),
in English or Portuguese.

The drawings are original, made directly on the pixel grid. Shape references:
[capybara profile (WWF)](https://www.wwf.or.jp/staffblog/news/5510.html)
and [sitting cat silhouette](https://freesvg.org/black-cat-vector-image).

## License

MIT — [LICENSE](LICENSE).
