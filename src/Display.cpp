#include "Display.h"
#include "Config.h"
#include <FastLED.h>

namespace {
CRGB leds[MATRIX_W * MATRIX_H];

// Coordenada lógica (x à direita, y pra baixo) -> índice físico.
// Fiação progressiva (linha*8 + coluna), confirmada no pomodoro_cube.
uint16_t physicalIndex(uint8_t x, uint8_t y) {
    uint8_t px = x, py = y;
    switch (DISPLAY_ROTATION & 3) {
        case 1: px = MATRIX_W - 1 - y; py = x; break;
        case 2: px = MATRIX_W - 1 - x; py = MATRIX_H - 1 - y; break;
        case 3: px = y; py = MATRIX_H - 1 - x; break;
        default: break;
    }
    if (DISPLAY_MIRROR_X) px = MATRIX_W - 1 - px;
    return py * MATRIX_W + px;
}
} // namespace

namespace Display {

void begin() {
    FastLED.addLeds<WS2812B, PIN_MATRIX_DATA, GRB>(leds, MATRIX_W * MATRIX_H);
    FastLED.setBrightness(MAX_BRIGHTNESS);
    // Limite de corrente: se o quadro pedir mais que isso, o FastLED
    // reduz o brilho antes de mandar pro fio.
    FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);
    // Sem dithering temporal: a ~50 fps ele vira cintilação visível nos
    // tons escuros com brilho baixo.
    FastLED.setDither(0);
    FastLED.clear(true);
}

void show(const Canvas &c, bool night) {
    // Saída do FastLED = v * (BRIGHTNESS + 1) / 256: este v vira exatamente
    // NIGHT_LEVEL degraus no canal mais forte.
    const float nightV = (NIGHT_LEVEL * 256.0f + MAX_BRIGHTNESS) / (MAX_BRIGHTNESS + 1);
    for (uint8_t y = 0; y < MATRIX_H; y++) {
        for (uint8_t x = 0; x < MATRIX_W; x++) {
            const Rgb &p = c.px[y][x];
            // A arte é RGB de design; o brilho por cor (gama, azul, teto de
            // ofuscamento, piso das escuras) é aplicado uma vez, aqui no fim,
            // antes do teto global do FastLED. Ver art/led-profile.json.
            CRGB &out = leds[physicalIndex(x, y)];
            LedProfile::color(p.r, p.g, p.b, out.r, out.g, out.b);
            if (night) {
                uint8_t peak = out.r > out.g ? out.r : out.g;
                if (out.b > peak) peak = out.b;
                if (peak) {
                    float k = nightV / peak;
                    out.r = (uint8_t)(out.r * k);
                    out.g = (uint8_t)(out.g * k);
                    out.b = (uint8_t)(out.b * k);
                }
            }
        }
    }
    FastLED.show();
}

} // namespace Display
