#pragma once
#include <Arduino.h>

// Wrapper fino sobre o QMI8658 (via SensorLib) que expõe só o que o
// PixelGochi precisa: dois gestos, "chacoalhar" e "virar de cabeça
// pra baixo". Ver Config.h para os limiares.
namespace Imu {

bool begin();

// Chamar a cada loop(). Dispara callbacks em Pet conforme os gestos
// forem detectados (é o próprio módulo que decide os limiares).
void update();

} // namespace Imu
