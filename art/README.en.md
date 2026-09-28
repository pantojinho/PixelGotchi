# Art for 64 LEDs

[Português](README.md) · **English**

`pets.art`, `props.art` and `font.art` are the source of the firmware and
simulator art. `python tools/gen_art.py` validates references, sizes,
characters and palettes, then generates C++ and JavaScript. To draw a pet
without editing text, use the [website editor](../docs/en/EDITOR.md); its
**Export .art** button produces this same format.

## Format

```text
palette example
  B #8DAAE2
  P #FF7FA6
end

sprite example_idle example
B...B
BPBPB
B.B.B
BBPBB
end

anim example_idle 450 example_idle
```

`.` is transparent: an unlit LED. Every row of a sprite has the same width;
maximum width and height are 8. Animations list each frame's time in
milliseconds and the sprite names in order. `sprite@palette` reuses a drawing
with another palette. `palette variant : base` inherits the indices for
runtime palette swaps.

A pet needs the animations `idle`, `blink`, `walk`, `eat`, `sleep`, `happy`,
`sad`, `hungry`, `tired` and `egg`, prefixed by its ID. The `pet` declaration
sets the name, food, wild palette and whether it is drawn in profile
(`side=1`). Keep the existing species order: the saved state uses that index.

## Drawing guidelines

- The silhouette has to work before the colors. Black does not work as an
  outline: it disappears into the background. Unlit eyes use that contrast.
- Cat: separate pointy ears, light muzzle, chest, paws and tail with at least
  one pixel of separation from the body.
- Capybara: profile, short rounded ear, nose at the tip of a wide snout,
  rounded back and short legs. No long tail.
- Prefer large color masses. Close tones blend in the LED halo. Check in LED
  mode at 5/255 and also with the design palette.
- Keep ears and snout in every expression. Sadness must not turn the species
  into another silhouette.
- The cat and capybara poses are 7 wide: only one pixel is left to walk, but
  the essential parts stay readable. When mirroring the capybara, the whole
  profile changes direction.
- Every pose is anchored at the bottom center. Sleep uses a height of 4,
  leaving room for the Zzz. Check both ends of the movement.
- Large effects and food alternate in time with the pet. Never clear columns
  of the composition to fake bites: that would erase the pet too.
- The eight menu options keep the bottom row for the navigation dots.

The simulator's LED mode is an approximation, without measuring diffusion,
gamma or the calibration of each unit. Final validation happens on the
physical matrix.

## Brightness and color profile

`led-profile.json` sets the global brightness (**5/255**) and the gamma curve
(**1.6**). The generator produces `src/art/LedProfile.h` and
`ART.ledProfile` in the simulator, including the same 256-entry table. Do not
edit the outputs by hand.

The curve is applied once, at the Display output, after composition/DNA and
before FastLED brightness. Design mode shows the original RGB; LED mode uses
the firmware's table and brightness quantization. The monitor still only
approximates perception, without physically reproducing the board.

Choose lightness and hue differences between body, face and details. On the
capybara, the copper body is darker than the amber snout, and the nose has a
third level. The tests check the separation after gamma, brightness and DNA,
and that the nose stays lit while sleeping. That verifies the digital values;
the final visual reading depends on the LEDs and the ambient light.

For a dark room you can lower `brightness`. Run the generator and rebuild/
reflash to apply it. The profile accepts brightness 1–30 and gamma 1–2.2; the
1.6 default is a starting point to keep detail in 64 LEDs, subject to
evaluation on the board.
