#include "Input.h"
#include "Config.h"
#include "Pet.h"
#include <Arduino.h>

extern Pet pet;

namespace {
bool lastRawState = HIGH;
bool debouncedState = HIGH; // HIGH = solto (INPUT_PULLUP, ativo em LOW)
unsigned long lastEdgeAt = 0;
unsigned long pressStartedAt = 0;
bool longPressFired = false;
} // namespace

namespace Input {

void begin() {
    pinMode(PIN_BOOT_BUTTON, INPUT_PULLUP);
}

void update() {
    bool raw = digitalRead(PIN_BOOT_BUTTON);
    unsigned long now = millis();

    if (raw != lastRawState) {
        lastEdgeAt = now;
        lastRawState = raw;
    }

    if (now - lastEdgeAt >= BUTTON_DEBOUNCE_MS && raw != debouncedState) {
        debouncedState = raw;
        if (debouncedState == LOW) {
            // borda de descida: começou a pressionar
            pressStartedAt = now;
            longPressFired = false;
        } else {
            // borda de subida: soltou
            unsigned long heldMs = now - pressStartedAt;
            if (!longPressFired && heldMs < BUTTON_LONG_PRESS_MS) {
                pet.onButtonShortPress();
            }
        }
    }

    if (debouncedState == LOW && !longPressFired &&
        now - pressStartedAt >= BUTTON_LONG_PRESS_MS) {
        longPressFired = true;
        pet.onButtonLongPress();
    }
}

} // namespace Input
