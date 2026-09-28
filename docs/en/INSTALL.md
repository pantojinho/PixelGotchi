# Install the firmware

[README](../../README.en.md) · [Build yours](BUILD-GUIDE.md) · [How to play](GAME.md) · [Editor](EDITOR.md) · [Hardware](HARDWARE.md) · [Development](DEVELOPMENT.md) · [AI prompts](AI-PROMPTS.md) · [Português](../INSTALAR.md)

There are two ways to flash PixelGotchi onto the board. **For most people,
the browser installer is enough**: nothing to install on the computer. The
manual install is for those changing the code or art, or without Chrome/Edge.

## From the browser (recommended)

You need: the board, a **USB-C cable that carries data** (some cables only
charge) and **Chrome or Edge on a computer** (Windows, macOS, Linux or
ChromeOS). Phones and Firefox/Safari cannot reach the USB port (Web Serial).

1. Open the [PixelGotchi installer](https://pantojinho.github.io/PixelGotchi/install.html?lang=en).
2. Plug the board into the computer.
3. Tick that you understood the notice: installing **erases the board**,
   including a pet already saved on it.
4. Click the button, pick the board port in the browser dialog and then
   **Install**. Follow the progress until it finishes.
5. Tap **RESET** on the board (with BOOT released). The pet selection appears.

Before enabling the button, the page checks the package version, size and
SHA-256, and flashing uses exactly those bytes. If the file is missing or
inconsistent, installing stays blocked and the page offers a retry. The
website firmware is always the latest `master`, built automatically by
GitHub Actions.

**Port not showing up?** Close other programs using the board (serial
monitor, Arduino IDE), swap the cable and try flashing mode: hold **BOOT**,
tap **RESET**, release **BOOT** and pick the port again.

The installer flashes PixelGotchi as it is in the repository and **erases the
whole board**, including the saved pet and the pet sent from the editor. To
send your own pet after installing, use the [editor](EDITOR.md): it sends only
the art, without reflashing the firmware.

## Manual install over USB

You need the board (see [Hardware](HARDWARE.md)), a **USB-C data cable**,
internet access to download the tools, and a Windows, macOS or Linux
computer. Flashing replaces the demo program that shipped on the board.

### 1. Get the project and the tools

Install [Git](https://git-scm.com/downloads) and [Python 3.12](https://www.python.org/downloads/).
On Windows, enable "Add Python to PATH" during installation. Open a terminal
in the folder where you want to keep the project and run:

```sh
git clone https://github.com/pantojinho/PixelGotchi.git
cd PixelGotchi
```

Without Git: on GitHub, click **Code → Download ZIP**, extract everything and
open the terminal in the folder that contains `platformio.ini`.

Install PlatformIO in a Python environment **next to** the repository, so the
dependencies do not end up inside the project files.

**Windows — PowerShell:**

```powershell
py -3.12 -m venv ..\pixelgotchi-env
..\pixelgotchi-env\Scripts\python.exe -m pip install platformio==6.2.0
```

**macOS / Linux:**

```sh
python3 -m venv ../pixelgotchi-env
../pixelgotchi-env/bin/python -m pip install platformio==6.2.0
```

In the next examples, use your environment's Python path. No need to activate
it. The first run can take a few minutes.
Reference: [PlatformIO Core installation](https://docs.platformio.org/en/stable/core/installation/methods/installer-script.html).

### 2. Connect and find the port

Plug the board's USB-C into the computer. Close serial monitors using the
board. List the devices:

```powershell
# Windows
..\pixelgotchi-env\Scripts\python.exe -m platformio device list
```

```sh
# macOS / Linux
../pixelgotchi-env/bin/python -m platformio device list
```

Note the port that appears when you plug in and disappears when you unplug the
board. Examples: `COM5` on Windows, `/dev/cu.usbmodem...` on macOS or
`/dev/ttyACM0` on Linux. **These are examples: use your port.** With several
boards connected, identify this one before flashing.
References: [listing devices](https://docs.platformio.org/en/stable/core/userguide/device/cmd_list.html)
and [upload port](https://docs.platformio.org/en/stable/projectconf/sections/env/options/upload/upload_port.html).

### 3. Build and flash

**Windows — replace `COM5` with your port:**

```powershell
..\pixelgotchi-env\Scripts\python.exe -m platformio run -e esp32-s3-matrix
..\pixelgotchi-env\Scripts\python.exe -m platformio run -e esp32-s3-matrix -t upload --upload-port COM5
```

**macOS / Linux — replace `/dev/ttyACM0` with your port:**

```sh
../pixelgotchi-env/bin/python -m platformio run -e esp32-s3-matrix
../pixelgotchi-env/bin/python -m platformio run -e esp32-s3-matrix -t upload --upload-port /dev/ttyACM0
```

Wait for **`[SUCCESS]`** on both build and upload. PlatformIO downloads the
compiler and libraries and generates the art automatically. Use the project's
`platformio.ini`: the environment already sets **4 MB flash**, FastLED 3.6.0
and USB CDC. The generic `esp32-s3-devkitc-1` name in that file is
intentional; the Matrix-specific settings live in the project.

If the upload does not connect, put the board in download mode:

1. Hold **BOOT**.
2. Press and release **RESET** while keeping BOOT pressed.
3. Release **BOOT**.
4. List the ports again — the name may change — and repeat the upload.
5. When done, press **RESET** with BOOT released to start the game.

That is the ESP32-S3 flashing mode described by
[Espressif](https://docs.espressif.com/projects/esptool/en/latest/esp32s3/advanced-topics/boot-mode-selection.html).
While playing, use BOOT after the firmware has started.

### 4. Check that it started

Open the monitor and, if needed, tap RESET to see the boot:

```powershell
# Windows — your port may change after the upload
..\pixelgotchi-env\Scripts\python.exe -m platformio device monitor --port COM5 --baud 115200
```

```sh
# macOS / Linux
../pixelgotchi-env/bin/python -m platformio device monitor --port /dev/ttyACM0 --baud 115200
```

Expected output: `[PixelGochi] iniciando...`, `[Imu] QMI8658 ok` and
`[Game] fase=...` (the logs are in Portuguese). Use `Ctrl+C` to close the
monitor before another upload. On the matrix, pick the pet with clicks; hold
BOOT 0.6 s and release to start the egg. Gentle movement adds up to the five
minutes of incubation.

| Problem | What to check |
|---|---|
| No port shows up | Use a data cable and another USB port; try BOOT + RESET and list again |
| Port busy | Close the serial monitor, other IDEs and apps using that port |
| Upload does not connect | Check the current port and use the download mode above |
| Linux says permission denied | Fix the port permission/group for your distribution; reconnect afterwards |
| Empty monitor | Use 115200, check the port after reset and keep BOOT released |
| IMU did not answer | Confirm the exact board model; BOOT works but gestures need the sensor |
| Image or tilt inverted | See `DISPLAY_ROTATION`, `DISPLAY_MIRROR_X` and the calibration in [Hardware](HARDWARE.md) |

This board uses the ESP32-S3 native USB; do not assume you need a CH340 driver
from another board. For detection problems, check the
[Matrix documentation](https://docs.waveshare.com/ESP32-S3-Matrix/Arduino).

**Graphical alternative:** open the project folder in
[VS Code](https://code.visualstudio.com/) with the
[PlatformIO IDE](https://platformio.org/install/ide?install=vscode) extension.
Under **Project Tasks → esp32-s3-matrix**, run **Build** and **Upload**; use
the monitor at 115200. With more than one port available, prefer the commands
above to choose the board explicitly.

### Updating an existing install

With a Git copy, check your changes before updating:

```sh
git status
git pull --ff-only
```

If you have local changes, save them before pulling. Then repeat the build and
upload with the current port. With a ZIP copy, download and extract the new
version separately. A normal upload does not erase the whole flash. Keeping
the pet depends on the saved format and partitions being compatible between
versions; it is not a guaranteed migration to any future firmware.

**Verified in software:** build, C++ tests and simulator; see the evidence in
[TESTES.md](../TESTES.md) (Portuguese). USB flashing, gestures and the real
look still need the tests on a physical board.
