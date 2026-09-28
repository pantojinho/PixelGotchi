# Pet editor

[README](../../README.en.md) · [Build yours](BUILD-GUIDE.md) · [How to play](GAME.md) · [Editor](EDITOR.md) · [Hardware](HARDWARE.md) · [Development](DEVELOPMENT.md) · [AI prompts](AI-PROMPTS.md) · [Português](../EDITOR.md)

Create your own pet in the browser and send it to the board over USB, with
nothing to install and no compiling. Open the
[editor on the website](https://pantojinho.github.io/PixelGotchi/editor.html?lang=en)
or, on the local copy, `http://localhost:8765/editor.html`.

## Step by step

1. **Start from** one of the six pets (or blank) and **Use this template**.
   Starting from a template is the fastest way: every pose already exists and
   you only change what you want.
2. **Name** (up to 12 letters, no accents), **favorite food** and whether it
   is drawn **in profile** (facing right; the game mirrors it when walking left).
3. **Draw** on the 8×8 grid: click or drag to paint, right click erases,
   **Fill** fills an area. **Mirror** and the arrows help alignment. Black is
   an unlit LED: the silhouette is made of color, not outlines.
4. **Animations**: pick Idle, Blink, Walk, Eat, Sleep, Happy, Sad, Hungry or
   Tired. Each is a sequence of **steps**; each step shows a **frame**. The
   same frame can appear in several steps and animations (the editor tells you
   where it is used; editing it changes all of those places). **+ New frame**
   creates a copy to change; **+ Repeat this one** repeats the same frame. Set
   each step's time in ms.
5. **Check**: in red, what the board would not accept; in orange, what works
   but may look odd. When everything is fine it shows the package size.
6. **Test in the simulator** opens the game with your pet selected, with the
   same controls as the board. It also shows up in the animation gallery.
7. **Send to the board**: Chrome or Edge on a computer, board on a USB-C data
   cable. **Connect board**, pick the port, **Send pet**.

The project is saved in the browser. Use **Download project** to keep a
`.pixelgotchi.json` file and **Open project** to continue on another computer.
Undo/redo: the buttons or `Ctrl+Z` / `Ctrl+Y`.

## What happens on the board

- Without ticking anything, the pet is **saved as the 7th species**. It shows
  up in the selection when you restart the game (hold BOOT for 8 s).
- Ticking **Replace my current pet with an egg of this one** swaps the current
  pet for an egg of the new one right away. The previous pet is lost.
- If the current pet **already is** the editor one, the new drawing shows up
  right away: tweak and resend as often as you like without losing the pet.
- The board holds **one** editor pet at a time; sending another replaces it.
- The firmware **installer** erases the whole board, including the editor
  pet. Keep the project file to send it again later.

The board needs the current firmware. If the editor says it did not answer,
flash the firmware with the [installer](https://pantojinho.github.io/PixelGotchi/install.html?lang=en)
and connect again. An interrupted or corrupted transfer never replaces the
pet already saved.

## Drawing tips for 64 LEDs

- Leave at least **one free column** (7 wide): at 8 it cannot walk.
- Every pose shares the same columns; the dotted line on the grid shows the
  width the game will use. The pet stands on the bottom row.
- **Eat** must differ from **Idle** at the mouth: the game compares both to
  find the mouth and put the food right in front of it.
- A low **Sleep** pose (up to 4 or 5 rows) leaves room for the Zzz and the
  dream bubble.
- A few colors with very different lightness work better than close shades,
  which blend in the LED glow. Check the LED-look preview.
- See also the [art guidelines](../../art/README.en.md#drawing-guidelines).

## Limits

| Item | Limit |
|---|---|
| Colors | 1 to 15 (plus the unlit LED) |
| Distinct frames | up to 40 |
| Steps per animation | 1 to 12, each 40 to 5000 ms |
| Name | up to 12 ASCII characters |
| Package | up to 3072 bytes |

## For developers

The editor builds a binary **PGP1** package (`preview/petpack.js`) that the
firmware validates and stores in NVS (`src/CustomPet.*`). The format and the
protocol are documented at the top of [`src/CustomPet.h`](../../src/CustomPet.h).
Protocol summary, as text lines over the same USB as the logs, at 115200:

```text
PG?            -> PG HELLO <protocol> <max bytes> <has pet 0/1> <active 0/1> <name|->
PGPUT <bytes>  -> PG READY
PGD <hex>      -> PG ACK <received>        (up to 64 bytes per line)
PGEND          -> PG SAVED <name> | PG ERR <reason>
PGADOPT        -> PG ADOPTED
PGDEL          -> PG DELETED
```

The whole package is validated (size, color indices, references, timings and
CRC-32) before it is written; NVS writes the new value before releasing the
old one. `test/test_petpack.cjs` generates fixtures that the C++ test loads
into the firmware, ensuring the browser and the board read the same bytes.
