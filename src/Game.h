#pragma once
#include <stdint.h>

// Cenas, entrada e desenho. PetSim cuida das regras; aqui é tudo que se vê.
namespace Game {
void begin();
void update(); // chamar todo loop(): consome eventos, avança animações e desenha
// Teste de cor: pinta a tela em 4 quadrantes (0xRRGGBB, ordem ↖ ↗ ↙ ↘) por
// `ms`, pelo mesmo caminho de cor/brilho do jogo. Usado pelo comando "cores".
void showSwatches(const uint32_t colors[4], uint32_t ms);
// Bichinho do editor (CustomPet): está em uso, foi trocado/apagado, ou adotar agora.
bool customPetActive();
void customPetChanged();
void adoptCustomPet();
} // namespace Game
