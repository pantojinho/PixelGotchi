// Executa o código REAL com GPIO, relógio, NVS e sensor simulados.
// Game/Imu inclusos aqui permitem observar cenas sem API de debug no firmware.
#include "../src/Game.cpp"
#include "../src/Input.cpp"
#include "../src/Imu.cpp"
#include "../src/Storage.h"
#include <assert.h>

uint32_t testMs = 10000;
bool testButton = HIGH;
uint8_t stored[sizeof(PetState)]{};
size_t storedSize = 0;
namespace Storage {
void begin() {}
bool load(void *dst, size_t n) {
    if (storedSize != n) return false;
    memcpy(dst, stored, n); return true;
}
void save(const void *src, size_t n) {
    assert(n <= sizeof(stored)); memcpy(stored, src, n); storedSize = n;
}
}
namespace Display { void begin() {} void show(const Canvas &) {} }

void advance(uint32_t ms) { testMs += ms; Input::update(); }
void down() { testButton = LOW; advance(1); advance(25); }
void up() { testButton = HIGH; advance(1); advance(25); }
void alive(uint8_t species = 0) {
    Events::clear();
    sim.choose(species, 0x12345678); sim.hatch();
    act = Act::None; gestureSleep = false; playedOnce = false;
    petX = centerCx(idleW()); beh = Beh::Stand; behUntil = testMs + 5000;
    tiltValue = 0; tiltArmed = true; go(Scene::Life);
}
void dispatch(Ev e) { Events::push(e); Game::update(); }
void sample(float x, float y, float z, uint32_t ms = 20) {
    testMs += ms; testAccel = {{x, y, z}}; testSampleReady = true; Imu::update();
}

int main() {
    Input::begin(); Imu::begin();
    // Contato instável nunca vira clique.
    testButton = LOW; advance(1); advance(10);
    testButton = HIGH; advance(1); advance(30);
    assert(Events::pop() == Ev::None);
    down(); advance(100); up();
    assert(Events::pop() == Ev::Short && Events::pop() == Ev::None);
    down(); advance(600);
    assert(Events::pop() == Ev::None); // confirmação só ao soltar
    up(); assert(Events::pop() == Ev::Long && Events::pop() == Ev::None);
    down(); advance(8000);
    assert(Events::pop() == Ev::Reset && Events::pop() == Ev::None);
    up(); assert(Events::pop() == Ev::None); // reset não produz Long/Short

    // Primeira amostra invertida não dispara movimento ou sacudida.
    sample(0, 0, -GRAVITY);
    assert(Imu::lastMotionMs() == 0 && Events::pop() == Ev::None);
    // FaceDown exige orientação contínua por 1,5 s.
    sample(0, 0, -GRAVITY, 1499); assert(Events::pop() == Ev::None);
    sample(0, 0, -GRAVITY, 1); assert(Events::pop() == Ev::FaceDown);
    // Zona de histerese não acorda. Mais tarde, posição normal acorda uma vez.
    for (int i = 0; i < 60; i++) sample(0, 0, -5);
    assert(Events::pop() == Ev::None);
    for (int i = 0; i < 60; i++) sample(0, 0, GRAVITY);
    bool sawUp = false;
    for (Ev e; (e = Events::pop()) != Ev::None;) if (e == Ev::FaceUp) { assert(!sawUp); sawUp = true; }
    assert(sawUp);

    alive(); uint8_t hunger = sim.s().hunger;
    dispatch(Ev::Short);
    assert(scene == Scene::Life && act == Act::Eat && sim.s().hunger > hunger);
    // Uma sacudida não interrompe a refeição nem gasta energia.
    uint8_t energy = sim.s().energy;
    dispatch(Ev::Shake); assert(act == Act::Eat && sim.s().energy == energy);
    testMs += 3001; dispatch(Ev::Shake);
    assert(act == Act::Play && sim.s().energy == energy - PLAY_ENERGY_COST);
    testMs += 3001; dispatch(Ev::Shake);
    assert(sim.s().energy == energy - PLAY_ENERGY_COST); // cooldown de 5 s
    testMs += 2000; dispatch(Ev::Shake);
    assert(sim.s().energy == energy - 2 * PLAY_ENERGY_COST);

    alive(); dispatch(Ev::FaceDown);
    assert(sim.s().asleep && gestureSleep);
    dispatch(Ev::Shake); assert(sim.s().asleep);
    dispatch(Ev::Long); assert(scene == Scene::Menu && menuIdx == 4);
    dispatch(Ev::FaceUp); assert(!sim.s().asleep && !gestureSleep); // até no menu
    doMenu(4, testMs); assert(sim.s().asleep && !gestureSleep);
    dispatch(Ev::FaceUp); assert(sim.s().asleep); // sono manual não segue o gesto
    dispatch(Ev::Short); assert(!sim.s().asleep && sim.s().hunger == 80);

    // Menu sugerido, tilt com rearme, clique e timeout.
    alive(); PetState needs = sim.s(); needs.poop = 2; needs.sick = true;
    Storage::save(&needs, sizeof(needs)); sim.begin();
    dispatch(Ev::Long); assert(scene == Scene::Menu && menuIdx == 3);
    tiltValue = 0; testMs += 21; Game::update();
    tiltValue = .8f; testMs += 21; Game::update(); assert(menuIdx == 4);
    testMs += 21; Game::update(); assert(menuIdx == 4); // não repete enquanto inclinado
    dispatch(Ev::Short); assert(menuIdx == 5);
    tiltValue = 0; testMs += MENU_TIMEOUT_MS + 1; Game::update(); assert(scene == Scene::Life);

    // Segurar no menu não confirma cuidados antes do reset.
    alive(); dispatch(Ev::Long); menuIdx = 0;
    down(); advance(8000); Game::update();
    assert(scene == Scene::Select && sim.s().phase == Phase::Select && act == Act::None);
    up(); Game::update(); assert(sim.s().phase == Phase::Select);
    // Ovo parado após ligar não acumula os 15 s de graça.
    sim.choose(0, 42); motionAt = 0; lastEggMs = testMs; go(Scene::Egg);
    testMs += 1000; Game::update(); assert(sim.s().incubationMs == 0);

    // As poses cabem sem clipping; boca e patas sobrevivem à alimentação.
    for (uint8_t species = 0; species < 2; species++) {
        alive(species);
        const PetDef &d = def();
        const Anim *poses[] = {d.idle, d.blink, d.walk, d.eat, d.sleep, d.happy, d.sad, d.hungry};
        const int positions[] = {minCx(), maxCx()};
        for (const Anim *a : poses) {
            for (uint8_t i = 0; i < a->count; i++) {
                const Sprite &f = *a->frames[i];
                for (int x : positions) {
                    cv.clear(); cv.blitAnchored(f, x, 7, false);
                    int expected = 0, actual = 0;
                    for (int p = 0; p < f.w * f.h; p++) expected += f.px[p] != 0;
                    for (auto &row : cv.px) for (Rgb c : row) actual += c.r || c.g || c.b;
                    assert(expected == actual);
                }
            }
        }
        startAct(Act::Eat, 3000); testMs += 1000;
        cv.clear(); drawAction(testMs); Canvas eating = cv;
        cv.clear(); drawPet(*d.eat, testMs - actAt, petX, 7, testMs);
        assert(memcmp(eating.px, cv.px, sizeof(cv.px)) == 0);
    }
    puts("PASS: BOOT, debounce, reset, IMU, sleep/wake, menu, play cooldown, egg and sprite composition");
}
