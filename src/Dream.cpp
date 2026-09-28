#include "Dream.h"
#include <string.h>

namespace Dream {
namespace {
void put(uint8_t rows[8], uint8_t x, uint8_t y) { rows[y] |= (uint8_t)(1u << x); }
} // namespace

void Automaton::seed(uint32_t seedValue, bool calm) {
    memset(rows_, 0, sizeof(rows_));
    generation_ = 0;
    if (calm) {
        // Sementes reconhecíveis e estáveis: osciladores e um bloco imóvel.
        switch (seedValue % 4) {
            case 0:
                put(rows_, 2, 3); put(rows_, 3, 3); put(rows_, 4, 3); // blinker
                break;
            case 1:
                put(rows_, 3, 2); put(rows_, 4, 2); put(rows_, 5, 2);
                put(rows_, 2, 3); put(rows_, 3, 3); put(rows_, 4, 3); // toad
                break;
            default:
                if (seedValue % 4 == 2) {
                    put(rows_, 3, 3); put(rows_, 4, 3);
                    put(rows_, 3, 4); put(rows_, 4, 4); // bloco imóvel
                } else {
                    // Glider, a nave que cruza a matriz e reaparece na borda.
                    put(rows_, 3, 2);
                    put(rows_, 4, 3);
                    put(rows_, 2, 4); put(rows_, 3, 4); put(rows_, 4, 4);
                }
                break;
        }
        // O mesmo padrão pode visitar outras posições e orientações.
        uint8_t transformed[8]{};
        const uint8_t turns = (seedValue >> 2) & 3;
        for (uint8_t y = 0; y < 8; ++y) for (uint8_t x = 0; x < 8; ++x) {
            if (!alive(x, y)) continue;
            uint8_t nx = x, ny = y;
            for (uint8_t i = 0; i < turns; ++i) {
                const uint8_t oldX = nx; nx = 7 - ny; ny = oldX;
            }
            put(transformed, (nx + ((seedValue >> 4) & 7)) % 8,
                (ny + ((seedValue >> 7) & 7)) % 8);
        }
        memcpy(rows_, transformed, sizeof(rows_));
        return;
    }

    // DNA e atributos escolhem um campo esparso que tende a se apagar rápido.
    uint32_t rng = seedValue ^ 0x9E3779B9u;
    if (!rng) rng = 0xA341316Cu;
    for (uint8_t y = 0; y < 8; ++y) {
        for (uint8_t x = 0; x < 8; ++x) {
            rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
            if (rng % 100 < 18) put(rows_, x, y);
        }
    }
    if (!population()) put(rows_, 3, 3);
}

void Automaton::step() {
    uint8_t next[8]{};
    for (uint8_t y = 0; y < 8; ++y) {
        for (uint8_t x = 0; x < 8; ++x) {
            uint8_t neighbors = 0;
            for (int8_t dy = -1; dy <= 1; ++dy) {
                for (int8_t dx = -1; dx <= 1; ++dx) {
                    if (!dx && !dy) continue;
                    const uint8_t nx = (uint8_t)((x + dx + 8) % 8);
                    const uint8_t ny = (uint8_t)((y + dy + 8) % 8);
                    if (alive(nx, ny)) ++neighbors;
                }
            }
            if (neighbors == 3 || (alive(x, y) && neighbors == 2)) put(next, x, y);
        }
    }
    memcpy(rows_, next, sizeof(rows_));
    ++generation_;
}

bool Automaton::alive(uint8_t x, uint8_t y) const {
    return x < 8 && y < 8 && (rows_[y] & (1u << x));
}

uint8_t Automaton::population() const {
    uint8_t count = 0;
    for (uint8_t row : rows_)
        for (uint8_t x = 0; x < 8; ++x) count += (row >> x) & 1u;
    return count;
}

uint64_t Automaton::signature() const {
    uint64_t bits = 0;
    for (uint8_t y = 0; y < 8; ++y) bits |= (uint64_t)rows_[y] << (8 * y);
    return bits;
}

} // namespace Dream
