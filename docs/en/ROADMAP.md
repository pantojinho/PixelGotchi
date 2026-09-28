# Project roadmap

[README](../../README.en.md) · [Testing](TESTING.md) · [Contributing](../../CONTRIBUTING.md#english) · [Português](../ROADMAP.md)

## Live today (v0.1.0)

Released on 2026-09-28: [v0.1.0 release](https://github.com/pantojinho/PixelGotchi/releases/tag/v0.1.0)
and the [website](https://pantojinho.github.io/PixelGotchi/guia.html?lang=en), updated on every push to `master`.

- **Firmware** for the Waveshare ESP32-S3-Matrix: six species, an egg that
  hatches with movement, a DNA name, hunger/joy/energy, dirt, sickness,
  neglect and wild life, an 8-icon menu, status, naps, gestures (shake, tilt,
  flip) and everything saved in NVS.
- **Continuous animations**: meals with food in front of the mouth, small
  intentions while awake and Conway dreams with chapters.
- **Editor pet**: received over USB, validated and stored on the board as the
  7th species, without recompiling.
- **Bilingual website (PT/EN)**: "Start here" guide, simulator, pet editor
  and USB installer with the latest firmware.
- **3D case** (open, diffuser and battery versions) and a release package
  with a single image, binaries and STLs.
- **CI** with firmware tests (simulated hardware), website, editor and
  installer tests, and the ESP32-S3 build.

## Still to check on a physical board

Everything above was verified in software. The [board script](TESTING.md#board-script)
still needs to be run: browser flashing, sending from the editor (the board
may reset when the port opens), gestures, real colors, saving and the case fit.

## Known open items

| Item | How to close it |
|---|---|
| Case fit | Print and measure; adjust `btn_dx`, `btn_y`, `sw_h`, `pcb_t`, `plate_fit`, `usb_open_w/h` and, for the diffuser, `led_pitch` in the `.scad` files |
| Battery version | Build it with the TP4056 R3 swapped for 10 kΩ, measure the battery and the real runtime (~1.5 h estimated) |
| Battery level in the game | A 100k/100k divider on an ADC GPIO (IO1–IO7) and a low-battery warning |
| Yellows may look greenish on the LEDs | Check the chick with the serial command `cores RRGGBB …` and tune the palette |
| Happy chick (7 px) clips at the edge when hopping in a corner | Limit the position by the current frame width or slim the sprite |
| Update from the browser keeping the pet | The web installer flashes the full image (erases everything); offer program-only flashing |
| Clock while off | With the board off the pet's time stops; internet time would need a Wi-Fi version |

## Ideas for next versions

- Several editor pets on the same board, switchable from the menu.
- 360° orientation: the "floor" turns with the board; dizziness when upside down.
- Visible DNA: spots, stripes and eye color; named personalities.
- Letters in the air: the pet "talks" ("OI", "?", "FOME", "♥").
- More sensor reactions: dizziness on a hard shake, a hop on a tap.
- Mini-game: food falling from the top, tilt to catch it.
- Evolution egg → baby → adult depending on care.
- Phone setup (name, time, day/night cycle).
- Import PNGs from LibreSprite/Aseprite into `art/`.
- An egg-shaped, Tamagotchi-style case.

Want to take one? See [how to contribute](../../CONTRIBUTING.md#english) and
open an issue with your plan before starting the bigger ones.
