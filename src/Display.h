#pragma once
#include "Pet.h"

namespace Display {

void begin();
void showEggStage(uint8_t stage); // 0-3
void showCreature(Expression expr, const Species &sp);

// Acende os 4 cantos em sequência (vermelho/verde/azul/branco) por 1s
// cada. Serve pra confirmar visualmente a ordem de varredura da matriz;
// se a orientação do "rosto" desenhado parecer girada/espelhada na
// prática, ajuste as flags do Adafruit_NeoMatrix em Display.cpp.
void calibrationTest();

} // namespace Display
