#include "Imu.h"
#include "Config.h"
#include "Events.h"
#include <Wire.h>
#include <ImuDrv.hpp>

namespace {
SensorQMI8658 imu;
bool ready = false;

float ax = 0, ay = 0, az = GRAVITY;
float sx = 0, sy = 0, sz = GRAVITY; // média móvel: "onde a gravidade está"
float tiltValue = 0;
uint32_t motionAt = 0;
uint32_t shakeAt = 0;

bool faceDown = false;
uint32_t faceDownSince = 0;
bool faceDownSent = false;
bool firstSample = true;
} // namespace

namespace Imu {

bool begin() {
    // Endereço L (0x6B) confirmado no hardware real; H como reserva.
    ready = imu.begin(Wire, QMI8658_L_SLAVE_ADDRESS, PIN_IMU_SDA, PIN_IMU_SCL) ||
            imu.begin(Wire, QMI8658_H_SLAVE_ADDRESS, PIN_IMU_SDA, PIN_IMU_SCL);
    if (!ready) {
        Serial.println("[Imu] QMI8658 nao respondeu");
        return false;
    }
    imu.configAccel(AccelFullScaleRange::FS_8G, 250.0f, SensorQMI8658::LpfMode::MODE_0);
    imu.enableAccel();
    Serial.println("[Imu] QMI8658 ok");
    return true;
}

void update() {
    if (!ready) return;
    if (!imu.isDataReady(static_cast<uint8_t>(ImuBase::DataReadyMask::ACCEL))) return;

    AccelerometerData a;
    imu.readAccel(a);
    ax = a.mps2.x;
    ay = a.mps2.y;
    az = a.mps2.z;

    // A orientação inicial não é movimento. Evita sacudida fantasma
    // e incubação de um ovo parado ao ligar deitado ou de costas.
    if (firstSample) {
        sx = ax; sy = ay; sz = az;
        firstSample = false;
    }

    // Movimento = quanto a leitura atual se afasta da média recente. Pega
    // tanto sacudida quanto girar a placa devagar (muda a direção da
    // gravidade), que é o "ficar mexendo" que o ovo precisa.
    float dx = ax - sx, dy = ay - sy, dz = az - sz;
    float delta = sqrtf(dx * dx + dy * dy + dz * dz);
    const float k = 0.08f;
    sx += dx * k;
    sy += dy * k;
    sz += dz * k;

    uint32_t now = millis();
    if (delta > MOTION_THRESHOLD) motionAt = now;
    if (delta > SHAKE_THRESHOLD && now - shakeAt > SHAKE_COOLDOWN_MS) {
        shakeAt = now;
        Events::push(Ev::Shake);
    }

    // Inclinação lateral a partir da gravidade suavizada (ver TILT_SIGN).
    float t = TILT_SIGN * sy / GRAVITY;
    tiltValue = t < -1 ? -1 : (t > 1 ? 1 : t);

    bool down = faceDown ? sz < FACE_UP_Z : sz < FACE_DOWN_Z;
    if (down && !faceDown) {
        faceDown = true;
        faceDownSince = now;
        faceDownSent = false;
    } else if (!down && faceDown) {
        faceDown = false;
        if (faceDownSent) Events::push(Ev::FaceUp);
    }
    if (faceDown && !faceDownSent && now - faceDownSince >= FACE_DOWN_HOLD_MS) {
        faceDownSent = true;
        Events::push(Ev::FaceDown);
    }
}

bool ok() { return ready; }
float tilt() { return tiltValue; }
uint32_t lastMotionMs() { return motionAt; }

void accel(float &x, float &y, float &z) {
    x = ax;
    y = ay;
    z = az;
}

} // namespace Imu
