#pragma once
#include <stdint.h>

namespace Dream {

// Tipo de capítulo: cada um tem uma intenção visual clara em 64 pixels.
enum class Kind : uint8_t {
    Gliders, // uma ou duas naves atravessando a matriz (deslocamento)
    Pulse,   // osciladores: blinker, toad, beacon (pequena pulsação)
    Soup,    // campo esparso: grupos nascendo e se desfazendo
};

// Autômato de Conway 8x8. As bordas se conectam (mundo toroidal).
class Automaton {
public:
    void clear();
    void set(uint8_t x, uint8_t y);
    // Semente de um capítulo. A mesma (kind, seed) gera sempre o mesmo quadro;
    // a semente escolhe padrão, orientação e posição.
    void seedChapter(Kind kind, uint32_t seed);
    void step();
    bool alive(uint8_t x, uint8_t y) const;
    uint8_t population() const;
    uint64_t signature() const;
    uint32_t generation() const { return generation_; }

private:
    uint8_t rows_[8]{};
    uint32_t generation_ = 0;
};

} // namespace Dream
