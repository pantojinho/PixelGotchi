#pragma once
#include <Arduino.h>
#include "Config.h"

enum class Expression : uint8_t {
    IDLE_OPEN,
    IDLE_BLINK,
    HAPPY,
    HUNGRY,
    SAD,
};

struct Species {
    const char *name;
    uint8_t r, g, b; // cor do corpo
    bool hasEars;
};

// Adicione novas espécies aqui — sorteada uma vez, na primeira eclosão.
static const Species SPECIES_TABLE[] = {
    {"Blobby", 40, 200, 90, false},
    {"Catgotchi", 235, 130, 30, true},
};
constexpr uint8_t SPECIES_COUNT = sizeof(SPECIES_TABLE) / sizeof(SPECIES_TABLE[0]);

// Estado completo do bichinho. Pet.cpp é a única coisa que muda isto;
// Display.cpp só lê para desenhar.
struct PetState {
    bool hatched = false;
    uint8_t species = 0;

    // 0-100
    uint8_t hunger = 80;
    uint8_t happiness = 80;
    uint8_t energy = 80;

    bool asleep = false;

    // Incubação: soma apenas o tempo em que houve movimento (ver Config.h)
    unsigned long incubationMotionMs = 0;
};

class Pet {
public:
    void begin();  // carrega do NVS (ou começa ovo novo)
    void update(); // chamar todo loop() — cuida de ticks de stats e auto-save

    // Eventos de entrada, chamados pelos módulos Input/Imu
    void onShake();
    void onFlipHold();
    void onFlipRelease();
    void onButtonShortPress();
    void onButtonLongPress();

    // Para a UI de incubação
    uint8_t eggStage() const; // 0-3
    unsigned long incubationRemainingMs() const;

    const PetState &state() const { return _state; }
    Expression currentExpression();
    const Species &currentSpecies() const { return SPECIES_TABLE[_state.species]; }

private:
    PetState _state;
    unsigned long _lastStatTick = 0;
    unsigned long _lastSave = 0;
    unsigned long _lastMotionAt = 0;
    unsigned long _lastIncubationCheck = 0;

    unsigned long _actionOverrideUntil = 0;
    Expression _actionOverrideExpr = Expression::IDLE_OPEN;

    void tickStats();
    void tickIncubation(unsigned long now);
    void hatch();
    void saveIfDue(bool force = false);
    void triggerActionOverride(Expression e, unsigned long durationMs);
    static void clampAdd(uint8_t &stat, int delta);
};
