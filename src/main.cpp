#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
#include "Pet.h"
#include "Display.h"
#include "Imu.h"
#include "Input.h"

Pet pet; // definido aqui, referenciado via `extern Pet pet;` nos outros módulos

void setup() {
    Serial.begin(115200);
    unsigned long serialWaitUntil = millis() + 3000;
    while (!Serial && millis() < serialWaitUntil) delay(10);

    Serial.println("\n[PixelGochi] iniciando...");

    Wire.begin(PIN_IMU_SDA, PIN_IMU_SCL);

    Display::begin();
    Input::begin();
    pet.begin();

    if (!Imu::begin()) {
        Serial.println("[PixelGochi] seguindo sem IMU (so bota/gestos nao vao funcionar)");
    }

    // Segure o BOTAO BOOT durante o boot (sem estar em modo de gravacao)
    // pra rodar o teste de calibracao da matriz uma vez.
    if (digitalRead(PIN_BOOT_BUTTON) == LOW) {
        Serial.println("[PixelGochi] BOOT pressionado no boot -> calibracao da matriz");
        Display::calibrationTest();
    }

    Serial.printf("[PixelGochi] estado carregado: hatched=%d species=%d hunger=%d happy=%d energy=%d\n",
                  pet.state().hatched, pet.state().species, pet.state().hunger,
                  pet.state().happiness, pet.state().energy);
}

void loop() {
    Imu::update();
    Input::update();
    pet.update();

    if (!pet.state().hatched) {
        Display::showEggStage(pet.eggStage());
    } else {
        Display::showCreature(pet.currentExpression(), pet.currentSpecies());
    }
}
