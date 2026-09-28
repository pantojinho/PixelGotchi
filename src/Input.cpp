#include "Input.h"
#include "Config.h"
#include "Events.h"

namespace {
bool lastRaw = HIGH;
bool pressed = false;
uint32_t lastEdgeAt = 0;
uint32_t pressedAt = 0;
bool longSent = false;
bool resetSent = false;
} // namespace

namespace Input {

void begin() {
    pinMode(PIN_BOOT_BUTTON, INPUT_PULLUP);
}

void update() {
    uint32_t now = millis();
    bool raw = digitalRead(PIN_BOOT_BUTTON);
    if (raw != lastRaw) {
        lastRaw = raw;
        lastEdgeAt = now;
    }
    if (now - lastEdgeAt < BUTTON_DEBOUNCE_MS) return;

    bool down = (raw == LOW);
    if (down && !pressed) {
        pressed = true;
        pressedAt = now;
        longSent = resetSent = false;
    } else if (!down && pressed) {
        pressed = false;
        if (!longSent) Events::push(Ev::Short);
    }

    if (pressed) {
        uint32_t held = now - pressedAt;
        if (!longSent && held >= BUTTON_LONG_MS) {
            longSent = true;
            Events::push(Ev::Long);
        }
        if (!resetSent && held >= BUTTON_RESET_MS) {
            resetSent = true;
            Events::push(Ev::Reset);
        }
    }
}

uint32_t heldMs() {
    return pressed ? millis() - pressedAt : 0;
}

} // namespace Input
