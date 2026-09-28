#pragma once
#include <stdint.h>

namespace Art {

// Pixels guardam índices na paleta; 0 = transparente (LED apagado).
struct Sprite {
    uint8_t w, h;
    const uint8_t *px;   // w*h índices, linha a linha
    const uint32_t *pal; // 0xRRGGBB; pal[0] não é usado
};

struct Anim {
    uint16_t frameMs;
    uint8_t count;
    const Sprite *const *frames;
};

// Ordem dos campos Anim* casa com PET_ANIMS + PET_EGG_ANIMS em tools/gen_art.py.
struct PetDef {
    const char *id;
    const char *name;
    const Anim *idle, *blink, *walk, *eat, *sleep, *happy, *sad, *hungry, *egg;
    const Sprite *food;
    const uint32_t *wildPal; // mesma ordem de cores da paleta normal; nullptr se não tiver
    bool side;               // desenhado de perfil olhando pra direita (espelha ao andar pra esquerda)
};

inline const Sprite &frameAt(const Anim &a, uint32_t elapsedMs) {
    return *a.frames[(elapsedMs / a.frameMs) % a.count];
}

inline uint32_t animDuration(const Anim &a) { return (uint32_t)a.frameMs * a.count; }

} // namespace Art
