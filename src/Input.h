#pragma once

// Botão BOOT (GPIO0): clique curto = alimentar, clique longo = alternar sono.
// Lembrete: segurar o BOOT enquanto a placa liga/reseta entra em modo de
// gravação — isso é comportamento normal do ESP32, não bug do firmware.
namespace Input {

void begin();
void update(); // chamar todo loop()

} // namespace Input
