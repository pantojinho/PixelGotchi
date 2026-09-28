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

// Comandos simples pela USB:
//   "imu on" / "imu off"         — stream do acelerômetro (tools/calibrate_imu.py)
//   "cores RRGGBB RRGGBB RRGGBB RRGGBB" — 4 quadrantes de cor por 60 s, pra
//                                  comparar cores nos LEDs de verdade
void handleSerial() {
    static char line[64];
    static uint8_t len = 0;
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            line[len] = 0;
            uint32_t colors[4];
            if (strcmp(line, "imu on") == 0) Imu::setStreaming(true);
            else if (strcmp(line, "imu off") == 0) Imu::setStreaming(false);
            else if (sscanf(line, "cores %lx %lx %lx %lx", &colors[0], &colors[1], &colors[2], &colors[3]) == 4) {
                Game::showSwatches(colors, 60000);
                Serial.println("[cores] ok");
            }
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
