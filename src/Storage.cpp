#include "Storage.h"
#include <Preferences.h>

namespace {
Preferences prefs;
const char *NS = "pixelgochi";
} // namespace

namespace Storage {

void load(PetState &state) {
    prefs.begin(NS, /*readOnly=*/true);
    state.hatched = prefs.getBool("hatched", false);
    state.species = prefs.getUChar("species", 0);
    state.hunger = prefs.getUChar("hunger", 80);
    state.happiness = prefs.getUChar("happy", 80);
    state.energy = prefs.getUChar("energy", 80);
    state.asleep = prefs.getBool("asleep", false);
    state.incubationMotionMs = prefs.getULong("incMs", 0);
    prefs.end();
}

void save(const PetState &state) {
    prefs.begin(NS, /*readOnly=*/false);
    prefs.putBool("hatched", state.hatched);
    prefs.putUChar("species", state.species);
    prefs.putUChar("hunger", state.hunger);
    prefs.putUChar("happy", state.happiness);
    prefs.putUChar("energy", state.energy);
    prefs.putBool("asleep", state.asleep);
    prefs.putULong("incMs", state.incubationMotionMs);
    prefs.end();
}

} // namespace Storage
