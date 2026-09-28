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

void loop() {
    Input::update();
    Imu::update();
    Game::update();
}
