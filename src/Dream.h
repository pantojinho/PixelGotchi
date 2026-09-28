#pragma once
#include <stdint.h>

namespace Dream {

// Autômato de Conway 8x8. As bordas se conectam (mundo toroidal).
class Automaton {
public:
    void seed(uint32_t seed, bool calm);
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
