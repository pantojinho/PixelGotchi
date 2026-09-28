#include "PetSim.h"
#include "Config.h"
#include "Storage.h"
#include <esp_random.h>

namespace {
constexpr uint8_t STATE_VERSION = 3;

void add(uint8_t &v, int d) {
    int n = (int)v + d;
    v = n < 0 ? 0 : (n > 100 ? 100 : n);
}

// "-1 a cada N minutos" sem ponto flutuante: conta minutos até N.
bool every(uint8_t &acc, uint8_t n) {
    if (++acc < n) return false;
    acc = 0;
    return true;
}

uint16_t randomPoopDelay() {
    return POOP_MIN_MIN + esp_random() % (POOP_MAX_MIN - POOP_MIN_MIN + 1);
}
} // namespace

void PetSim::defaults() {
    st = {};
    st.version = STATE_VERSION;
    st.phase = Phase::Select;
}

void PetSim::begin() {
    if (!Storage::load(&st, sizeof(st)) || st.version != STATE_VERSION) defaults();
    lastUpdate = lastSave = millis();
}

void PetSim::update() {
    uint32_t now = millis();
    uint32_t dt = now - lastUpdate;
    lastUpdate = now;

    if (st.phase == Phase::Alive) {
        gameMsAcc += dt * TIME_SCALE;
        while (gameMsAcc >= 60000UL && st.phase == Phase::Alive) {
            gameMsAcc -= 60000UL;
            tickMinute();
        }
    }
    save(false);
}

void PetSim::tickMinute() {
    dirty = true;
    st.ageMin++;
    Dna d = dna();
    uint8_t hungerEvery = scaledEvery(HUNGER_EVERY_MIN, d.metabolism());
    uint8_t happyEvery = scaledEvery(HAPPY_EVERY_MIN, d.neediness());

    if (st.asleep) {
        if (every(st.energyAcc, SLEEP_GAIN_EVERY_MIN)) add(st.energy, +1);
        if (every(st.hungerAcc, hungerEvery * 2)) add(st.hunger, -1); // metabolismo mais lento dormindo
        if (st.energy >= 100) {
            st.asleep = false;
            notices |= N_WOKE;
        }
    } else {
        if (every(st.hungerAcc, hungerEvery)) add(st.hunger, -1);
        if (every(st.happyAcc, happyEvery)) add(st.happy, -1);
        if (every(st.energyAcc, ENERGY_EVERY_MIN)) add(st.energy, -1);
        if (st.energy == 0) {
            st.asleep = true; // desmaiou de cansaço
            notices |= N_FELL_ASLEEP;
        }
    }

    if (st.wild) {
        // Selvagem se vira: caça quando aperta a fome, faz cocô longe,
        // não adoece. Mas a vida no mato cobra o preço (ver WILD_DEATH_DAYS).
        if (!st.asleep && st.hunger < 30) {
            add(st.hunger, 40);
            notices |= N_FORAGED;
        }
        st.poop = 0;
        st.sick = false;
        if (++st.wildMin >= (uint32_t)WILD_DEATH_DAYS * 1440UL) {
            st.phase = Phase::Dead;
            st.asleep = false;
            notices |= N_DIED;
            save(true);
        }
        return;
    }

    if (!st.asleep) {
        if (st.poopInMin > 0) {
            st.poopInMin--;
        } else {
            if (st.poop < POOP_MAX) {
                st.poop++;
                notices |= N_POOPED;
            }
            st.poopInMin = randomPoopDelay();
        }
    }

    st.dirtyMin = st.poop ? st.dirtyMin + 1 : 0;
    st.starveMin = st.hunger == 0 ? st.starveMin + 1 : 0;
    if (!st.sick && (st.dirtyMin >= DIRTY_SICK_MIN || st.starveMin >= STARVE_SICK_MIN)) {
        st.sick = true;
        notices |= N_SICK;
    }

    uint8_t problems = (st.hunger == 0) + (st.happy == 0) + st.sick + (st.poop >= 2);
    if (problems) {
        st.neglect += problems;
    } else if (st.neglect > 0) {
        st.neglect--;
    }
    if (st.neglect >= NEGLECT_WILD) {
        st.wild = true;
        st.sick = false;
        st.poop = 0;
        notices |= N_WENT_WILD;
        save(true);
    }
}

void PetSim::choose(uint8_t species, uint32_t seed) {
    defaults();
    st.species = species;
    st.dna = seed;
    st.phase = Phase::Egg;
    st.hunger = 80;
    st.happy = 80;
    st.energy = 90;
    st.poopInMin = randomPoopDelay();
    save(true);
}

void PetSim::addIncubation(uint32_t realMs) {
    if (st.phase != Phase::Egg) return;
    st.incubationMs += realMs * TIME_SCALE;
    dirty = true;
}

float PetSim::incubationProgress() const {
    float p = (float)st.incubationMs / INCUBATION_MS;
    return p > 1 ? 1 : p;
}

void PetSim::hatch() {
    st.phase = Phase::Alive;
    gameMsAcc = 0;
    save(true);
}

void PetSim::restart() {
    defaults();
    save(true);
}

Result PetSim::feed() {
    if (st.phase != Phase::Alive || st.asleep || st.hunger >= 95) return Result::Refused;
    if (st.wild && (esp_random() & 1)) return Result::Refused; // arisco
    add(st.hunger, FEED_GAIN);
    st.starveMin = 0;
    if (st.poopInMin > 40) st.poopInMin -= 20; // comeu, vai fazer cocô antes
    save(true);
    return Result::Ok;
}

Result PetSim::play() {
    if (st.phase != Phase::Alive || st.asleep || st.wild || st.energy < PLAY_ENERGY_COST + 2)
        return Result::Refused;
    add(st.happy, PLAY_GAIN);
    add(st.energy, -PLAY_ENERGY_COST);
    add(st.hunger, -3);
    save(true);
    return Result::Ok;
}

Result PetSim::clean() {
    if (st.phase != Phase::Alive || st.poop == 0) return Result::Refused;
    st.poop = 0;
    st.dirtyMin = 0;
    save(true);
    return Result::Ok;
}

Result PetSim::medicine() {
    if (st.phase != Phase::Alive || !st.sick) return Result::Refused;
    st.sick = false;
    st.dirtyMin = st.starveMin = 0; // uma folga antes de poder adoecer de novo
    save(true);
    return Result::Ok;
}

Result PetSim::lightsOff() {
    if (st.phase != Phase::Alive || st.asleep || st.energy >= 90) return Result::Refused;
    st.asleep = true;
    save(true);
    return Result::Ok;
}

Result PetSim::wakeUp() {
    if (st.phase != Phase::Alive || !st.asleep) return Result::Refused;
    st.asleep = false;
    if (st.energy < 50) add(st.happy, -5); // acordou mal-humorado
    save(true);
    return Result::Ok;
}

Result PetSim::pet() {
    if (st.phase != Phase::Alive || st.asleep || st.wild) return Result::Refused;
    add(st.happy, PET_GAIN);
    dirty = true;
    return Result::Ok;
}

uint8_t PetSim::neglectStage() const {
    if (st.wild) return 3;
    if (st.neglect >= NEGLECT_STAGE2) return 2;
    if (st.neglect >= NEGLECT_STAGE1) return 1;
    return 0;
}

uint8_t PetSim::care() const {
    if (st.wild) return 0;
    uint32_t lost = (uint32_t)st.neglect * 100 / NEGLECT_WILD;
    return lost >= 100 ? 0 : 100 - lost;
}

bool PetSim::needsAttention() const {
    if (st.phase != Phase::Alive || st.wild) return false;
    return st.hunger < NEED_LOW || st.happy < NEED_LOW || st.sick || st.poop > 0 ||
           (!st.asleep && st.energy < 15);
}

uint8_t PetSim::takeNotices() {
    uint8_t n = notices;
    notices = 0;
    return n;
}

void PetSim::save(bool force) {
    uint32_t now = millis();
    if (!force && (!dirty || now - lastSave < SAVE_EVERY_MS)) return;
    Storage::save(&st, sizeof(st));
    lastSave = now;
    dirty = false;
}
