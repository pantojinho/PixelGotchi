// GERADO de art/led-profile.json por tools/gen_art.py -- não edite à mão.
#pragma once
#include <stdint.h>

namespace LedProfile {
constexpr uint8_t BRIGHTNESS = 13;
// Curva suave gamma 1.6; mantém preto/primárias e separa meios-tons.
constexpr uint8_t GAMMA_LUT[256] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3,
    3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 9,
    9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17,
    18, 18, 19, 19, 20, 21, 21, 22, 23, 23, 24, 25, 25, 26, 27, 27,
    28, 29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39,
    40, 41, 42, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53,
    53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 64, 65, 66, 67,
    68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 83, 84,
    85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 97, 98, 99, 100, 101,
    102, 103, 104, 106, 107, 108, 109, 110, 111, 113, 114, 115, 116, 117, 119, 120,
    121, 122, 123, 125, 126, 127, 128, 130, 131, 132, 133, 135, 136, 137, 138, 140,
    141, 142, 143, 145, 146, 147, 149, 150, 151, 153, 154, 155, 157, 158, 159, 161,
    162, 163, 165, 166, 167, 169, 170, 171, 173, 174, 176, 177, 178, 180, 181, 183,
    184, 185, 187, 188, 190, 191, 193, 194, 196, 197, 198, 200, 201, 203, 204, 206,
    207, 209, 210, 212, 213, 215, 216, 218, 219, 221, 222, 224, 225, 227, 228, 230,
    231, 233, 235, 236, 238, 239, 241, 242, 244, 245, 247, 249, 250, 252, 253, 255,
};
constexpr uint16_t GLARE_CAP = 420;
constexpr uint8_t BLUE_GAIN = 204; // /255
constexpr uint8_t MIN_PEAK = 3;
inline uint8_t channel(uint8_t value) { return GAMMA_LUT[value]; }

// Brilho por cor, antes do teto global do FastLED (BRIGHTNESS):
// 1) gama; 2) azul atenuado; 3) soma R+G+B limitada a GLARE_CAP (branco e
// tons claros ofuscam menos, cores puras intactas); 4) cor acesa nunca fica
// abaixo de MIN_PEAK níveis na saída (escuras não somem), mantendo o tom.
// Espelhado em preview/index.html (ledColor).
inline void color(uint8_t r, uint8_t g, uint8_t b, uint8_t &outR, uint8_t &outG, uint8_t &outB) {
    float fr = GAMMA_LUT[r], fg = GAMMA_LUT[g], fb = GAMMA_LUT[b] * BLUE_GAIN / 255.0f;
    float sum = fr + fg + fb;
    if (sum > GLARE_CAP) {
        float k = GLARE_CAP / sum;
        fr *= k; fg *= k; fb *= k;
    }
    float peak = fr > fg ? (fr > fb ? fr : fb) : (fg > fb ? fg : fb);
    // saída do FastLED = v * (BRIGHTNESS + 1) / 256; arredonda pra cima
    const float floorV = (MIN_PEAK * 256 + BRIGHTNESS) / (BRIGHTNESS + 1);
    if (peak > 0 && peak < floorV) {
        float k = floorV / peak;
        fr *= k; fg *= k; fb *= k;
    }
    auto q = [](float v) -> uint8_t { return v >= 255 ? 255 : (uint8_t)(v + 0.5f); };
    outR = q(fr); outG = q(fg); outB = q(fb);
}
} // namespace LedProfile
