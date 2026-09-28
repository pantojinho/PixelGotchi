#pragma once
#include <stdint.h>
#include "Dna.h"

enum class Phase : uint8_t { Select, Egg, Alive, Dead };

// Tudo que precisa sobreviver a reset/queda de energia. Mudou o layout?
// suba STATE_VERSION em PetSim.cpp (estado antigo é descartado).
struct PetState {
    uint8_t version;
    Phase phase;
    uint8_t species;
    uint32_t dna;
    uint8_t hunger, happy, energy; // 0-100 (hunger 100 = satisfeito)
    uint8_t poop;
    bool sick, asleep, wild;
    uint16_t neglect;
    uint32_t ageMin, wildMin;
    uint32_t incubationMs;
    uint16_t poopInMin, dirtyMin, starveMin;
    // Acumuladores pras taxas "-1 a cada N minutos".
    uint8_t hungerAcc, happyAcc, energyAcc;
};

// Avisos pra camada de jogo reagir com animação.
enum Notice : uint8_t {
    N_POOPED = 1 << 0,
    N_SICK = 1 << 1,
    N_FELL_ASLEEP = 1 << 2,
    N_WOKE = 1 << 3,
    N_WENT_WILD = 1 << 4,
    N_FORAGED = 1 << 5,
    N_DIED = 1 << 6,
};

enum class Result : uint8_t { Ok, Refused };

class PetSim {
public:
    void begin();
    void update(); // chamar sempre: avança o relógio de jogo e salva quando precisa

    // Fases
    void choose(uint8_t species, uint32_t seed); // Select -> Egg
    void addIncubation(uint32_t realMs);
    float incubationProgress() const;            // 0..1
    void hatch();                                // Egg -> Alive
    void restart();                              // qualquer -> Select

    // Cuidados
    Result feed();
    Result play();
    Result clean();
    Result medicine();
    Result lightsOff();
    Result wakeUp();
    Result pet(); // carinho

    uint8_t neglectStage() const; // 0 normal, 1 descuidado, 2 quase selvagem, 3 selvagem
    uint8_t care() const;         // 0-100, o "saúde" do status (inverso do descuido)
    uint16_t ageDays() const { return st.ageMin / 1440; }
    bool needsAttention() const;
    Dna dna() const { return Dna{st.dna}; }
    uint8_t takeNotices();
    const PetState &s() const { return st; }

private:
    PetState st;
    uint32_t lastUpdate = 0;
    uint32_t gameMsAcc = 0;
    uint32_t lastSave = 0;
    bool dirty = false;
    uint8_t notices = 0;

    void defaults();
    void tickMinute();
    void save(bool force);
};
