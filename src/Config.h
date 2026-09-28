#pragma once
#include <Arduino.h>

// ---- Hardware pins (Waveshare ESP32-S3-Matrix) ----
// Confirmados via projeto de terceiros feito para esta placa exata
// (github.com/shantanugoel/pomodoro_cube), já que não constam no
// pinout público da Waveshare.
constexpr uint8_t PIN_MATRIX_DATA = 14;
constexpr uint8_t PIN_IMU_SDA = 11;
constexpr uint8_t PIN_IMU_SCL = 12;
constexpr uint8_t PIN_IMU_INT = 13; // não usado no v1 (polling), reservado
constexpr uint8_t PIN_BOOT_BUTTON = 0;

constexpr uint8_t MATRIX_W = 8;
constexpr uint8_t MATRIX_H = 8;

// A Waveshare avisa: brilho alto demais esquenta a placa rápido e pode
// danificá-la. Este teto vale para os 3 canais RGB de cada LED.
constexpr uint8_t MAX_BRIGHTNESS = 40; // 0-255

// ---- Botão BOOT ----
constexpr unsigned long BUTTON_DEBOUNCE_MS = 30;
constexpr unsigned long BUTTON_LONG_PRESS_MS = 1200;

// ---- Gestos (IMU) ----
// Limiares empíricos de partida — ajuste vendo os logs seriais (DEBUG_IMU)
// se o chacoalhar ou o virar não estiverem respondendo bem na sua unidade.
constexpr float GRAVITY_MPS2 = 9.80665f;
constexpr float SHAKE_THRESHOLD_MPS2 = 6.0f;   // desvio da gravidade p/ contar shake
constexpr unsigned long SHAKE_COOLDOWN_MS = 350;
constexpr float FLIP_Z_THRESHOLD_MPS2 = -7.0f; // accel.z abaixo disso = de cabeça p/ baixo
constexpr unsigned long FLIP_HOLD_MS = 1000;   // precisa segurar a posição

// ---- Ovo / incubação ----
// Tempo total exigido é "tempo com movimento", não tempo corrido: se o ovo
// ficar parado, o contador só pausa (não zera). Assim é preciso realmente
// mexer nele de vez em quando durante a incubação, sem punir demais.
constexpr unsigned long INCUBATION_REQUIRED_MOTION_MS = 10UL * 60UL * 1000UL; // 10 min
constexpr unsigned long MOTION_IDLE_TIMEOUT_MS = 15UL * 1000UL; // pausa após 15s parado

// ---- Estatísticas do bichinho ----
constexpr unsigned long STAT_TICK_MS = 60UL * 1000UL; // 1 tick de stats por minuto
constexpr int8_t HUNGER_DECAY_PER_TICK = -1;
constexpr int8_t HAPPINESS_DECAY_PER_TICK = -1;
constexpr int8_t ENERGY_DECAY_PER_TICK_AWAKE = -1;
constexpr int8_t ENERGY_GAIN_PER_TICK_ASLEEP = 3;

constexpr int8_t FEED_HUNGER_GAIN = 25;
constexpr int8_t PLAY_HAPPINESS_GAIN = 20;
constexpr int8_t PLAY_ENERGY_COST = 5;

constexpr uint8_t STAT_LOW_THRESHOLD = 30;
constexpr uint8_t ENERGY_SLEEPY_THRESHOLD = 20;

// ---- Persistência ----
constexpr unsigned long SAVE_MIN_INTERVAL_MS = 20UL * 1000UL; // evita gravar NVS demais

// ---- Debug ----
// Descomente para imprimir leituras cruas do IMU no serial — útil pra
// calibrar os limiares de shake/flip na sua unidade física.
// #define DEBUG_IMU
