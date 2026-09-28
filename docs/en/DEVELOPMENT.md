# Run locally, test and publish

[README](../../README.en.md) · [Build yours](BUILD-GUIDE.md) · [How to play](GAME.md) · [Editor](EDITOR.md) · [Hardware](HARDWARE.md) · [Development](DEVELOPMENT.md) · [AI prompts](AI-PROMPTS.md) · [Português](../DESENVOLVIMENTO.md)

This guide is for running the website on your own computer, changing the art
or the code, running the tests, or publishing your own copy of the project.
Want to contribute back? See [CONTRIBUTING](../../CONTRIBUTING.md#english).

## Live website or local copy?

They are **the same files** (`preview/`). GitHub Actions publishes that folder
at [pantojinho.github.io/PixelGotchi](https://pantojinho.github.io/PixelGotchi/)
on every push to `master`, with the firmware already built for the installer.

| You want to… | Use |
|---|---|
| Try the simulator, play, draw a pet in the editor and flash the official game | The **live website**: nothing to install |
| Change drawings, colors, rules or code and see the result right away | The **local copy** (below) |
| Flash a firmware you modified | Local copy + PlatformIO ([Install](INSTALL.md#manual-install-over-usb)), or your published fork (below) |

The USB installer and the editor work in both cases because browsers allow
Web Serial on HTTPS pages **and** on `localhost`.

## Run the website on your computer

You only need [Python 3](https://www.python.org/downloads/) (on Windows, tick
**Add Python to PATH** during setup). Get the project with
`git clone https://github.com/pantojinho/PixelGotchi.git` or, without Git,
**Code → Download ZIP** on GitHub, and extract it.

**Easy way:** double-click `iniciar.bat` (Windows) or run `./iniciar.sh` in a
terminal (macOS/Linux). The browser opens the **Start here** page.

**From the terminal**, in the project folder:

```sh
python tools/preview.py
```

This serves `preview/` at `http://localhost:8765/`, opens the browser and
**regenerates and reloads the page when you save a file in `art/`**. Another
port: `python tools/preview.py 9000`. Stop with `Ctrl+C`. Add `?lang=en` or
use the PT · EN switch to change the language.

- **Interactive mockup:** pick the species and try BOOT, tilting, shaking and
  sleeping. **Space** works as BOOT; the **arrows** tilt. The need selector
  lets you try hunger, sadness, tiredness, dirt and sickness.
- **Gallery:** scenes and every animated pose; click a matrix to zoom in.
- **LED mode:** approximates the look at low brightness; design mode shows the
  original palette. The simulator brightness does not change the firmware.
- Filters: `?only=cat`, `?only=capy`, `?view=scenes`, `?view=sprites`.

The mockup uses the same sprites and reproduces the care controls. It does
not run the firmware nor simulate its clock, DNA, hatching, neglect or
saving. The real optical look depends on the LEDs.

### Local USB installer with your firmware

The local website has no built firmware (binaries are not in Git). To flash
your version from the browser, build it first. It needs PlatformIO
([how to install](INSTALL.md#1-get-the-project-and-the-tools)):

```sh
python tools/build_web_installer.py
python tools/preview.py
```

Open `http://localhost:8765/install.html`. The first command writes the
binary, manifest and firmware info into `preview/firmware/`.

## Publish your copy (fork) with its own website

1. On GitHub, click **Fork**.
2. In your fork: **Settings → Pages → Source: GitHub Actions**.
3. **Actions → Publish web preview and USB installer → Run workflow**.

The site will be at `https://YOUR-USER.github.io/PixelGotchi/`. From then on,
every push to your fork's `master` generates the art, builds the firmware **in
the cloud** and publishes the simulator and the installer with your changes.
So you can edit `art/*.art` on the GitHub website itself (pencil icon) and
flash the board without installing any compiler. The `verify` check will
point out that the generated files (`src/art/ArtData.*`, `preview/art.js`)
are out of date in the repository; that does not block publishing, but to
keep the fork clean run `python tools/gen_art.py` and commit the outputs. If
Pages is disabled, the workflow fails naming the missing setting.

## Tests without a board

The same commands as CI (`.github/workflows/verify.yml`), from the project root:

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

| Needs | For |
|---|---|
| Python 3.12 | art generator, local server, installer test |
| `g++` or `clang++` (or `CXX`; or `ZIG_BINARY` with Zig) | C++ tests (`test_controls.py`) |
| Node.js 22 | simulator, editor package and installer tests |
| PlatformIO 6.2.0 | firmware build ([installation](INSTALL.md#1-get-the-project-and-the-tools)) |

On Windows, the simplest C++ compiler is [MSYS2](https://www.msys2.org/)'s
(`pacman -S mingw-w64-ucrt-x86_64-gcc`) or Zig. On macOS, `xcode-select
--install`. On Linux, the `g++` package.

- **`test_controls.py`** runs the real C++ of Game, Input, Imu, PetSim,
  Canvas, Display, Dream, CustomPet and art, replacing only the clock, GPIO,
  sensor, NVS and LED output. It covers brightness/gamma, capybara contrast
  across all 64 DNA tones, BOOT (debounce, hold, reset), IMU (hysteresis,
  gestures), sleep, menu, egg, poses without clipping, meals for all six
  species, dream chapters, Conway, and the editor pet (USB protocol, errors,
  NVS, adopting and live swap).
- **`test_preview.cjs`** runs the real simulator renderers with a simulated
  clock: idle with no blackouts, meals, care, 3 min of dreaming, movement and BOOT.
- **`test_petpack.cjs`** checks the editor package for the six templates,
  validation, the simulator conversion and the `.art` export (running the real
  generator in a temporary copy). It also checks the fixtures in
  `test/fixtures/`, which the C++ test loads into the firmware: if you change
  the format, run `node test/test_petpack.cjs --update` and review the C++ test.
- **`test_installer.cjs`** and **`test_web_installer.py`** check the installer
  package, the SHA-256 and the lockout when something does not match.

GitHub Actions runs all of this on every push and pull request. Results and
the physical board test script are in [Testing](TESTING.md).

## Editing the art

To draw without touching code, use the [editor](EDITOR.md); its **Export
.art** button produces the text below so the pet can be added to the repository.

Edit **`art/*.art`** and run `python tools/gen_art.py`. Do not edit
`src/art/ArtData.*` or `preview/art.js` by hand. See the format and LED
guidelines in [art/README.en.md](../../art/README.en.md).

```text
art/             sprites, palettes, effects, font and LED color profile
tools/           art generator, local server, installer, release and tests
preview/         website: guide, simulator, editor and installer (i18n.js, petpack.js)
src/Game.*       scenes, input and animations
src/PetSim.*     state and rules
src/Dream.*      Conway automaton for the dreams
src/CustomPet.*  editor pet: PGP1 package and USB protocol
src/Input.*      BOOT button with debounce
src/Imu.*        accelerometer reading and gestures
src/Canvas.*     8×8 composition
src/Display.*    FastLED output, orientation and limits
src/Storage.*    NVS persistence
test/            C++, JS and Python tests, hardware mocks and fixtures
```

Code comments, logs and some internal docs are in Portuguese; identifiers
are mostly English. Both languages are welcome in issues and pull requests.

## Release package

`python tools/package_release.py v0.1.0` builds `dist/PixelGotchi-v0.1.0.zip`
with a single merged image, binaries with offsets, the case STLs and
`SHA256SUMS.txt`. To publish: **Actions → Release → Run workflow** with the
version (or push a `v*` tag); the workflow builds, packages and creates the
release with the notes from `docs/releases/<version>.md` (see
[Releases](https://github.com/pantojinho/PixelGotchi/releases)). Open items and ideas:
[roadmap](ROADMAP.md).
