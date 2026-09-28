#include "Canvas.h"
#include "art/ArtData.h"
#include <string.h>

Rgb rgb(uint32_t hex) {
    return {(uint8_t)(hex >> 16), (uint8_t)(hex >> 8), (uint8_t)hex};
}

Rgb dim(Rgb c, uint8_t amount) {
    return {(uint8_t)(c.r * amount / 255), (uint8_t)(c.g * amount / 255), (uint8_t)(c.b * amount / 255)};
}

namespace {
uint8_t sat8(int v) { return v > 255 ? 255 : (v < 0 ? 0 : v); }

Rgb applyLook(Rgb c, const Look &look) {
    if (look.tone != 0xFFFFFF) {
        c = {(uint8_t)(c.r * ((look.tone >> 16) & 0xFF) / 255),
             (uint8_t)(c.g * ((look.tone >> 8) & 0xFF) / 255),
             (uint8_t)(c.b * (look.tone & 0xFF) / 255)};
    }
    int l = (c.r + c.g + c.b) / 3;
    switch (look.tint) {
        case Tint::Sick:
            // mantém a luminosidade, puxa o matiz pra um verde meio doente
            return {sat8(l * 6 / 10), sat8(l + 40), sat8(l * 4 / 10)};
        case Tint::Ghost:
            return {sat8(l / 2), sat8(l * 7 / 10), sat8(l)};
        default:
            return c;
    }
}
} // namespace

void Canvas::clear() { memset(px, 0, sizeof(px)); }

void Canvas::set(int x, int y, Rgb c) {
    if (x < 0 || y < 0 || x >= MATRIX_W || y >= MATRIX_H) return;
    px[y][x] = c;
}

Rgb Canvas::get(int x, int y) const {
    if (x < 0 || y < 0 || x >= MATRIX_W || y >= MATRIX_H) return {0, 0, 0};
    return px[y][x];
}

void Canvas::blit(const Art::Sprite &s, int x, int y, bool flipX, const Look &look) {
    const uint32_t *pal = look.pal ? look.pal : s.pal;
    for (int sy = 0; sy < s.h; sy++) {
        for (int sx = 0; sx < s.w; sx++) {
            uint8_t idx = s.px[sy * s.w + (flipX ? s.w - 1 - sx : sx)];
            if (idx) set(x + sx, y + sy, applyLook(rgb(pal[idx]), look));
        }
    }
}

void Canvas::blitAnchored(const Art::Sprite &s, int cx, int bottomY, bool flipX, const Look &look) {
    blit(s, cx - s.w / 2, bottomY - s.h + 1, flipX, look);
}

int Canvas::textWidth(const char *str) {
    int w = 0;
    for (const char *p = str; *p; p++) {
        if (*p == ' ') {
            w += 2;
        } else if (const Art::Sprite *g = Art::glyph(*p)) {
            w += g->w + 1;
        }
    }
    return w > 0 ? w - 1 : 0;
}

int Canvas::text(const char *str, int x, int y, Rgb color) {
    int start = x;
    for (const char *p = str; *p; p++) {
        if (*p == ' ') {
            x += 2;
            continue;
        }
        const Art::Sprite *g = Art::glyph(*p);
        if (!g) continue;
        for (int gy = 0; gy < g->h; gy++)
            for (int gx = 0; gx < g->w; gx++)
                if (g->px[gy * g->w + gx]) set(x + gx, y + gy, color);
        x += g->w + 1;
    }
    return x - start;
}

void Canvas::scale(uint8_t amount) {
    for (auto &row : px)
        for (auto &c : row) c = dim(c, amount);
}
