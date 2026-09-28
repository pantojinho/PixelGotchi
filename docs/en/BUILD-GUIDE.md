# Build your PixelGotchi at home

[README](../../README.en.md) · [Build yours](BUILD-GUIDE.md) · [How to play](GAME.md) · [Editor](EDITOR.md) · [Hardware](HARDWARE.md) · [Development](DEVELOPMENT.md) · [AI prompts](AI-PROMPTS.md) · [Português](../GUIA-MONTAGEM.md)

From zero to a pet hatching in your hand. The basic version **needs no
soldering, wires or programming**: a ready-made board, a cable and a browser.

| Step | You need | Time |
|---|---|---|
| 0. Try it before buying | A browser | 5 min |
| 1. Buy the board | — | shipping time |
| 2. Flash the game | Computer with Chrome/Edge and a USB-C data cable | 5 min |
| 3. Hatch the egg | Move the board gently | 5 min |
| 4. Printed case (optional) | 3D printer or printing service | ~1 h of printing |
| 5. Battery (optional, advanced) | Soldering and a few parts | 1–2 h |
| 6. Your own pet (optional) | The editor in the browser | 15 min or more |

## 0. Try it before buying

Open the [simulator](https://pantojinho.github.io/PixelGotchi/?lang=en). It
uses the same drawings as the firmware: pick the pet, click **BOOT** to feed,
hold and release to open the menu, use the tilt, shake and flip buttons.
Waiting times are shortened so you can see dreams and happenings right away.

## 1. What to buy

| Item | Required? | Where to find it |
|---|---|---|
| **Waveshare ESP32-S3-Matrix** (25 × 25 mm board with an 8×8 RGB matrix, BOOT button and QMI8658 sensor) | Yes | [Waveshare store](https://www.waveshare.com/esp32-s3-matrix.htm) · [AliExpress](https://www.aliexpress.com/w/wholesale-waveshare-esp32%2Ds3%2Dmatrix.html) · [Mercado Livre (Brazil)](https://lista.mercadolivre.com.br/esp32-s3-matrix) |
| **USB-C data cable** | Yes | Many cables only charge. If the computer does not "see" the board, swap the cable first |
| Phone USB charger or power bank | To use it away from the computer | Whatever you have |
| 3D-printed case | No | Ready-made files in the project (step 4) |
| Battery version parts | No | [List in the case documentation](../../hardware/case/README.en.md#shopping-list) |

**Check the model before paying.** There are several "ESP32 + 8×8 matrix"
boards with different pinouts. The listing must say **Waveshare
ESP32-S3-Matrix** and the photos must match the front and back below. The
AliExpress and Mercado Livre links are searches: pick the seller.

![Front and back of the Waveshare ESP32-S3-Matrix, with matrix, USB-C, BOOT and RESET](../images/hardware-components.png)

More details (pins, dimensions, brightness): [Hardware](HARDWARE.md) and the
[official Waveshare documentation](https://docs.waveshare.com/ESP32-S3-Matrix).

## 2. Flash the game from the browser

1. On a computer, open the [installer](https://pantojinho.github.io/PixelGotchi/install.html?lang=en)
   in **Chrome or Edge** (Firefox, Safari and phones cannot access USB).
2. Plug in the board with the data cable.
3. Tick the notice (flashing erases what is on the board), click the button,
   pick the board port and then **Install**.
4. When it finishes, tap **RESET** on the board.

No port showing up? Swap the cable, close programs that use serial ports and
try flashing mode: hold **BOOT**, tap **RESET**, release **BOOT** and pick the
port again. More options in [Install](INSTALL.md).

## 3. First time: pick and hatch

1. The board shows a pet. **Click** BOOT (or tilt) to switch between
   capybara, cat, frog, chick, bunny and axolotl.
2. **Hold BOOT for 0.6 s and release** to choose. An egg appears.
3. **Move the board gently**: the egg hatches after 5 minutes of accumulated
   movement. Stopping only pauses it; click BOOT to see the progress bar.
4. The pet hatches and says its own name, taken from its DNA.

From then on: **click = food**, **hold and release = menu**, **shake = play**,
**screen face down = sleep**. Everything is in [How to play](GAME.md). The pet
stays saved even if the board is turned off; its clock only runs while powered.

## 4. 3D-printed case (optional)

A **28.7 × 28.7 × 8.6 mm** case with a USB-C opening, pins that press BOOT
and RESET from the back, and a keychain loop. Prints **without supports**.

![Exploded case](../../hardware/case/images/exploded.png)

**No printer?** 3D printing services, makerspaces and technical schools print
small parts like these; send the `.stl` files below and ask for PLA.

| File | How many | Note |
|---|---|---|
| [`front_open.stl`](../../hardware/case/stl/front_open.stl) **or** [`front_diffuser.stl`](../../hardware/case/stl/front_diffuser.stl) | 1 | Open window (any color) or diffuser with an 8×8 grid (white/natural PLA, "pixel" effect) |
| [`back.stl`](../../hardware/case/stl/back.stl) | 1 | Back cover |
| [`pin.stl`](../../hardware/case/stl/pin.stl) | 2 (+1 spare) | Button pins; print them with the case and use a brim |

Suggested settings in any slicer ([PrusaSlicer](https://www.prusa3d.com/page/prusaslicer_424/),
[OrcaSlicer](https://www.orcaslicer.com/), Cura, Creality Print): PLA,
0.16–0.2 mm layers (pins 0.12 mm), 4 walls, 20% infill (pins 100%), no
supports. Values tested on an Ender-3 V3 KE and fitting tips:
[case documentation](../../hardware/case/README.en.md).

### Assembly

1. Board **LEDs facing down** inside the front, USB-C aligned with the opening.
2. The two pins on the buttons, thin tip down: **R** over RESET, **B** over BOOT.
3. Snap the back cover on until it clicks. To open: a fingernail in the notch
   at the bottom.
4. Power it on and check: the board must **not** reset or enter flashing mode
   by itself. If it does, a pin is too long: sand the tip or raise `sw_h` in
   `hardware/case/placa.scad` (it shortens the pin) and regenerate.

The measurements came from Waveshare's drawing and from photos; the first
print is a fit test. To adjust clearances and generate new STLs with
OpenSCAD, see the [case documentation](../../hardware/case/README.en.md#generating-the-stls).

## 5. Battery version (optional, advanced)

A 28.7 × 47.7 × 18.8 mm case with a 160 mAh LiPo, a TP4056 charger and a power
switch, charging through the same USB-C. **It requires soldering to the board
pads and replacing a resistor on the charger**; wrong wiring can damage the
board or the battery. Estimated runtime: ~1.5 h. Follow the
[shopping list, wiring and assembly](../../hardware/case/README.en.md#battery-version)
and test everything outside the case before closing it.

## 6. Create your own pet (optional)

In the [website editor](https://pantojinho.github.io/PixelGotchi/editor.html?lang=en)
you start from one of the six pets, redraw the poses, test them in the
simulator and send the pet to the board over the same USB cable, with no
compiling. It becomes the 7th species or replaces the current pet with an
egg. Step by step in [Editor](EDITOR.md).

## Something went wrong?

| Symptom | What to do |
|---|---|
| The browser shows no port | Data cable, another USB port, close serial monitors, BOOT + RESET |
| Flashed but the matrix stays dark | Tap RESET with BOOT released; check it really is the ESP32-S3-Matrix |
| Image upside down or tilt inverted | Another board revision: adjust in [Hardware](HARDWARE.md) |
| Gestures do not work, BOOT does | The QMI8658 sensor did not answer; check the exact model |
| Too bright or colors blend | Brightness is already low (5/255); see the LED profile in [Hardware](HARDWARE.md) |
| The board gets hot | Unplug it. The firmware caps brightness and current; do not raise those limits |

Still stuck? Use the [assisted install prompt](AI-PROMPTS.md#1-install-on-the-board-with-a-local-agent)
with an AI agent on your computer, or open an
[issue](https://github.com/pantojinho/PixelGotchi/issues) with photos of the
board and the error message.
