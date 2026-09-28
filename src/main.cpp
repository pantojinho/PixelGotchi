#include <Arduino.h>
#include "Display.h"
#include "Game.h"
#include "Imu.h"
#include "Input.h"

void setup() {
    Serial.begin(115200);
    uint32_t waitUntil = millis() + 2000;
    while (!Serial && millis() < waitUntil) delay(10);
    Serial.println("\n[PixelGochi] iniciando...");

    Display::begin();
    Input::begin();
    if (!Imu::begin()) Serial.println("[PixelGochi] sem IMU: gestos nao vao funcionar");
    Game::begin();
}

// Comandos simples pela USB (usados por tools/calibrate_imu.py).
void handleSerial() {
    static char line[24];
    static uint8_t len = 0;
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            line[len] = 0;
            if (strcmp(line, "imu on") == 0) Imu::setStreaming(true);
            else if (strcmp(line, "imu off") == 0) Imu::setStreaming(false);
            len = 0;
        } else if (len < sizeof(line) - 1) {
            line[len++] = c;
        }
    }
}

void loop() {
    handleSerial();
    Input::update();
    Imu::update();
    Game::update();
}
