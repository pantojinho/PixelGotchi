#pragma once
#include <Arduino.h>

// ============================================================ HARDWARE
// Waveshare ESP32-S3-Matrix. Pinos internos confirmados no projeto
// github.com/shantanugoel/pomodoro_cube (não constam no pinout público).
constexpr uint8_t PIN_MATRIX_DATA = 14;
constexpr uint8_t PIN_IMU_SDA = 11;
constexpr uint8_t PIN_IMU_SCL = 12;
constexpr uint8_t PIN_BOOT_BUTTON = 0;

constexpr uint8_t MATRIX_W = 8;
constexpr uint8_t MATRIX_H = 8;

// Orientação da imagem na matriz física. A fiação dos LEDs é progressiva
// (índice = linha*8 + coluna). Se o desenho aparecer girado/espelhado,
// ajuste aqui (0..3 = 0/90/180/270 graus horário).
constexpr uint8_t DISPLAY_ROTATION = 0;
constexpr bool DISPLAY_MIRROR_X = false;

// ============================================================ SEGURANÇA DOS LEDs
// A Waveshare avisa que brilho alto esquenta e pode danificar a placa
// (já aconteceu neste projeto com um driver ruim). Três camadas:
// teto de brilho, orçamento de corrente do FastLED, e taxa de quadros.
constexpr uint8_t MAX_BRIGHTNESS = 30;   // 0-255
constexpr uint16_t MAX_MILLIAMPS = 400;  // matriz inteira, a 5V
constexpr uint16_t FRAME_MS = 20;        // ~50 fps
constexpr uint8_t NIGHT_DIM = 110;       // brilho relativo (0-255) com a luz apagada

// ============================================================ BOTÃO
constexpr uint16_t BUTTON_DEBOUNCE_MS = 25;
constexpr uint16_t BUTTON_LONG_MS = 600;       // clique longo = confirmar
constexpr uint16_t BUTTON_RESET_SHOW_MS = 3000; // a partir daqui mostra a barra de reset
constexpr uint16_t BUTTON_RESET_MS = 8000;      // segurou até aqui = recomeçar do zero

// ============================================================ IMU / GESTOS
// Mapeamento de eixos vindo do pomodoro_cube: com a imagem "em pé",
// x > 0 aponta a gravidade pra linha 7 (baixo) e y < 0 pra coluna 7
// (direita). Se inclinar pra direita mover o bicho pra esquerda,
// troque o sinal aqui.
constexpr float TILT_SIGN = -1.0f;
constexpr float GRAVITY = 9.80665f;
constexpr float MOTION_THRESHOLD = 1.2f;   // m/s^2 de variação = "está mexendo"
constexpr float SHAKE_THRESHOLD = 7.0f;    // m/s^2 de variação brusca = chacoalhada
constexpr uint16_t SHAKE_COOLDOWN_MS = 350;
constexpr float FACE_DOWN_Z = -7.0f;       // z abaixo disso = virado de cara pra baixo
constexpr uint16_t FACE_DOWN_HOLD_MS = 1500;
constexpr float TILT_STEP = 0.45f;         // inclinação pra trocar de bicho na seleção
constexpr float TILT_REARM = 0.20f;

// ============================================================ TEMPO DE JOGO
// Multiplicador do relógio do bichinho. 1 = tempo real. Pra testar as
// regras rápido, suba (ex: 60 = um minuto de jogo por segundo).
constexpr uint16_t TIME_SCALE = 1;

// Ovo: minutos de *movimento* acumulado pra chocar (ficar parado pausa).
constexpr uint32_t INCUBATION_MS = 10UL * 60UL * 1000UL;
constexpr uint32_t INCUBATION_IDLE_GRACE_MS = 15UL * 1000UL;

// ============================================================ REGRAS DO BICHINHO
// Tudo em minutos de jogo. Stats vão de 0 a 100.
constexpr uint8_t HUNGER_EVERY_MIN = 4;     // -1 de saciedade
constexpr uint8_t HAPPY_EVERY_MIN = 5;      // -1 de alegria
constexpr uint8_t ENERGY_EVERY_MIN = 6;     // -1 de energia acordado
constexpr uint8_t SLEEP_GAIN_EVERY_MIN = 2; // +1 de energia dormindo
constexpr uint16_t POOP_MIN_MIN = 90;       // intervalo entre cocôs (aleatório)
constexpr uint16_t POOP_MAX_MIN = 150;
constexpr uint8_t POOP_MAX = 3;
constexpr uint16_t DIRTY_SICK_MIN = 90;     // cocô sem limpar por esse tempo = doença
constexpr uint16_t STARVE_SICK_MIN = 120;   // fome zerada por esse tempo = doença

// Descuido acumula em estágios. Cada minuto com problema ativo (fome 0,
// alegria 0, doente, 2+ cocôs) soma 1 ponto por problema; minuto sem
// problema desconta 1. Estágios: normal -> descuidado -> quase selvagem ->
// SELVAGEM (arisco, se vira sozinho). Selvagem por WILD_DEATH_DAYS = morre.
// Bem cuidado, vive pra sempre.
constexpr uint16_t NEGLECT_STAGE1 = 240;
constexpr uint16_t NEGLECT_STAGE2 = 480;
constexpr uint16_t NEGLECT_WILD = 720;      // ~12h com um problema ativo
constexpr uint8_t WILD_DEATH_DAYS = 5;

constexpr uint8_t FEED_GAIN = 30;
constexpr uint8_t PLAY_GAIN = 20;
constexpr uint8_t PLAY_ENERGY_COST = 8;
constexpr uint8_t PET_GAIN = 5;             // carinho (chacoalhar de leve)
constexpr uint16_t PET_COOLDOWN_MS = 20000;
constexpr uint8_t NEED_LOW = 25;            // abaixo disso o bicho reclama

constexpr uint32_t SAVE_EVERY_MS = 5UL * 60UL * 1000UL;
constexpr uint16_t MENU_TIMEOUT_MS = 8000;
constexpr uint16_t STATUS_TIMEOUT_MS = 6000;
