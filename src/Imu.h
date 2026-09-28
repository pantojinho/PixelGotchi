#pragma once
#include <stdint.h>

// QMI8658 (via SensorLib). Publica Ev::Shake / Ev::FaceDown / Ev::FaceUp
// e expõe inclinação e "está mexendo" pra quem precisar ler continuamente.
namespace Imu {

bool begin();
void update();

bool ok();
float tilt();              // ~ -1 (esquerda pra baixo) .. +1 (direita pra baixo)
uint32_t lastMotionMs();   // millis() do último movimento perceptível
void accel(float &x, float &y, float &z); // m/s^2, pra debug/calibração

} // namespace Imu
