# Hardware, brightness and sensor

[README](../../README.en.md) · [Build yours](BUILD-GUIDE.md) · [How to play](GAME.md) · [Editor](EDITOR.md) · [Hardware](HARDWARE.md) · [Development](DEVELOPMENT.md) · [AI prompts](AI-PROMPTS.md) · [Português](../HARDWARE.md)

[Board documentation](https://docs.waveshare.com/ESP32-S3-Matrix).

![Front and back of the Waveshare ESP32-S3-Matrix, with matrix, USB-C, BOOT and RESET](../images/hardware-components.png)

The target is the **Waveshare ESP32-S3-Matrix**, **25 × 25 mm**, with 64 RGB
LEDs, USB-C, BOOT, RESET and a QMI8658 on board. To play and install over
USB, **you do not need to solder wires or add another button**. If you bought
it on AliExpress, check the name and both sides of the board above: having an
ESP32 and an 8×8 matrix does not guarantee the same pinout.

<details>
<summary>Dimensions and external pinout</summary>

![ESP32-S3-Matrix board dimensions in millimeters](../images/hardware-dimensions.png)

![Pinout of the ESP32-S3-Matrix external headers](../images/hardware-pinout.png)

The GPIOs in the table below are **internal** connections used by the
firmware; they are not wires you need to connect to the headers in the image.
The three hardware images were provided by the project author. See also the
[official Waveshare reference](https://docs.waveshare.com/ESP32-S3-Matrix).

</details>

| Component | Pins |
|---|---|
| 64 WS2812B LEDs, **RGB** color order (not the usual GRB) | GPIO14 |
| QMI8658, I²C | SDA GPIO11, SCL GPIO12 |
| BOOT | GPIO0 |

The firmware caps brightness at **5/255** (it used to be 30, 18 and 13: with
more brightness neighboring colors "bleed" and become hard to tell apart),
the matrix current at **400 mA**, and uses FastLED **3.6.0** with the RMT
driver and no temporal dithering. Waveshare warns that excessive brightness
heats up and can damage the board.

Colors go through a **gentle 1.6 gamma curve** before the global brightness.
It lowers the mid-tones and helps separate light and dark shades. The
capybara uses a copper body, a light amber snout and a dark brown nose, with
more distance between the colors. Black stays off and primary colors stay pure.

After the curve, **brightness is adjusted per color**: each pixel's R+G+B sum
is capped (`glare_cap`), so white and light tones dazzle less than pure
colors; blue is attenuated (`blue_gain`); and dark colors never go below
`min_peak` steps, so they do not disappear. While the pet sleeps, every lit
LED sits at the lowest visible step.

The profile lives in [`art/led-profile.json`](../../art/led-profile.json) and
generates the same table for the firmware and the simulator. To dim it
further, try `"brightness": 4`, regenerate the art and flash the board again;
do not raise brightness to compensate for contrast. This change does not
erase the saved pet. The optical result still needs checking on your unit;
the curve is not a measured hardware calibration.

Holding BOOT during reset/power-up enters the ESP32 flashing mode. To play,
press it **after** the firmware has started.

Image orientation: `DISPLAY_ROTATION` and `DISPLAY_MIRROR_X` in `src/Config.h`.

**Accelerometer:** the QMI8658 sits on the back of the board, so "screen up"
is the **negative** z axis. Axes and signs are in `src/ImuCalib.h`, measured
on a real board. If tilting or "turn over to sleep" behave wrong (another
board, another batch), recalibrate in 4 positions and flash again:

```bash
python tools/calibrate_imu.py
```

Gesture sensitivity and timing are in `src/Config.h`. The IMU tries addresses
0x6B and 0x6A. The first sample is not counted as movement and the flip
gesture uses hysteresis to avoid flickering between sleep and awake.

## 3D case

A **28.7 × 28.7 × 8.6 mm** case with access to the USB-C, the BOOT and RESET
buttons on the back (through pins that never stay pressed on their own) and a
keychain loop; open-window or diffuser-with-8×8-grid versions. No supports.
There is also a **battery version** (28.7 × 47.7 × 18.8 mm, 160 mAh LiPo,
TP4056 charger and power switch, charging through the same USB-C). See the
[case documentation](../../hardware/case/README.en.md) (STLs, settings for the
Ender-3 V3 KE, wiring and assembly).

![Exploded case](../../hardware/case/images/exploded.png)
