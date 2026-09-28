# Prompts for AI assistants

[README](../../README.en.md) · [Build yours](BUILD-GUIDE.md) · [How to play](GAME.md) · [Editor](EDITOR.md) · [Hardware](HARDWARE.md) · [Development](DEVELOPMENT.md) · [AI prompts](AI-PROMPTS.md) · [Português](../PROMPTS-IA.md)

Copy the whole block and paste it into the AI. There are two kinds of assistant:

- **Regular chat** (in the browser or on a phone): talks and writes text, but
  does not touch your computer or the board. Good for prompts 2 and 4.
- **Local agent** (a coding agent running on your computer, with a terminal):
  can download the project, run commands and flash the board over USB.
  Required for prompts 1, 3 and 5.

To just flash the game you do not need an AI: the
[browser installer](https://pantojinho.github.io/PixelGotchi/install.html?lang=en)
does it in a few clicks. To create a pet, the
[website editor](EDITOR.md) draws, tests and sends it without AI or compiling.

## 1. Install on the board with a local agent

```text
Install PixelGotchi from https://github.com/pantojinho/PixelGotchi
on my Waveshare ESP32-S3-Matrix connected over USB. I authorize downloading
the tools, building and flashing the firmware to this board.

1. Read README.en.md, docs/en/INSTALL.md and platformio.ini at the current
   revision. Confirm the ESP32-S3-Matrix model, with an 8x8 RGB matrix, BOOT
   and QMI8658. Keep existing local files; clone into its own folder if needed.
2. Identify my operating system, Git, Python and PlatformIO. Use a Python
   environment outside the repository and PlatformIO 6.2.0, as in the guide.
   Install only what is needed.
3. List the USB/serial ports and identify my board. If it is ambiguous, ask me
   to unplug/replug the board to confirm. Do not pick a port just because it
   is first in the list.
4. Build the esp32-s3-matrix environment. Art generation is automatic. Keep
   the 4 MB flash, FastLED 3.6.0, USB CDC and the project's brightness and
   current limits. Do not change the art or the game rules.
5. Upload using the identified port explicitly. If download mode is needed,
   guide me through holding BOOT + tapping RESET + releasing BOOT, list the
   port again and continue. After flashing, RESET with BOOT released starts
   the game. Do not run erase_flash.
6. Open the monitor at 115200 and check the startup and QMI8658 logs (they are
   in Portuguese). Report the real result, the installed revision and the port
   used. If USB is not reachable, explain the blocker and give me the exact
   commands to finish; do not claim success without an upload.
7. Explain pet selection, incubation by movement, click to feed, hold and
   release for the menu, and the play/sleep gestures. Separate what you
   verified in software from what I must check on the LEDs.
```

## 2. Draw a new pet (regular chat)

Replace what is between `< >`. The AI returns text in the project's `.art`
format; then use prompt 3 or follow [Editing the art](DEVELOPMENT.md#editing-the-art)
to test and flash it. (Prefer the [editor](EDITOR.md) if you want to draw it
yourself and send it without compiling.)

```text
I want to create a new pet for PixelGotchi, a virtual pet on an 8x8 RGB LED
matrix (https://github.com/pantojinho/PixelGotchi).
The pet: <e.g. a front-facing penguin, black and white, orange beak>.
Short lowercase ID: <e.g. pingu>. Name: <e.g. Penguin>.

Produce a text block in the project's art/*.art format, strictly following:

Format
- "palette <name>" followed by lines "  <LETTER> #RRGGBB" and "end".
- "sprite <name> <palette>" followed by the drawing rows and "end".
  "." is an unlit LED. Every row of a sprite has the same width.
  Maximum width and height: 8. Only use letters defined in the palette.
- "anim <name> <ms_per_frame> <sprite> <sprite> ..." on one line.
- "sprite@palette" reuses a drawing with another palette.
- "palette <id>_wild : <id>" redefines the same letters with darker, earthy
  colors (the wild version, when the pet is neglected).

What to deliver (replace <id> with the ID)
- palette <id> with 2 to 4 very saturated colors that differ in lightness.
- palette <id>_wild : <id>.
- palette egg_<id> with the letters E (shell), S (spots), H (highlight,
  #FFFFFF) and C (crack), in that order, in the pet's colors.
- Sprites and the required animations:
  <id>_idle, <id>_blink, <id>_walk, <id>_eat, <id>_sleep, <id>_happy,
  <id>_sad, <id>_hungry, <id>_tired and
  "anim <id>_egg 700 egg0@egg_<id>" (the egg drawing already exists).
- Last, the line:
  pet <id> "<Name>" food=<food_melon|food_fish|food_fly|food_worm|food_carrot|food_shrimp>
  wild=<id>_wild   (add side=1 if the pet is drawn in profile facing right)

Drawing rules for 64 LEDs
- Unlit background; do not use black as an outline. The silhouette must be
  recognizable by shape alone. Eyes are usually an unlit pixel.
- Standing poses at most 7 wide and 7 tall; all align to the bottom center.
  Sleep 4 rows tall (leaves room for the Zzz).
- Sad and tired never wider than idle.
- Expressions: happy = closed eyes; sad = blue tear (#4FA8FF) under the eye;
  hungry = open mouth; tired = head down, half-closed eyes and a yawn;
  eat = the mouth changes between two frames (the game uses that difference
  to find the mouth and put the food right in front of it).
- Keep ears, snout and face in every pose.

Return only the .art block, ready to paste at the end of art/pets.art, with
the pet line last. Then explain in 3 lines how the drawing was designed.
```

## 3. Create, test and flash a pet (local agent)

```text
In the PixelGotchi repository (https://github.com/pantojinho/PixelGotchi),
add a new pet and get it ready for me to test and flash.
<Paste the pet description here, or the .art block you already have.>

1. Read art/README.en.md, docs/en/DEVELOPMENT.md and the end of art/pets.art
   (eggs, wild palettes and "pet" lines).
2. Add the palette, the wild palette "<id>_wild : <id>", the egg_<id> palette,
   sprites and the idle, blink, walk, eat, sleep, happy, sad, hungry, tired
   and egg animations. Add the "pet" line AFTER the existing ones: the saved
   state uses the list position, so do not reorder species.
3. Run python tools/gen_art.py until it passes without errors.
4. Run python tools/test_controls.py and node test/test_preview.cjs.
   If a composition test fails, fix the drawing (width, height, difference
   between idle and eat), not the test.
5. Run python tools/preview.py and tell me what to check in the simulator
   (selection, meal, sleep, sad, hungry) with the new pet.
6. Only flash the board if I ask, following docs/en/INSTALL.md with a port I
   confirmed. Do not change brightness, current or game rules.
7. Show the final diff and list what was verified in software and what I
   still need to look at on the LEDs.
```

## 4. Help with buying, printing and assembly (regular chat)

```text
I am building a PixelGotchi (https://github.com/pantojinho/PixelGotchi):
Waveshare ESP32-S3-Matrix, a 3D-printed case and, optionally, a battery.
Use docs/en/BUILD-GUIDE.md and hardware/case/README.en.md from that
repository as reference. My situation: <e.g. I have an Ender-3 and never
printed small parts / I bought a board and want to check it is the right one
(describe it or send photos) / I want to build the battery version>.

Guide me step by step. Do not make up measurements: when something depends on
my board or printer, tell me how to measure it. For the battery version,
highlight the electrical risks (replacing the TP4056 resistor, the diode,
never connecting the battery to 3V3) before any soldering.
```

## 5. Change the code with a local agent

```text
I want to modify PixelGotchi (https://github.com/pantojinho/PixelGotchi):
<describe the change>.

Before editing, read README.en.md, CONTRIBUTING.md, docs/en/DEVELOPMENT.md,
docs/en/GAME.md, src/Config.h and the tests in test/. Project rules:
- Art only in art/*.art, generated by tools/gen_art.py; never edit
  src/art/ArtData.* or preview/art.js by hand.
- Firmware (src/) and simulator (preview/) must stay equivalent: change both
  when the change is visible.
- Website texts come in Portuguese and English (preview/i18n.js).
- Do not raise brightness (art/led-profile.json), current (MAX_MILLIAMPS) nor
  change the FastLED version.
- Before finishing, run the same commands as CI (docs/en/DEVELOPMENT.md,
  "Tests without a board") and show the output. Say clearly what could not be
  verified without the physical board.
```
