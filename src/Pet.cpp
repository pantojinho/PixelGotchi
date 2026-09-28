#include "Pet.h"
#include "Storage.h"
#include <esp_random.h>

void Pet::clampAdd(uint8_t &stat, int delta) {
    int v = (int)stat + delta;
    if (v < 0) v = 0;
    if (v > 100) v = 100;
    stat = (uint8_t)v;
}

void Pet::begin() {
    Storage::load(_state);
    unsigned long now = millis();
    _lastMotionAt = now;
    _lastIncubationCheck = now;
    _lastStatTick = now;
    _lastSave = now;
}

void Pet::update() {
    unsigned long now = millis();
    if (!_state.hatched) {
        tickIncubation(now);
    } else if (now - _lastStatTick >= STAT_TICK_MS) {
        tickStats();
        _lastStatTick = now;
    }
    saveIfDue(false);
}

void Pet::tickStats() {
    clampAdd(_state.hunger, HUNGER_DECAY_PER_TICK);
    clampAdd(_state.happiness, HAPPINESS_DECAY_PER_TICK);
    if (_state.asleep) {
        clampAdd(_state.energy, ENERGY_GAIN_PER_TICK_ASLEEP);
    } else {
        clampAdd(_state.energy, ENERGY_DECAY_PER_TICK_AWAKE);
    }
}

void Pet::tickIncubation(unsigned long now) {
    // Só conta o tempo em que houve movimento recente — ficar parado
    // pausa a incubação em vez de zerar (ver Config.h).
    if (now - _lastMotionAt < MOTION_IDLE_TIMEOUT_MS) {
        unsigned long dt = now - _lastIncubationCheck;
        _state.incubationMotionMs += dt;
        if (_state.incubationMotionMs >= INCUBATION_REQUIRED_MOTION_MS) {
            hatch();
        }
    }
    _lastIncubationCheck = now;
}

void Pet::hatch() {
    _state.hatched = true;
    _state.species = esp_random() % SPECIES_COUNT;
    _state.hunger = 80;
    _state.happiness = 90;
    _state.energy = 80;
    saveIfDue(true);
}

void Pet::triggerActionOverride(Expression e, unsigned long durationMs) {
    _actionOverrideExpr = e;
    _actionOverrideUntil = millis() + durationMs;
}

void Pet::onShake() {
    unsigned long now = millis();
    _lastMotionAt = now;
    if (!_state.hatched) return;

    clampAdd(_state.happiness, PLAY_HAPPINESS_GAIN);
    clampAdd(_state.energy, -PLAY_ENERGY_COST);
    triggerActionOverride(Expression::HAPPY, 2000);
    saveIfDue(false);
}

void Pet::onFlipHold() {
    if (_state.hatched) {
        _state.asleep = true;
        saveIfDue(true);
    }
}

void Pet::onFlipRelease() {
    if (_state.hatched && _state.asleep) {
        _state.asleep = false;
        saveIfDue(true);
    }
}

void Pet::onButtonShortPress() {
    unsigned long now = millis();
    _lastMotionAt = now; // conta como interação durante a incubação também
    if (!_state.hatched) return;

    clampAdd(_state.hunger, FEED_HUNGER_GAIN);
    triggerActionOverride(Expression::HAPPY, 1200);
    saveIfDue(true);
}

void Pet::onButtonLongPress() {
    if (_state.hatched) {
        _state.asleep = !_state.asleep;
        saveIfDue(true);
    }
}

void Pet::saveIfDue(bool force) {
    unsigned long now = millis();
    if (force || now - _lastSave >= SAVE_MIN_INTERVAL_MS) {
        Storage::save(_state);
        _lastSave = now;
    }
}

uint8_t Pet::eggStage() const {
    if (_state.incubationMotionMs >= INCUBATION_REQUIRED_MOTION_MS) return 3;
    float progress = (float)_state.incubationMotionMs / (float)INCUBATION_REQUIRED_MOTION_MS;
    int stage = (int)(progress * 4.0f);
    if (stage > 3) stage = 3;
    if (stage < 0) stage = 0;
    return (uint8_t)stage;
}

unsigned long Pet::incubationRemainingMs() const {
    if (_state.incubationMotionMs >= INCUBATION_REQUIRED_MOTION_MS) return 0;
    return INCUBATION_REQUIRED_MOTION_MS - _state.incubationMotionMs;
}

Expression Pet::currentExpression() {
    unsigned long now = millis();
    if (now < _actionOverrideUntil) return _actionOverrideExpr;

    if (_state.asleep || _state.energy <= ENERGY_SLEEPY_THRESHOLD) {
        return Expression::IDLE_BLINK; // "olhos fechados" também serve de cara de sono
    }
    if (_state.hunger <= STAT_LOW_THRESHOLD) return Expression::HUNGRY;
    if (_state.happiness <= STAT_LOW_THRESHOLD) return Expression::SAD;

    // pisca rapidamente a cada ~3s pra não ficar com cara de estátua
    if ((now % 3000) < 150) return Expression::IDLE_BLINK;
    return Expression::IDLE_OPEN;
}
