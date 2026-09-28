// GERADO por tools/calibrate_imu.py a partir das leituras da placa.
// Pra refazer: python tools/calibrate_imu.py
#pragma once
#include <stdint.h>

// Eixo (0=x, 1=y, 2=z) em que a gravidade aparece com a tela pra cima, e o
// sinal que deixa esse valor positivo nessa posição.
constexpr uint8_t SCREEN_AXIS = 2;  // z
constexpr float SCREEN_SIGN = -1.0f;

// Eixo que muda ao inclinar pra direita/esquerda, e o sinal que deixa
// "lado direito pra baixo" positivo.
constexpr uint8_t TILT_AXIS = 1;  // y
constexpr float TILT_SIGN = -1.0f;
