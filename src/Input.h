#pragma once
#include <stdint.h>

// Botão BOOT (GPIO0). Publica Ev::Short / Ev::Long / Ev::Reset.
// Segurar o BOOT enquanto a placa liga/reseta entra em modo de gravação —
// comportamento normal do ESP32, não do firmware.
namespace Input {

void begin();
void update();

// Há quanto tempo o botão está pressionado (0 se solto) — usado pra
// desenhar a barra de "segure pra recomeçar".
uint32_t heldMs();

} // namespace Input
