#include "Display.h"
#include "Config.h"
#include "PixelArt.h"
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

namespace {

// Palpite de partida pra ordem de varredura dos 64 WS2812B — comum em
// painéis 8x8 baratos, mas não confirmado para esta placa específica.
// Rode Display::calibrationTest() depois de flashear pra validar; se os
// cantos acenderem fora de ordem/espelhados, troqueas flags abaixo.
Adafruit_NeoMatrix matrix(MATRIX_W, MATRIX_H, PIN_MATRIX_DATA,
                          NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                          NEO_GRB + NEO_KHZ800);

uint16_t colorForEgg(uint8_t idx) {
    switch (idx) {
        case 1: return matrix.Color(235, 220, 180); // casca
        case 3: return matrix.Color(255, 255, 255);  // brilho
        case 4: return matrix.Color(110, 70, 30);    // rachadura
        default: return matrix.Color(0, 0, 0);
    }
}

uint16_t colorForCreature(uint8_t idx, const Species &sp) {
    switch (idx) {
        case 1: return matrix.Color(sp.r, sp.g, sp.b);
        case 2: return matrix.Color(10, 10, 10);
        case 3: return matrix.Color(255, 255, 255);
        default: return matrix.Color(0, 0, 0);
    }
}

void drawSprite(const Sprite8x8 &grid, uint16_t (*colorFn)(uint8_t)) {
    for (uint8_t y = 0; y < MATRIX_H; y++) {
        for (uint8_t x = 0; x < MATRIX_W; x++) {
            matrix.drawPixel(x, y, colorFn(grid[y][x]));
        }
    }
}

const Sprite8x8 *eggSpriteFor(uint8_t stage) {
    switch (stage) {
        case 0: return &EGG_STAGE_0;
        case 1: return &EGG_STAGE_1;
        case 2: return &EGG_STAGE_2;
        default: return &EGG_STAGE_3;
    }
}

const Sprite8x8 *creatureSpriteFor(Expression expr) {
    switch (expr) {
        case Expression::IDLE_BLINK: return &CREATURE_IDLE_BLINK;
        case Expression::HAPPY: return &CREATURE_HAPPY;
        case Expression::HUNGRY: return &CREATURE_HUNGRY;
        case Expression::SAD: return &CREATURE_SAD;
        default: return &CREATURE_IDLE_OPEN;
    }
}

} // namespace

namespace Display {

void begin() {
    matrix.begin();
    matrix.setBrightness(MAX_BRIGHTNESS);
    matrix.fillScreen(0);
    matrix.show();
}

void showEggStage(uint8_t stage) {
    drawSprite(*eggSpriteFor(stage), colorForEgg);
    matrix.show();
}

void showCreature(Expression expr, const Species &sp) {
    const Sprite8x8 &grid = *creatureSpriteFor(expr);
    for (uint8_t y = 0; y < MATRIX_H; y++) {
        for (uint8_t x = 0; x < MATRIX_W; x++) {
            matrix.drawPixel(x, y, colorForCreature(grid[y][x], sp));
        }
    }
    if (sp.hasEars) {
        uint16_t earColor = matrix.Color(sp.r, sp.g, sp.b);
        matrix.drawPixel(1, 0, earColor);
        matrix.drawPixel(6, 0, earColor);
    }
    matrix.show();
}

void calibrationTest() {
    struct Corner { uint8_t x, y; uint8_t r, g, b; const char *name; };
    static const Corner corners[] = {
        {0, 0, 255, 0, 0, "top-left (vermelho)"},
        {7, 0, 0, 255, 0, "top-right (verde)"},
        {0, 7, 0, 0, 255, "bottom-left (azul)"},
        {7, 7, 255, 255, 255, "bottom-right (branco)"},
    };
    for (const auto &c : corners) {
        matrix.fillScreen(0);
        matrix.drawPixel(c.x, c.y, matrix.Color(c.r, c.g, c.b));
        matrix.show();
        Serial.printf("[Display] calibracao: %s\n", c.name);
        delay(1000);
    }
    matrix.fillScreen(0);
    matrix.show();
}

} // namespace Display
