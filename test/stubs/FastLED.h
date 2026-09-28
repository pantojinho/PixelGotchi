#pragma once
#include <stdint.h>

struct CRGB {
    uint8_t r, g, b;
    CRGB(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) : r(red), g(green), b(blue) {}
};
struct WS2812B {};
enum EOrder { RGB, GRB };

// Captura a saída do Display real. Não simula a eletrônica/óptica dos LEDs.
struct FakeFastLED {
    CRGB *pixels = nullptr;
    int count = 0;
    EOrder order = GRB;
    uint8_t brightness = 255, dither = 1, volts = 0;
    uint16_t milliamps = 0;
    template<typename Chip, uint8_t Pin, EOrder Order>
    void addLeds(CRGB *data, int size) { pixels = data; count = size; order = Order; }
    void setBrightness(uint8_t value) { brightness = value; }
    void setMaxPowerInVoltsAndMilliamps(uint8_t v, uint16_t ma) { volts = v; milliamps = ma; }
    void setDither(uint8_t value) { dither = value; }
    void clear(bool) { for (int i = 0; i < count; i++) pixels[i] = CRGB(); }
    void show() {}
};
extern FakeFastLED FastLED;
