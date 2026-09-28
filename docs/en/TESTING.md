# Testing

[README](../../README.en.md) · [Development](DEVELOPMENT.md) · [Roadmap](ROADMAP.md) · [Português](../TESTES.md)

There are two kinds of checks: the **automatic tests**, which run on every
push with no board at all, and the **physical board script**, which only
someone holding a Matrix can run. Passing the automatic tests does not prove
USB flashing, the sensor, the real colors or the case fit.

## Automatic tests

They run on GitHub Actions on every push and pull request
([verify.yml](../../.github/workflows/verify.yml)). To run them on your
computer, see [Tests without a board](DEVELOPMENT.md#tests-without-a-board).

| Suite | What it guarantees |
|---|---|
| Generated art | `tools/gen_art.py` validates `art/*.art` and the committed outputs are up to date |
| `tools/test_controls.py` (real C++, simulated hardware) | BOOT (debounce, hold, reset), IMU (gestures, hysteresis), menu, status, egg, sleep and naps, poses without clipping, meals for all six species without covering the pet, dreams (chapters, bubble, 3 min without blackouts), Conway around the pet, LED contrast and the editor pet (USB protocol, validation, NVS, adopting, live swap) |
| `test/test_preview.cjs` | Simulator: six species without blackouts, meals, care, 3 min of dreaming, movement and BOOT |
| `test/test_petpack.cjs` | Editor package for the six templates, validation, simulator and `.art` export; generates the fixtures the C++ test loads into the firmware |
| `test/test_installer.cjs` and `test_web_installer.py` | Web installer: consent, SHA-256, missing or inconsistent package |
| ESP32-S3 build | The firmware compiles with PlatformIO |

## Status on a physical board (v0.1.0)

Everything above passes. **The board script below has not been run in full
yet.** If you build one, your results help a lot: use the **Test report**
issue template and say which revision you tested.

## Board script

You need: a Waveshare ESP32-S3-Matrix, a USB-C data cable, a computer with
Chrome or Edge. Note the revision (release version or commit).

| ID | What to do | Expected |
|---|---|---|
| HW-01 | Flash with the [web installer](https://pantojinho.github.io/PixelGotchi/install.html?lang=en) and tap RESET | It installs to the end and the pet selection shows up |
| HW-02 | Serial monitor at 115200 and RESET | Startup logs, `QMI8658 ok` and the game phase; no reboot loop |
| HW-03 | Change species (click and tilt) and confirm (hold and release) | One change per input; confirms only once, on release |
| HW-04 | Move the egg gently, stop 15 s, continue | Incubates with movement, pauses when still and resumes |
| HW-05 | Feed the cat and the capybara | Pet always visible; food in front of the mouth, shrinks, heart at the end |
| HW-06 | Use the eight menu items and the status | Everything reachable with BOOT only; readable icons and text |
| HW-07 | Shake twice in a row, then after 5 s | The second one in a row is ignored; the later one plays |
| HW-08 | Screen face down 1.5 s and flip back; then sleep from the menu | Gesture sleep wakes on flipping back; menu sleep continues |
| HW-09 | Leave it still 2 min with good energy; then sleep and wait 3 min | Conway around the pet; asleep, bubble and dream chapters with no blackouts |
| HW-10 | With energy below 60, leave it 3 min untouched | It naps by itself; moving shows it asleep; BOOT wakes it |
| HW-11 | In the [editor](EDITOR.md), connect the board and send a pet; then send with "Replace" | The board answers; the pet shows up as the 7th species; with "Replace", it becomes an egg |
| HW-12 | Pull the cable in the middle of an editor transfer and reconnect | The previous pet is kept |
| HW-13 | Restart, and power off/on after 5 min of play | Pet and editor pet come back saved |
| HW-14 | Flash only the release `firmware.bin` at 0x10000 | Updates keeping the pet |
| HW-15 | Use 30 min on USB | Comfortable colors, no white flash, freezing or unusual heat |
| HW-16 | Case: print, assemble and power on | Firm fit; the pins do not keep BOOT/RESET pressed; USB reachable |
| HW-17 | At the end, hold BOOT 8 s | Red bar from 3 s and back to the selection |

Do HW-17 last: it erases the game progress.

### Report template

```text
Date / tester:
Revision (release or commit):
Board / computer / OS / browser:
Cable / port / USB power:
Cases run and result (pass / fail / not run):
Photos, videos and logs:
Failures: steps, expected, observed:
Orientation or sensitivity tweaks used:
```
