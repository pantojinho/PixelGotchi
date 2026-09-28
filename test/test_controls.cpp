// Executa o código REAL com GPIO, relógio, NVS e sensor simulados.
// Game/Imu inclusos aqui permitem observar cenas sem API de debug no firmware.
#include "../src/Game.cpp"
#include "../src/Input.cpp"
#include "../src/Imu.cpp"
#include "../src/Storage.h"
#include "../src/Dream.h"
#include <assert.h>
#include <FastLED.h>

FakeFastLED FastLED;

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

void advance(uint32_t ms) { testMs += ms; Input::update(); }
void down() { testButton = LOW; advance(1); advance(25); }
void up() { testButton = HIGH; advance(1); advance(25); }
void alive(uint8_t species = 0) {
    Events::clear();
    sim.choose(species, 0x12345678); sim.hatch();
    act = Act::None; gestureSleep = false; playedOnce = false;
    petX = centerCx(idleW()); beh = Beh::Stand; behUntil = testMs + 5000;
    lastInputAt = testMs; observedMotionAt = Imu::lastMotionMs();
    sleepWasActive = false; dreamSeeded = false; dreamView = false;
    tiltValue = 0; tiltArmed = true; go(Scene::Life);
}

void dispatch(Ev e) { Events::push(e); Game::update(); }

bool lit(const Canvas &c, int x, int y) {
    if (x < 0 || x >= MATRIX_W || y < 0 || y >= MATRIX_H) return false;
    const Rgb p = c.get(x, y);
    return p.r || p.g || p.b;
}
int litCount(const Canvas &c) {
    int n = 0;
    for (int y = 0; y < MATRIX_H; ++y) for (int x = 0; x < MATRIX_W; ++x) n += lit(c, x, y);
    return n;
}

void testDreamAutomaton() {
    Dream::Automaton life;
    life.clear(); life.set(2, 3); life.set(3, 3); life.set(4, 3); // blinker: period 2
    life.step(); assert(life.population() == 3 && life.alive(3, 2) && life.alive(3, 3) && life.alive(3, 4));
    life.step(); assert(life.population() == 3 && life.alive(2, 3) && life.alive(3, 3) && life.alive(4, 3));
    life.clear(); life.set(3, 3); life.set(4, 3); life.set(3, 4); life.set(4, 4); // bloco imóvel
    for (int i = 0; i < 8; ++i) life.step();
    assert(life.population() == 4 && life.alive(3, 3) && life.alive(4, 4));
    life.clear(); // glider clássico, com as bordas conectadas
    life.set(3, 2); life.set(4, 3); life.set(2, 4); life.set(3, 4); life.set(4, 4);
    for (int i = 0; i < 4; ++i) life.step();
    assert(life.population() == 5 && life.alive(3, 5) && life.alive(5, 5));
    for (int i = 0; i < 12; ++i) life.step();
    assert(life.population() == 5 && life.alive(0, 0) && life.alive(0, 7) &&
           life.alive(6, 0) && life.alive(7, 0) && life.alive(7, 6)); // cruzou a borda

    // Capítulos: cada tipo mantém a intenção em qualquer orientação/posição.
    for (uint32_t seed = 0; seed < 4096; seed += 7) {
        Dream::Automaton g; g.seedChapter(Dream::Kind::Gliders, seed);
        const uint8_t pop = g.population();
        assert(pop == 5 || pop == 10);
        for (int i = 0; i < 32; ++i) { g.step(); assert(g.population() == pop); } // atravessam sem colidir
        Dream::Automaton p; p.seedChapter(Dream::Kind::Pulse, seed);
        const uint64_t start = p.signature();
        p.step(); assert(p.signature() != start);
        p.step(); assert(p.signature() == start); // pulsação de período 2
        Dream::Automaton soup; soup.seedChapter(Dream::Kind::Soup, seed * 2654435761u);
        Dream::Automaton again; again.seedChapter(Dream::Kind::Soup, seed * 2654435761u);
        assert(soup.signature() == again.signature() && soup.population()); // reproduzível
    }
}

// Três minutos de sono: capítulos mudam, sem tela apagada nem imobilidade longa.
void testDreamChapters() {
    alive();
    assert(sim.lightsOff() == Result::Ok);
    const uint32_t start = testMs;
    uint32_t firstChapter = 0, stillSince = 0, emptySince = 0;
    uint64_t lastSig = 0;
    bool sawDream = false;
    for (uint32_t t = 0; t <= 180000 + SLEEP_DREAM_AFTER_MS; t += 100) {
        const uint32_t now = start + t;
        updateDreamView(now, false);
        cv.clear(); nightFrame = false;
        drawLife(now);
        assert(litCount(cv) > 0); // nenhuma passagem pela tela inteira apagada
        if (!dreamView || now - dreamViewAt < DREAM_BUBBLE_MS) continue;
        if (!sawDream) { sawDream = true; firstChapter = dreamChapter; stillSince = emptySince = now; }
        const uint64_t sig = dreamGrid.signature();
        if (sig != lastSig) stillSince = now;
        if (dreamGrid.population()) emptySince = now;
        lastSig = sig;
        assert(now - stillSince <= (DREAM_STILL_STEPS + 1u) * DREAM_STEP_MS + 100);
        assert(now - emptySince <= DREAM_STEP_MS + 100);
    }
    assert(sawDream && dreamChapter - firstChapter >= 4); // capítulos de 20 a 40 s
    // Entrada: olhos fechados, bolhinhas e bolha crescendo sem apagão.
    alive();
    assert(sim.lightsOff() == Result::Ok);
    updateDreamView(testMs, false);
    updateDreamView(testMs + SLEEP_DREAM_AFTER_MS, false);
    assert(dreamView);
    for (uint32_t e = 0; e < DREAM_BUBBLE_MS; e += 50) {
        cv.clear();
        drawSleepingDream(dreamViewAt + e);
        assert(litCount(cv) > 0);
    }
}

// Refeição: a silhueta do pet fica intacta; a comida aparece, diminui e some.
void testMealSequence() {
    for (uint8_t species = 0; species < PET_COUNT; species++) {
        alive(species);
        const PetDef &d = def();
        doMenu(0, testMs);
        assert(act == Act::Eat && meal.x1 >= 0);
        if (species == 0) assert(meal.x1 == 7 && meal.y1 == 4); // capivara: logo à frente do focinho
        int lastFood = 3;
        for (uint32_t t = 0; t < EAT_MS; t += 20) {
            const uint32_t now = actAt + t;
            cv.clear(); drawAction(now); const Canvas scene = cv;
            const Anim &pose = t < EAT_APPROACH_MS ? *d.idle : t < EAT_MS - 600 ? *d.eat : *d.happy;
            cv.clear(); drawPet(pose, t, petX, 7, now); const Canvas petOnly = cv;
            int food = 0;
            for (int y = 0; y < MATRIX_H; ++y)
                for (int x = 0; x < MATRIX_W; ++x) {
                    if (lit(petOnly, x, y)) {
                        const Rgb a = petOnly.get(x, y), b = scene.get(x, y);
                        assert(a.r == b.r && a.g == b.g && a.b == b.b); // efeito nunca cobre o pet
                    }
                    const bool piece = (x == meal.x1 && y == meal.y1) || (x == meal.x2 && y == meal.y2);
                    food += piece && lit(scene, x, y) && !lit(petOnly, x, y);
                }
            if (t >= EAT_FOOD_MS + 100 && t < EAT_LAST_BITE_MS) assert(food >= 1);
            if (t >= EAT_APPROACH_MS) { assert(food <= lastFood); lastFood = food; } // só diminui
            if (t >= EAT_LAST_BITE_MS) assert(food == 0);
        }
        testMs = actAt + EAT_MS + 1;
        dispatch(Ev::None);
        assert(act == Act::None && litCount(cv) > 0); // volta ao descanso sem apagão
    }
}

// Conway ao redor do pet: só em pixels livres com respiro; autômato intacto.
void testAmbientConway() {
    for (uint8_t species = 0; species < 2; species++) {
        alive(species);
        bool sawCells[2] = {false, false};
        for (uint32_t visit = 0; visit < 2; ++visit) {
            const uint32_t begin = lastInputAt + IDLE_DREAM_AFTER_MS + visit * IDLE_DREAM_CYCLE_MS;
            for (uint32_t t = 0; t < IDLE_DREAM_SHOW_MS; t += 100) {
                const uint32_t now = begin + t;
                updateDreamView(now, false);
                assert(dreamView && dreamAmbient);
                cv.clear(); nightFrame = false;
                drawPet(*def().idle, now, petX, 7, now); const Canvas base = cv;
                drawAmbient(now);
                assert(dreamGrid.population() == (visit % 2 ? 3 : 5)); // máscara não apaga células
                for (int y = 0; y < MATRIX_H; ++y)
                    for (int x = 0; x < MATRIX_W; ++x) {
                        if (lit(base, x, y) || !lit(cv, x, y)) continue;
                        const Rgb c = cv.get(x, y);
                        const bool bird = c.r == 205 && c.g == 228 && c.b == 255;
                        if (!bird) {
                            assert(!lit(base, x - 1, y) && !lit(base, x + 1, y) &&
                                   !lit(base, x, y - 1) && !lit(base, x, y + 1));
                            sawCells[visit] = true;
                        }
                    }
            }
        }
        assert(sawCells[0] && sawCells[1]); // glider e blinker aparecem de fato
    }
}

// Acontecimentos ociosos cedem a BOOT e movimento.
void testIdlePriority() {
    alive();
    beh = Beh::Nap; behAt = testMs; behUntil = testMs + 9000;
    dispatch(Ev::Short);
    assert(act == Act::Eat && beh == Beh::Stand); // cuidado pedido entra na hora
    alive();
    beh = Beh::Watch; behAt = testMs; behUntil = testMs + 8000; bugX = 0; bugDir = 1;
    testMs += 21; motionAt = testMs; Game::update();
    assert(beh == Beh::Stand); // movimento interrompe o acontecimento
    motionAt = 0; observedMotionAt = 0;
    // Cada comportamento desenha o pet (nada substitui a silhueta).
    for (uint8_t b = 0; b < (uint8_t)Beh::COUNT; ++b) {
        alive(1);
        beh = (Beh)b; behAt = testMs; bugX = 5; bugY = 0;
        for (uint32_t t = 0; t < 4000; t += 100) {
            cv.clear(); drawLife(testMs + t);
            assert(litCount(cv) >= 10);
        }
    }
}

void testDreamTimingAndMotion() {
    alive();
    updateDreamView(testMs + IDLE_DREAM_AFTER_MS, false);
    assert(dreamView && !sim.s().asleep);
    updateDreamView(testMs + IDLE_DREAM_AFTER_MS + IDLE_DREAM_SHOW_MS, false);
    assert(!dreamView);
    updateDreamView(testMs + IDLE_DREAM_AFTER_MS + IDLE_DREAM_CYCLE_MS, false);
    assert(dreamView);

    alive();
    assert(sim.lightsOff() == Result::Ok);
    updateDreamView(testMs, false);
    assert(!dreamView);
    updateDreamView(testMs + SLEEP_DREAM_AFTER_MS, false);
    assert(dreamView && sim.s().asleep);
    updateDreamView(testMs + SLEEP_DREAM_AFTER_MS + 1, true);
    assert(!dreamView && sim.s().asleep); // movimento mostra o pet sem acordá-lo
    updateDreamView(testMs + SLEEP_DREAM_AFTER_MS + SLEEP_PET_REVEAL_MS + 2, false);
    assert(dreamView && sim.s().asleep); // volta ao sonho quando para de mexer
    assert(sim.wakeUp() == Result::Ok);
    updateDreamView(testMs + SLEEP_DREAM_AFTER_MS + SLEEP_PET_REVEAL_MS + 3, false);
    assert(!dreamView && !sleepWasActive);

    alive();
    while (sim.s().energy >= NEED_LOW) assert(sim.play() == Result::Ok);
    const uint32_t autoSleepAt = testMs + AUTO_SLEEP_IDLE_MS + 2;
    lastInputAt = testMs;
    sceneLife(autoSleepAt, Ev::None);
    assert(sim.s().asleep); // cochilo disparado pela regra de energia baixa
    updateDreamView(autoSleepAt, false);
    updateDreamView(autoSleepAt + SLEEP_DREAM_AFTER_MS, false);
    assert(dreamView && sim.s().asleep); // cochilo automático entra no mesmo sonho
}
void sample(float x, float y, float z, uint32_t ms = 20) {
    testMs += ms; testAccel = {{x, y, z}}; testSampleReady = true; Imu::update();
}

// Amostra com a tela a `up` m/s² "pra cima" (negativo = virada pra mesa),
// usando a calibração real do sensor (ele fica no verso da placa).
void sampleScreen(float up, uint32_t ms = 20) {
    float v[3] = {0, 0, 0};
    v[SCREEN_AXIS] = SCREEN_SIGN * up;
    sample(v[0], v[1], v[2], ms);
}

CRGB ledOutput(Rgb color, bool night = false) {
    Canvas canvas;
    canvas.clear(); canvas.set(0, 0, color);
    Display::show(canvas, night);
    assert(canvas.get(0, 0).r == color.r && canvas.get(0, 0).g == color.g && canvas.get(0, 0).b == color.b);
    CRGB result;
    for (int i = 0; i < FastLED.count; i++) {
        const CRGB p = FastLED.pixels[i];
        if (p.r || p.g || p.b) {
            // scale8 fixo do FastLED 3.6.0, sem correção de balanço de branco.
            result = CRGB(p.r * (FastLED.brightness + 1) / 256,
                          p.g * (FastLED.brightness + 1) / 256,
                          p.b * (FastLED.brightness + 1) / 256);
        }
    }
    return result;
}

int luminance(CRGB c) { return (54 * c.r + 183 * c.g + 19 * c.b); }
void testLedContrast() {
    assert(FastLED.brightness == LedProfile::BRIGHTNESS && FastLED.count == 64);
    assert(FastLED.order == RGB); // conferido na placa: com GRB o vermelho sai verde
    assert(FastLED.volts == 5 && FastLED.milliamps == 400 && FastLED.dither == 0);
    assert(LedProfile::channel(0) == 0 && LedProfile::channel(255) == 255);
    for (int i = 1; i < 256; i++) assert(LedProfile::channel(i) >= LedProfile::channel(i - 1));
    CRGB black = ledOutput({0, 0, 0}), white = ledOutput({255, 255, 255});
    assert(black.r == 0 && black.g == 0 && black.b == 0);
    // Brilho por cor: branco (3 canais) ofusca menos que uma cor pura; azul atenuado.
    CRGB red = ledOutput({255, 0, 0}), green = ledOutput({0, 255, 0}), blue = ledOutput({0, 0, 255});
    assert(red.r == LedProfile::BRIGHTNESS && red.g == 0 && red.b == 0);
    assert(green.g == LedProfile::BRIGHTNESS && green.r == 0 && green.b == 0);
    assert(blue.b < red.r && blue.b * 10 >= red.r * 6 && blue.r == 0 && blue.g == 0);
    assert(white.r == white.g && white.r > 0 && white.r < red.r && white.b <= white.r);
    // Piso: uma cor bem escura não some.
    CRGB darkest = ledOutput({30, 10, 5});
    assert(darkest.r >= LedProfile::MIN_PEAK);
    // Todos os 64 tons possíveis do DNA: focinho destaca, nariz não vira preto.
    for (uint32_t variant = 0; variant < 64; variant++) {
        Look look; look.tone = Dna{variant << 26}.tone();
        Canvas capy; capy.clear(); capy.blitAnchored(SPR_capy_idle0, 3, 7, false, look);
        const Rgb body = capy.get(0, 5), muzzle = capy.get(4, 3), nose = capy.get(6, 3);
        CRGB b = ledOutput(body), m = ledOutput(muzzle), n = ledOutput(nose);
        assert(luminance(m) >= 2 * luminance(b));
        assert(luminance(b) >= luminance(n) && n.r >= LedProfile::MIN_PEAK);
        // Dormindo: tudo no degrau mínimo, mas nada some.
        CRGB night = ledOutput(nose, true), nightBody = ledOutput(body, true);
        assert(night.r == NIGHT_LEVEL && nightBody.r == NIGHT_LEVEL);
    }
}

int main() {
    testDreamAutomaton();
    testDreamTimingAndMotion();
    testDreamChapters();
    testMealSequence();
    testAmbientConway();
    testIdlePriority();
    Display::begin(); testLedContrast();
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
    sampleScreen(-GRAVITY);
    assert(Imu::lastMotionMs() == 0 && Events::pop() == Ev::None);
    // FaceDown exige orientação contínua por 1,5 s.
    sampleScreen(-GRAVITY, 1499); assert(Events::pop() == Ev::None);
    sampleScreen(-GRAVITY, 1); assert(Events::pop() == Ev::FaceDown);
    // Zona de histerese não acorda. Mais tarde, posição normal acorda uma vez.
    for (int i = 0; i < 60; i++) sampleScreen(-5);
    assert(Events::pop() == Ev::None);
    for (int i = 0; i < 60; i++) sampleScreen(GRAVITY);
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

    // Cansado e sem clique/gesto por 30 s: dorme sozinho. Descansado, não.
    alive(); PetState tired = sim.s(); tired.energy = 10;
    Storage::save(&tired, sizeof(tired)); sim.begin();
    lastInputAt = testMs; testMs += AUTO_SLEEP_IDLE_MS - 1000; Game::update();
    assert(!sim.s().asleep);
    testMs += 2000; Game::update(); assert(sim.s().asleep);
    alive(); lastInputAt = testMs; testMs += AUTO_SLEEP_IDLE_MS + 1000; Game::update();
    assert(!sim.s().asleep);

    // Segurar no menu não confirma cuidados antes do reset.
    alive(); dispatch(Ev::Long); menuIdx = 0;
    down(); advance(8000); Game::update();
    assert(scene == Scene::Select && sim.s().phase == Phase::Select && act == Act::None);
    up(); Game::update(); assert(sim.s().phase == Phase::Select);
    // Ovo parado após ligar não acumula os 15 s de graça.
    sim.choose(0, 42); motionAt = 0; lastEggMs = testMs; go(Scene::Egg);
    testMs += 1000; Game::update(); assert(sim.s().incubationMs == 0);

    // As poses cabem sem clipping; boca e patas sobrevivem à alimentação.
    // Triste e cansado (poses paradas, em qualquer ponto) nunca são mais largos que o idle.
    for (uint8_t species = 0; species < PET_COUNT; species++) {
        const PetDef &d = PETS[species];
        const Anim *still[] = {d.sad, d.tired};
        for (const Anim *a : still)
            for (uint8_t i = 0; i < a->count; i++) assert(a->frames[i]->w <= d.idle->frames[0]->w);
    }
    for (uint8_t species = 0; species < 2; species++) {
        alive(species);
        const PetDef &d = def();
        const Anim *poses[] = {d.idle, d.blink, d.walk, d.eat, d.sleep, d.happy, d.sad, d.hungry, d.tired};
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
    }
    puts("PASS: Conway dreams/chapters/ambient, meal sequence, idle priority, LED contrast, BOOT, IMU, sleep/wake, menu, egg and sprite composition");
}
