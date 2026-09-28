#pragma once
#include <Arduino.h>
#include "Config.h"

// Cada sprite é uma grade 8x8 de índices de paleta:
//   0 = apagado
//   1 = cor do corpo (definida por espécie, ver Species em Pet.h)
//   2 = detalhe escuro (olhos/boca)
//   3 = destaque claro (brilho/bochecha)
//   4 = rachadura (só usado nos sprites do ovo)
using Sprite8x8 = uint8_t[MATRIX_H][MATRIX_W];

// ---------------- Ovo ----------------
// Progride de "liso" (EGG_0) até "quase abrindo" (EGG_3) conforme o
// tempo de incubação avança. Ajuste os pixels livremente depois de ver
// o resultado real na matriz.
static const Sprite8x8 EGG_STAGE_0 = {
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 3, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
};

static const Sprite8x8 EGG_STAGE_1 = {
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 3, 4, 1, 1, 1},
    {1, 1, 1, 1, 4, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
};

static const Sprite8x8 EGG_STAGE_2 = {
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 4, 1, 1, 1},
    {1, 1, 1, 4, 1, 1, 1, 1},
    {1, 1, 1, 1, 4, 1, 1, 1},
    {1, 1, 1, 4, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
};

static const Sprite8x8 EGG_STAGE_3 = {
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 0, 1, 1, 0},
    {1, 1, 1, 4, 0, 4, 1, 1},
    {1, 1, 4, 0, 0, 0, 4, 1},
    {1, 1, 1, 4, 0, 4, 1, 1},
    {1, 1, 1, 1, 4, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
};

// ---------------- Criatura ----------------
// Uma única forma base, recolorida por espécie (ver SPECIES em Pet.h).
// A linha 0 fica livre de propósito: é onde as orelhas (quando a
// espécie tem) são desenhadas por cima, no Display.cpp.

static const Sprite8x8 CREATURE_IDLE_OPEN = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 3, 1, 1, 1, 1, 3, 1},
    {1, 1, 2, 2, 2, 2, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
};

static const Sprite8x8 CREATURE_IDLE_BLINK = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 3, 1, 1, 1, 1, 3, 1},
    {1, 1, 2, 2, 2, 2, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
};

static const Sprite8x8 CREATURE_HAPPY = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 3, 2, 1, 1, 2, 3, 1},
    {1, 1, 2, 2, 2, 2, 1, 1},
    {0, 1, 1, 2, 2, 1, 1, 0},
};

static const Sprite8x8 CREATURE_HUNGRY = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 2, 2, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
};

static const Sprite8x8 CREATURE_SAD = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 2, 2, 1, 1, 0},
};
