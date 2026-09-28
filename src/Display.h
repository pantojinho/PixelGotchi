#pragma once
#include "Canvas.h"

namespace Display {

void begin();
// night = luz apagada (bicho dormindo): todo LED aceso no brilho mínimo.
void show(const Canvas &c, bool night = false);

} // namespace Display
