#pragma once
#include "Pet.h"

namespace Storage {
void load(PetState &state);
void save(const PetState &state);
} // namespace Storage
