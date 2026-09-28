#pragma once
#include <stddef.h>
#include <stdint.h>

// "DNA" do bichinho: 32 bits sorteados na escolha do ovo (MAC da placa +
// instante do clique + RNG de hardware). Cada fatia vira um traço fixo
// pra vida toda — invisíveis (ritmo) e um visível (tom de cor).
struct Dna {
    uint32_t bits;

    // 80..143 (%): quanto maior, mais rápido dá fome.
    uint8_t metabolism() const { return 80 + (bits & 0x3F); }
    // 80..143 (%): quanto maior, mais rápido fica carente/triste.
    uint8_t neediness() const { return 80 + ((bits >> 6) & 0x3F); }
    // 0..255: preguiçoso (0) a elétrico (255); pesa nas ações sozinho.
    uint8_t activity() const { return (bits >> 12) & 0xFF; }
    // 0..255: curiosidade; mais fuçar/olhar em volta.
    uint8_t curiosity() const { return (bits >> 20) & 0xFF; }

    // Nome próprio: 2 ou 3 sílabas consoante+vogal ("KALU", "MOBITE"),
    // tirado de um hash do DNA (só maiúsculas: é o que a fonte 3x5 tem).
    void name(char *out, size_t n) const {
        static const char CONS[] = "BCDFGJKLMNPRSTVZ";
        static const char VOW[] = "AEIOU";
        uint32_t h = bits * 2654435761u;
        h ^= h >> 15; h *= 2246822519u; h ^= h >> 13;
        uint8_t syllables = 2 + (h & 1);
        h >>= 1;
        size_t i = 0;
        for (uint8_t s = 0; s < syllables && i + 2 < n; s++) {
            out[i++] = CONS[h & 15]; h >>= 4;
            out[i++] = VOW[(h & 7) % 5]; h >>= 3;
        }
        if (n) out[i] = 0;
    }

    // Leve variação de cor (cada canal entre ~86% e 100%).
    uint32_t tone() const {
        uint8_t r = 220 + ((bits >> 26) & 0x3) * 11;
        uint8_t g = 220 + ((bits >> 28) & 0x3) * 11;
        uint8_t b = 220 + ((bits >> 30) & 0x3) * 11;
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
};

// Divide um intervalo "a cada N minutos" pelo fator em %, mínimo 1.
inline uint8_t scaledEvery(uint8_t base, uint8_t pct) {
    uint16_t v = (uint16_t)base * 100 / pct;
    return v < 1 ? 1 : (v > 255 ? 255 : v);
}
