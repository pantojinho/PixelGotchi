#pragma once

// Cenas, entrada e desenho. PetSim cuida das regras; aqui é tudo que se vê.
namespace Game {
void begin();
void update(); // chamar todo loop(): consome eventos, avança animações e desenha
} // namespace Game
