#pragma once
#include <stdint.h>

enum class Ev : uint8_t {
    None,
    Short,     // clique curto
    Long,      // soltou depois de BUTTON_LONG_MS, antes do reset
    Reset,     // segurou BUTTON_RESET_MS
    Shake,     // chacoalhada
    FaceDown,  // virado de cara pra baixo por FACE_DOWN_HOLD_MS
    FaceUp,    // desvirou
};

namespace Events {
void push(Ev e);
Ev pop(); // Ev::None se vazio
void clear();
} // namespace Events
