#pragma once
#include <stdint.h>
#include "Config.h"
#include "art/ArtTypes.h"

// Tratamentos de cor aplicados ao desenhar um sprite.
enum class Tint : uint8_t {
    None,
    Sick,   // puxa pro verde
    Ghost,  // azul-claro translúcido
};

struct Rgb {
    uint8_t r, g, b;
};

// Como desenhar um sprite: paleta alternativa (mesma ordem de cores),
// tinta e um "tom" multiplicativo por canal (0xRRGGBB, FF = sem mudança)
// usado pra dar a cada bicho uma leve variação de cor vinda do DNA.
struct Look {
    const uint32_t *pal = nullptr;
    Tint tint = Tint::None;
    uint32_t tone = 0xFFFFFF;
};

class Canvas {
public:
    void clear();
    void set(int x, int y, Rgb c);
    Rgb get(int x, int y) const;

    // Canto superior esquerdo em (x, y); o que sair da tela é cortado.
    void blit(const Art::Sprite &s, int x, int y, bool flipX = false, const Look &look = Look());
    // Ancorado pelo centro-inferior (cx, bottomY).
    void blitAnchored(const Art::Sprite &s, int cx, int bottomY, bool flipX = false, const Look &look = Look());

    // Texto na fonte 3x5 (art/font.art); devolve a largura em px.
    int text(const char *str, int x, int y, Rgb color);
    static int textWidth(const char *str);

    void scale(uint8_t amount); // multiplica o quadro inteiro por amount/255

    Rgb px[MATRIX_H][MATRIX_W];
};

Rgb rgb(uint32_t hex);
Rgb dim(Rgb c, uint8_t amount);
