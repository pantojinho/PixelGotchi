# How to play and how the pet lives

[README](../../README.en.md) · [Build yours](BUILD-GUIDE.md) · [How to play](GAME.md) · [Editor](EDITOR.md) · [Hardware](HARDWARE.md) · [Development](DEVELOPMENT.md) · [AI prompts](AI-PROMPTS.md) · [Português](../JOGO.md)

Everything here applies to both the board and the website simulator unless
stated otherwise. The simulator shortens waiting times for the demo.

## The pets

<details>
<summary>See the capybara poses</summary>

![Capybara frames with a wide, flat-tipped snout: idle, blink, walk, chewing, sleep and expressions](../images/system-capybara-frames.jpg)

</details>

<details>
<summary>See an example of the animation gallery</summary>

![Axolotl states in the gallery: eating, sleeping, happy, sad and hungry](../images/system-gallery.jpg)

The simulator gallery shows every species' poses before you flash the board.

</details>

Capybara, cat, frog, chick, bunny and axolotl. Each species has idle,
walking, blinking, eating, sleeping, happy, sad (the tear rolls down),
hungry and tired (nodding off and yawning) animations, plus an egg palette
and a wild variant.

- **Cat:** sitting, triangular ears with pink insides, unlit eyes, cream
  muzzle and chest, paws and a moving tail.
- **Capybara:** in profile, rounded body, small ear, long wide snout,
  separate nose and short legs. When happy, it balances a tangerine on its head.
- During care, food and hearts use **free pixels around the pet**, keeping
  the silhouette and face intact even with only 64 pixels. Sleeping uses a
  lower pose and dims the LEDs.
- The capybara rests and sniffs more; the cat watches and chases more. DNA
  still varies each individual's personality and color tone.
- Each pet has its **own name** derived from its DNA (e.g. KALU, MOBITE).
  It introduces itself when it hatches, opens the status screen and goes on
  the gravestone.
- **Energy and sleep:** awake it spends 1 energy every 12 minutes (full to
  empty in ~20 h) and each play session costs 5. With energy below 60 and
  nobody around for 3 minutes, it takes a **nap** on its own; tired (below
  25), it sleeps after 30 s without clicks or gestures. Asleep it recovers 1
  per minute, dreams and wakes up by itself when its energy is full.

The words the board scrolls (status pages, gravestone) are in Portuguese:
COMIDA = food, ALEGRIA = joy, ENERGIA = energy, SAUDE = health, DOENTE = sick,
SUJO = dirty, DIAS = days, SELVAGEM = wild.

## Happenings, meals and dreams

**Meals.** The pet looks toward its snout side, the food (1 or 2 pixels in
the food's colors) appears in the first free pixel in front of its mouth, it
steps closer when there is room and chews while the food shrinks; at the end
it is satisfied and a little heart appears. The silhouette is never replaced.
The capybara chews in place (its long snout already reaches the edge) and
moves its jaw. Every meal still gives the same fixed fullness gain.

**Awake**, pauses alternate with small intentions: sniffing a spot on the
ground, looking around, watching something go by overhead, following a
butterfly, and yawning/settling in before a nap. DNA picks how often they
happen and how long the pauses are. The capybara sniffs and watches calmly;
the cat watches more, attentively. BOOT or movement interrupt a happening
immediately, and a requested care always takes priority.

**Conway around the pet.** After 2 minutes without BOOT or movement, with
enough energy and no urgent need, the pet stops and watches for 7 s every
45 s: a glider crossing (even visits) or a pulsing blinker with a little bird
(odd visits). Cells only show in free pixels with a 1-pixel gap from the pet;
the automaton keeps running in full underneath the mask.

**Sleep.** From the menu, the gesture or the automatic low-energy nap, the
pet stays visible for 8 s; then it closes its eyes, little bubbles rise from
its head and a bubble grows until it becomes the whole world, replacing the
pet pixel by pixel. The dream changes chapter every 20–40 s (gliders
crossing, pulses, and groups being born and fading), also by pixel
replacement. An empty or frozen pattern moves on to the next chapter right
away; a leftover oscillator pulses for at most 10 s. Moving or shaking shows
the pet still asleep for another 8 s without waking it, then the dream
returns through the bubble; BOOT wakes it, and it also wakes up when its
energy is full.

One generation every half second, with wrapped edges. DNA, state and a
chapter counter pick pattern, orientation and position reproducibly; cool
tones mean a calm dream, warm tones a restless one. This only affects the
display: it does not change hunger, energy or the feeding rules. The rules
and the glider follow the
[Game of Life on LED matrices](https://www.makerguides.com/game-of-life-dot-matrix-max7219/)
reference. The simulator shortens only the idle wait (12 s instead of 2 min).

The drawings are original, made directly on the pixel grid. Shape
references: [capybara profile (WWF)](https://www.wwf.or.jp/staffblog/news/5510.html)
and [sitting cat silhouette](https://freesvg.org/black-cat-vector-image).
Reference images are not distributed with the project.

## Controls

**One rule for menus: click changes, hold and release confirms.**
After **0.6 s**, a green dot in the top-right corner shows you can release.
The long action only happens when you release the button.

| Situation | Input | Result |
|---|---|---|
| Initial selection | Click BOOT or tilt to one side | Next species; tilting left goes back. A pet sent from the [editor](EDITOR.md) shows up as the 7th species |
| Initial selection | Hold BOOT 0.6 s and release | Picks the species and starts the egg |
| Egg | Move the board gently | Builds up incubation: 5 minutes; stopping for 15 s pauses without resetting |
| Egg | Click BOOT | Shows progress at the top for 2 s |
| Pet awake | Click BOOT | Feeds, with food → chewing → heart animation |
| Pet asleep | Click BOOT | Wakes it up; does not feed |
| Life | Hold BOOT 0.6 s and release | Opens the menu on the most urgent care |
| Menu | Click or tilt | Changes the icon; return to center before the next tilt |
| Menu | Hold BOOT 0.6 s and release | Runs the selected care |
| Life | Shake | Plays: joy +20, energy −5, fullness −3; 5 s pause between gestures |
| Life | Tilt sideways | The pet goes to the lower side |
| Life | Turn the matrix face down for 1.5 s | Sleeps |
| Sleep started by the gesture | Turn it back up | Wakes up; sleep chosen in the menu continues until BOOT/menu or full energy |
| Status | Wait or click | Name → food → joy → energy → health → age → back to the pet |
| Status | Hold BOOT 0.6 s and release | Back to the pet immediately |
| Any screen | Hold BOOT 8 s | Restarts at the selection; red bar from 3 s |
| After death | Hold BOOT 0.6 s and release | Restarts at the selection |

### 8×8 menu

Order of the eight icons/dots: **apple (red) · gamepad (purple) · bubbles
(light blue) · pharmacy cross (green) · moon (yellow) · heart (pink) · bars ·
back (white)**. The white dot on the bottom row shows the position. The menu
closes after 8 s without input. Without the IMU, every care is still
reachable with BOOT.

**When the pet needs something**, every 4 s the screen briefly shows the
menu icon that fixes it. The rest of the time, a small dot blinks in the
top-right corner in that icon's color: green sick, light blue dirty, red
hungry, yellow tired, purple bored/sad. Holding BOOT opens the menu right on
that icon.

**Status**: first the pet's name scrolls by. Then one page per attribute:
icon at the top, an 8-LED bar at the bottom and the scrolling word. Full
COMIDA = satisfied, ALEGRIA, ENERGIA and SAUDE (becomes DOENTE or SUJO when
that applies; drops when you neglect it). A blinking icon = low attribute.
Finally, the age in days.

The menu suggests waking up if it is asleep; otherwise it prioritizes
sickness, dirt, hunger, tiredness and sadness. When everything is fine, it
suggests petting. Petting adds 5 joy without spending energy.

Shakes do not interrupt a care animation nor wake the pet. Food is refused
if it is already full; play, if it lacks energy. Holding for reset never
triggers an intermediate confirmation.

## Life and saving

The state is saved in NVS and survives restarts. Hunger, happiness and energy
decay slowly, with rates and personality in `src/Config.h` and `src/Dna.h`.
While asleep, energy regenerates and hunger drops more slowly. The clock only
runs while the board is powered.

Dirt and prolonged hunger cause sickness. Neglect builds up in stages:
normal → neglected → almost wild → wild. A wild pet gets skittish, forages on
its own and can die after five days in that state. Well cared for, it lives
forever.
