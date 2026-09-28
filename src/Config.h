#pragma once
#include <Arduino.h>
#include "art/LedProfile.h"

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
constexpr uint8_t MAX_BRIGHTNESS = LedProfile::BRIGHTNESS; // 18/255; art/led-profile.json
constexpr uint16_t MAX_MILLIAMPS = 400;  // matriz inteira, a 5V
constexpr uint16_t FRAME_MS = 20;        // ~50 fps
// Dormindo: cada LED aceso vai pra esse degrau (o mais baixo que ainda dá
// pra ver). Em 1, só o canal mais forte de cada cor sobrevive ao
// arredondamento — qualquer marrom/laranja vira vermelho puro, sem jeito de
// escolher uma cor que escape disso. Em 2, um canal secundário com pelo
// menos ~1/4 do canal principal ainda aparece, o suficiente pra reconhecer
// o tom (ainda bem mais escuro que acordado).
constexpr uint8_t NIGHT_LEVEL = 2;

// ============================================================ BOTÃO
constexpr uint16_t BUTTON_DEBOUNCE_MS = 25;
constexpr uint16_t BUTTON_LONG_MS = 600;       // segurar e soltar = menu/confirmar
constexpr uint16_t BUTTON_RESET_SHOW_MS = 3000; // a partir daqui mostra a barra de reset
constexpr uint16_t BUTTON_RESET_MS = 8000;      // segurou até aqui = recomeçar do zero

// ============================================================ IMU / GESTOS
// Eixos/sinais do sensor ficam em src/ImuCalib.h (tools/calibrate_imu.py).
constexpr float GRAVITY = 9.80665f;
constexpr float MOTION_THRESHOLD = 1.2f;   // m/s^2 de variação = "está mexendo"
constexpr float SHAKE_THRESHOLD = 7.0f;    // m/s^2 de variação brusca = chacoalhada
constexpr uint16_t SHAKE_COOLDOWN_MS = 350;
constexpr float FACE_DOWN_Z = -7.0f;       // z abaixo disso = virado de cara pra baixo
constexpr float FACE_UP_Z = -3.0f;         // histerese: não acordar por ruído perto do limiar
constexpr uint16_t FACE_DOWN_HOLD_MS = 1500;
constexpr float TILT_STEP = 0.45f;         // inclinação pra trocar de bicho na seleção
constexpr float TILT_REARM = 0.20f;

// ============================================================ TEMPO DE JOGO
// Multiplicador do relógio do bichinho. 1 = tempo real. Pra testar as
// regras rápido, suba (ex: 60 = um minuto de jogo por segundo).
constexpr uint16_t TIME_SCALE = 1;

// Ovo: minutos de *movimento* acumulado pra chocar. Ficar parado só pausa
// a contagem (sem perder nada); mexer de novo continua de onde parou.
constexpr uint32_t INCUBATION_MS = 5UL * 60UL * 1000UL;
constexpr uint32_t INCUBATION_IDLE_GRACE_MS = 15UL * 1000UL;

// ============================================================ REGRAS DO BICHINHO
// Tudo em minutos de jogo. Stats vão de 0 a 100.
constexpr uint8_t HUNGER_EVERY_MIN = 4;     // -1 de saciedade
constexpr uint8_t HAPPY_EVERY_MIN = 5;      // -1 de alegria
constexpr uint8_t ENERGY_EVERY_MIN = 6;     // -1 de energia acordado
constexpr uint8_t SLEEP_GAIN_EVERY_MIN = 1; // +1 de energia dormindo (0→100 em ~1h40)
// Cansado (energia < NEED_LOW) e ninguém brinca com ele (sem clique nem gesto)
// por esse tempo: pega no sono sozinho. Só segurar a placa não o mantém acordado.
constexpr uint32_t AUTO_SLEEP_IDLE_MS = 30000;
// Em energia boa, depois de 2 min sem BOOT/gesto/movimento, o mundo de Conway
// aparece ao redor do pet por 7 s a cada 45 s (glider ou blinker, só nos
// pixels livres). Ao dormir, sonha após 8 s de pet visível.
constexpr uint32_t IDLE_DREAM_AFTER_MS = 2UL * 60UL * 1000UL;
constexpr uint32_t IDLE_DREAM_CYCLE_MS = 45UL * 1000UL;
constexpr uint32_t IDLE_DREAM_SHOW_MS = 7UL * 1000UL;
constexpr uint32_t SLEEP_DREAM_AFTER_MS = 8UL * 1000UL;
constexpr uint32_t SLEEP_PET_REVEAL_MS = 8UL * 1000UL;
constexpr uint16_t DREAM_STEP_MS = 500; // uma geração a cada meio segundo
// Entrada no sonho: olhos fechados, bolha acima da cabeça, bolha vira o mundo.
constexpr uint16_t DREAM_BUBBLE_MS = 2400;
// Capítulos do sono: duração sorteada (DNA + contador) entre esses limites.
// Vazio passa na hora; imóvel por DREAM_STILL_STEPS gerações também; um
// oscilador que sobrou de outro capítulo pulsa por até DREAM_OSC_MAX_MS.
constexpr uint32_t DREAM_CHAPTER_MIN_MS = 20000;
constexpr uint32_t DREAM_CHAPTER_MAX_MS = 40000;
constexpr uint8_t DREAM_STILL_STEPS = 6;
constexpr uint32_t DREAM_OSC_MAX_MS = 10000;
constexpr uint16_t DREAM_FADE_MS = 1000; // troca de capítulo por substituição de pixels
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
constexpr uint8_t PET_GAIN = 5;             // carinho no menu
constexpr uint16_t PLAY_COOLDOWN_MS = 5000; // sacudidas repetidas não gastam energia em sequência
constexpr uint16_t TEXT_STEP_MS = 150;      // texto rolando: ms por coluna (maior = mais devagar)
constexpr uint8_t NEED_LOW = 25;            // abaixo disso o bicho reclama
constexpr uint16_t NEED_BUBBLE_EVERY_MS = 4000; // a cada 4 s mostra o ícone do que precisa...
constexpr uint16_t NEED_BUBBLE_MS = 900;        // ...por 0,9 s

constexpr uint32_t SAVE_EVERY_MS = 5UL * 60UL * 1000UL;
constexpr uint16_t MENU_TIMEOUT_MS = 8000;
