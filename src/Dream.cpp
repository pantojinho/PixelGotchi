#include "Dream.h"
#include <string.h>

namespace Dream {
namespace {
struct Cell { uint8_t x, y; };

// Padrões em coordenadas locais (caixa de até 4x4).
const Cell GLIDER[] = {{1, 0}, {2, 1}, {0, 2}, {1, 2}, {2, 2}};
const Cell BLINKER[] = {{1, 0}, {1, 1}, {1, 2}};
const Cell TOAD[] = {{1, 0}, {2, 0}, {3, 0}, {0, 1}, {1, 1}, {2, 1}};
const Cell BEACON[] = {{0, 0}, {1, 0}, {0, 1}, {3, 2}, {2, 3}, {3, 3}};

uint32_t xorshift(uint32_t &s) {
    s ^= s << 13; s ^= s >> 17; s ^= s << 5;
    return s;
}
} // namespace

void Automaton::clear() {
    memset(rows_, 0, sizeof(rows_));
    generation_ = 0;
}

void Automaton::set(uint8_t x, uint8_t y) {
    rows_[y & 7] |= (uint8_t)(1u << (x & 7));
}

void Automaton::seedChapter(Kind kind, uint32_t seed) {
    clear();
    // Orientação (espelhos e transposição) e posição saem da semente. Aplicar
    // a mesma transformação ao quadro inteiro preserva o comportamento.
    const bool flipX = seed & 1, flipY = seed & 2, swap = seed & 4;
    const uint8_t ox = (seed >> 3) & 7, oy = (seed >> 6) & 7;
    auto place = [&](const Cell *cells, uint8_t count, uint8_t dx, uint8_t dy) {
        for (uint8_t i = 0; i < count; ++i) {
            uint8_t x = cells[i].x + dx, y = cells[i].y + dy;
            if (flipX) x = 7 - x;
            if (flipY) y = 7 - y;
            if (swap) { const uint8_t t = x; x = y; y = t; }
            set(x + ox, y + oy);
        }
    };
    switch (kind) {
        case Kind::Gliders:
            place(GLIDER, 5, 0, 0);
            // Duas naves na mesma direção, a (4,4) uma da outra, não se tocam no toro.
            if ((seed >> 9) & 1) place(GLIDER, 5, 4, 4);
            break;
        case Kind::Pulse:
            switch ((seed >> 10) & 3) {
                case 0: place(BLINKER, 3, 0, 0); break;
                case 1: place(TOAD, 6, 0, 0); break;
                case 2: place(BEACON, 6, 0, 0); break;
                default: place(BLINKER, 3, 0, 0); place(BLINKER, 3, 4, 4); break;
            }
            break;
        case Kind::Soup: {
            // Campo esparso; descarta sementes que se apagam antes de mostrar algo.
            uint32_t rng = seed ^ 0x9E3779B9u;
            if (!rng) rng = 0xA341316Cu;
            for (uint8_t attempt = 0; attempt < 6; ++attempt) {
                clear();
                const uint8_t density = 20 + xorshift(rng) % 13; // 20..32%
                for (uint8_t y = 0; y < 8; ++y)
                    for (uint8_t x = 0; x < 8; ++x)
                        if (xorshift(rng) % 100 < density) set(x, y);
                Automaton probe = *this;
                for (uint8_t i = 0; i < 8; ++i) probe.step();
                if (probe.population()) break;
            }
            generation_ = 0;
            if (!population()) place(GLIDER, 5, 0, 0);
            break;
        }
    }
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
            if (neighbors == 3 || (alive(x, y) && neighbors == 2)) next[y] |= (uint8_t)(1u << x);
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
