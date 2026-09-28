#include "Imu.h"
#include "Config.h"
#include "Pet.h"
#include <Wire.h>
#include <ImuDrv.hpp>

extern Pet pet;

namespace {
SensorQMI8658 imu;
bool ready = false;

unsigned long lastShakeAt = 0;
bool flipped = false;
unsigned long flipStartedAt = 0;
bool flipHoldFired = false;
} // namespace

namespace Imu {

bool begin() {
    // A Waveshare não documenta o endereço I2C do QMI8658 onboard;
    // tenta os dois endereços possíveis do chip.
    ready = imu.begin(Wire, QMI8658_H_SLAVE_ADDRESS, PIN_IMU_SDA, PIN_IMU_SCL) ||
            imu.begin(Wire, QMI8658_L_SLAVE_ADDRESS, PIN_IMU_SDA, PIN_IMU_SCL);
    if (!ready) {
        Serial.println("[Imu] QMI8658 nao respondeu em nenhum endereco I2C");
        return false;
    }

    imu.configAccel(AccelFullScaleRange::FS_8G, 1000.0f, SensorQMI8658::LpfMode::MODE_0);
    imu.enableAccel();

    Serial.println("[Imu] QMI8658 ok");
    return true;
}

void update() {
    if (!ready) return;
    if (!imu.isDataReady(static_cast<uint8_t>(ImuBase::DataReadyMask::ACCEL))) return;

    AccelerometerData accel;
    imu.readAccel(accel);

#ifdef DEBUG_IMU
    Serial.printf("[Imu] accel m/s2: %6.2f %6.2f %6.2f\n", accel.mps2.x, accel.mps2.y, accel.mps2.z);
#endif

    float magnitude = sqrtf(accel.mps2.x * accel.mps2.x +
                             accel.mps2.y * accel.mps2.y +
                             accel.mps2.z * accel.mps2.z);
    unsigned long now = millis();

    // --- Shake: desvio abrupto da gravidade (1g em repouso) ---
    if (fabsf(magnitude - GRAVITY_MPS2) > SHAKE_THRESHOLD_MPS2 &&
        now - lastShakeAt > SHAKE_COOLDOWN_MS) {
        lastShakeAt = now;
        pet.onShake();
    }

    // --- Flip: placa de cabeça pra baixo, sustentado ---
    bool nowFlipped = accel.mps2.z < FLIP_Z_THRESHOLD_MPS2;
    if (nowFlipped && !flipped) {
        flipped = true;
        flipStartedAt = now;
        flipHoldFired = false;
    } else if (!nowFlipped && flipped) {
        flipped = false;
        if (flipHoldFired) pet.onFlipRelease();
    }
    if (flipped && !flipHoldFired && now - flipStartedAt >= FLIP_HOLD_MS) {
        flipHoldFired = true;
        pet.onFlipHold();
    }
}

} // namespace Imu
