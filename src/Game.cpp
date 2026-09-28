#include "Game.h"
#include "Canvas.h"
#include "CustomPet.h"
#include "Config.h"
#include "Display.h"
#include "Events.h"
#include "Imu.h"
#include "Input.h"
#include "PetSim.h"
#include "Dream.h"
#include "art/ArtData.h"
#include <esp_random.h>
#include <string.h>

using namespace Art;

namespace {

PetSim sim;
Canvas cv;

// ============================================================ utilidades
uint32_t rnd(uint32_t n) { return n ? esp_random() % n : 0; }
uint32_t rndRange(uint32_t a, uint32_t b) { return a + rnd(b - a + 1); }

const Rgb C_WHITE{255, 255, 255};
const Rgb C_DIM{40, 40, 52};
const Rgb C_RESET{255, 40, 40};
// Uma cor por necessidade = a cor do ícone do menu que resolve (art/props.art).
// Aparece no pontinho de alerta e nas barras do status.
const Rgb C_HUNGER{255, 58, 74};   // comida: vermelho
const Rgb C_BORED{154, 92, 255};   // brincar: roxo
const Rgb C_DIRTY{127, 216, 255};  // limpar: azul-claro
const Rgb C_SICK{62, 214, 76};     // remédio: verde
const Rgb C_ENERGY{255, 138, 26};  // dormir: amarelo "de LED" (#FF8A1A, escolhido na placa)
const Rgb C_HAPPY{255, 111, 168};  // alegria: rosa (coração)
const Rgb C_CARE = C_SICK;         // saúde

const Sprite &frame(const Anim &a, uint32_t t) { return frameAt(a, t); }

// cx que centraliza um sprite de largura w na matriz.
int centerCx(int w) { return (MATRIX_W - w) / 2 + w / 2; }

// Espécies de fábrica (0..PET_COUNT-1) e, se houver, a do editor (PET_COUNT).
uint8_t speciesCount() { return PET_COUNT + (CustomPet::available() ? 1 : 0); }
const PetDef &petAt(uint8_t species) {
    if (species < PET_COUNT) return PETS[species];
    return CustomPet::available() ? CustomPet::def() : PETS[0];
}
const PetDef &def() { return petAt(sim.s().species); }

// Cor principal do bicho (1ª cor da paleta), pra escrever o nome dele.
Rgb petColor() { return rgb(frameAt(*def().idle, 0).pal[1]); }

void scrollText(const char *str, uint32_t t, int y, Rgb color) {
    int w = Canvas::textWidth(str);
    int span = w + MATRIX_W + 2;
    cv.text(str, MATRIX_W - (int)((t / TEXT_STEP_MS) % span), y, color);
}

// Quanto dura uma passada completa do texto pela tela.
uint32_t scrollMs(const char *str) { return (uint32_t)(Canvas::textWidth(str) + MATRIX_W + 2) * TEXT_STEP_MS; }

// ============================================================ cenas
enum class Scene : uint8_t { Select, Egg, Hatch, Life, Menu, Status, Dead };
Scene scene = Scene::Select;
uint32_t sceneAt = 0;

void go(Scene s) {
    scene = s;
    sceneAt = millis();
}

// ---- seleção
uint8_t selIdx = 0;
bool tiltArmed = true;

// ---- ovo
uint32_t lastEggMs = 0;
uint32_t progressUntil = 0;

// ---- vida: comportamento autônomo
// Pausas (Stand) alternam com pequenas intenções: farejar um ponto, olhar em
// volta, observar algo passando (Watch), seguir uma borboleta (Chase) e se
// acomodar antes de cochilar (Nap). Cuidados e entradas sempre têm prioridade.
enum class Beh : uint8_t { Stand, Walk, Sniff, Look, Watch, Hop, Nap, Chase, COUNT };
Beh beh = Beh::Stand;
uint32_t behAt = 0, behUntil = 0;
int petX = 3, targetX = 3;
bool faceRight = true;
uint32_t lastStep = 0;
uint32_t blinkAt = 0;
int bugX = 0, bugY = 1, bugDir = 1; // borboleta (Chase) ou o que passa (Watch)
uint32_t bugStep = 0;
constexpr uint16_t NAP_SETTLE_MS = 1500; // boceja e se ajeita antes de fechar os olhos

// Refeição (3 s): olha (0..300), comida aparece, aproxima e mastiga (800..2400),
// comida diminui a cada mordida, satisfeito com coração nos últimos 600 ms.
constexpr uint32_t EAT_MS = 3000;
constexpr uint16_t EAT_FOOD_MS = 300;
constexpr uint16_t EAT_APPROACH_MS = 800;
constexpr uint16_t EAT_FIRST_BITE_MS = 1500;
constexpr uint16_t EAT_LAST_BITE_MS = 2100;

// ---- vida: ações com animação própria
enum class Act : uint8_t { None, Eat, Play, Clean, Medicine, Pet, Refuse, Flee, Forage, Wild, Grumpy };
Act act = Act::None;
uint32_t actAt = 0, actDur = 0;
uint32_t lastPlayAt = 0;
bool playedOnce = false;
bool gestureSleep = false;
bool nightFrame = false; // quadro atual é de "luz apagada" (brilho mínimo)
uint32_t lastInputAt = 0; // último clique/gesto (cansado + muito tempo sem isso = dorme)
uint32_t observedMotionAt = 0;

// Refeição: posições calculadas no início (comida fica parada no mundo).
struct Meal { int8_t fromX, toX, dir, x1, y1, x2, y2; };
Meal meal{3, 3, 1, -1, -1, -1, -1};

// Sonhos procedurais: Conway ao redor do pet no idle e mundo completo no sono.
Dream::Automaton dreamGrid, dreamPrevGrid;
bool sleepWasActive = false, dreamSeeded = false, dreamView = false;
bool dreamAmbient = false, dreamPlaceAmbient = false;
uint32_t sleepStartedAt = 0, sleepPetUntil = 0, lastDreamStep = 0, dreamSeedValue = 1;
uint32_t dreamBaseSeed = 1, dreamChapter = 0, dreamChapterAt = 0, dreamChapterMs = 0, dreamViewAt = 0;
uint32_t ambientVisit = 0;
uint64_t dreamSig1 = 0, dreamSig2 = 0; // assinaturas das duas gerações anteriores
uint8_t dreamStillSteps = 0, dreamOscSteps = 0;
bool dreamIsCalm = true, dreamFading = false;
Dream::Kind dreamKind = Dream::Kind::Gliders;
Rgb dreamBright{90, 210, 245}, dreamDim{28, 82, 135};
Rgb dreamPrevBright{90, 210, 245}, dreamPrevDim{28, 82, 135};

// ---- menu / status
const Sprite *const MENU_ICONS[] = {&SPR_icon_food, &SPR_icon_play, &SPR_icon_clean,
                                    &SPR_icon_medicine, &SPR_icon_sleep, &SPR_icon_pet,
                                    &SPR_icon_status, &SPR_icon_back};
constexpr uint8_t MENU_COUNT = sizeof(MENU_ICONS) / sizeof(MENU_ICONS[0]);
constexpr uint8_t MENU_STATUS = 6;
uint8_t menuIdx = 0;
uint32_t menuAt = 0;
uint8_t statusPage = 0;

// Uma inclinação = um passo. Voltar ao centro rearma seleção e menu.
bool tiltStep(uint8_t &index, uint8_t count) {
    float tilt = Imu::tilt();
    if (tiltArmed && (tilt > TILT_STEP || tilt < -TILT_STEP)) {
        index = (index + (tilt > 0 ? 1 : count - 1)) % count;
        tiltArmed = false;
        return true;
    }
    if (tilt < TILT_REARM && tilt > -TILT_REARM) tiltArmed = true;
    return false;
}

uint8_t suggestedCare() {
    const PetState &s = sim.s();
    if (s.asleep) return 4;
    if (s.sick) return 3;
    if (s.poop) return 2;
    if (s.hunger < NEED_LOW) return 0;
    if (s.energy < NEED_LOW) return 4;
    if (s.happy < NEED_LOW) return 1;
    return 5; // carinho quando está tudo bem
}

// ============================================================ bicho: limites e aparência
int idleW() { return frame(*def().idle, 0).w; }
int minCx() { return idleW() / 2; }
int maxCx() {
    int w = idleW();
    return MATRIX_W - 1 - (w - 1 - w / 2);
}
int clampX(int x) { return x < minCx() ? minCx() : (x > maxCx() ? maxCx() : x); }

Look petLook(uint32_t now) {
    Look l;
    l.tone = sim.dna().tone();
    const PetState &s = sim.s();
    bool wildNow = s.wild;
    // Estágio 2 de descuido: de vez em quando "pisca" a cara selvagem.
    if (!s.wild && sim.neglectStage() >= 2 && (now % 6000) < 250) wildNow = true;
    if (act == Act::Wild) wildNow = ((now - actAt) / 150) % 2;
    if (wildNow) l.pal = def().wildPal;
    if (s.sick) l.tint = Tint::Sick;
    return l;
}

bool petFlip() { return def().side && !faceRight; }

void drawPet(const Anim &a, uint32_t t, int cx, int bottom, uint32_t now) {
    cv.blitAnchored(frame(a, t), cx, bottom, petFlip(), petLook(now));
}

void startAct(Act a, uint32_t dur) {
    act = a;
    actAt = millis();
    actDur = dur;
}

// ============================================================ comportamento autônomo
// Cadeia de Markov simples: pesos pro próximo comportamento dependem do
// atual, do humor e da personalidade (DNA). Selvagem = mais arisco.
void pickBehavior(uint32_t now) {
    const PetState &s = sim.s();
    Dna d = sim.dna();
    int w[(int)Beh::COUNT];
    w[(int)Beh::Stand] = 30;
    w[(int)Beh::Walk] = 20 + d.activity() / 8;
    w[(int)Beh::Sniff] = 6 + d.curiosity() / 16;
    w[(int)Beh::Look] = 6 + d.curiosity() / 16;
    w[(int)Beh::Watch] = 4 + d.curiosity() / 16;
    w[(int)Beh::Hop] = s.happy > 60 ? 3 + d.activity() / 32 : 0;
    w[(int)Beh::Nap] = s.energy < 40 ? 20 : 3;
    w[(int)Beh::Chase] = s.energy > 40 ? 3 + d.activity() / 32 : 0;
    // Capivara tranquila; gato curioso e caçador. O DNA ainda varia cada pet.
    if (strcmp(def().id, "capy") == 0) {
        w[(int)Beh::Stand] += 20;
        w[(int)Beh::Sniff] += 16;
        w[(int)Beh::Watch] += 6;
        w[(int)Beh::Nap] += 8;
        w[(int)Beh::Chase] = 0;
        w[(int)Beh::Hop] /= 2;
    } else if (strcmp(def().id, "cat") == 0) {
        w[(int)Beh::Look] += 6;
        w[(int)Beh::Watch] += 14;
        w[(int)Beh::Chase] += s.energy > 40 ? 14 : 0;
    }
    if (beh == Beh::Walk) w[(int)Beh::Stand] *= 2;
    if (beh == Beh::Stand) w[(int)Beh::Walk] = w[(int)Beh::Walk] * 3 / 2;
    if (beh != Beh::Stand && beh != Beh::Walk) w[(int)beh] = 0; // não repete o "especial"
    if (s.wild) {
        w[(int)Beh::Look] *= 3;
        w[(int)Beh::Watch] *= 2;
        w[(int)Beh::Sniff] *= 2;
        w[(int)Beh::Walk] = w[(int)Beh::Walk] * 3 / 2;
        w[(int)Beh::Hop] = 0;
        w[(int)Beh::Chase] *= 2;
        w[(int)Beh::Nap] /= 2;
    }
    int total = 0;
    for (int v : w) total += v;
    int r = rnd(total);
    int i = 0;
    while (r >= w[i]) r -= w[i++];
    beh = (Beh)i;
    behAt = now;

    switch (beh) {
        case Beh::Walk:
            targetX = rndRange(minCx(), maxCx());
            if (targetX == petX) beh = Beh::Stand;
            behUntil = now + 8000;
            break;
        case Beh::Stand: {
            // Pausas mais longas no pet preguiçoso, mais curtas no elétrico.
            const uint32_t lazy = (255u - d.activity()) * 12u; // 0..3060 ms
            behUntil = now + 1500 + lazy + rnd(2500);
            break;
        }
        case Beh::Sniff: behUntil = now + rndRange(1800, 3000); break;
        case Beh::Look: behUntil = now + rndRange(1500, 3000); break;
        case Beh::Watch:
            // Algo cruza o alto da tela; ele acompanha até sair do outro lado.
            bugDir = rnd(2) ? 1 : -1;
            bugX = bugDir > 0 ? -1 : MATRIX_W;
            bugY = rndRange(0, 1);
            bugStep = now;
            behUntil = now + 8000;
            break;
        case Beh::Hop: behUntil = now + 1500; break;
        case Beh::Nap: behUntil = now + NAP_SETTLE_MS + rndRange(5000, 9000); break;
        case Beh::Chase:
            bugX = rnd(MATRIX_W);
            bugY = rndRange(0, 2);
            behUntil = now + 6000;
            break;
        default: break;
    }
}

// BOOT ou movimento: interrompe o acontecimento ocioso na hora, com uma piscada
// (resposta visível). Cuidados pedidos entram por cima em seguida.
void reactToInput(uint32_t now) {
    if (beh == Beh::Stand || beh == Beh::Walk) return;
    beh = Beh::Stand;
    behAt = now;
    behUntil = now + 2500;
    blinkAt = now;
}

void stepToward(int target, uint32_t now, uint16_t stepMs) {
    if (now - lastStep < stepMs || petX == target) return;
    lastStep = now;
    faceRight = target > petX;
    petX += faceRight ? 1 : -1;
}

void updateBehavior(uint32_t now) {
    const PetState &s = sim.s();
    uint16_t stepMs = s.wild ? 170 : 330;

    // Inclinar a placa: o bicho escorrega/anda pro lado mais baixo.
    float tilt = Imu::tilt();
    if (tilt > 0.35f || tilt < -0.35f) {
        beh = Beh::Walk;
        targetX = tilt > 0 ? maxCx() : minCx();
        behUntil = now + 1500;
        stepToward(targetX, now, 200);
        return;
    }

    // Com fome ou doente ele não passeia: fica num canto pedindo ajuda.
    if (s.hunger < NEED_LOW || s.sick || s.happy < NEED_LOW) {
        stepToward(minCx(), now, 400);
        return;
    }
    if (s.energy < NEED_LOW) return; // cansado demais pra passear: fica onde está

    switch (beh) {
        case Beh::Walk:
            stepToward(targetX, now, stepMs);
            if (petX == targetX) behUntil = now;
            break;
        case Beh::Look:
            if ((now - behAt) / 700 % 2 == 1) faceRight = !faceRight, behAt = now;
            break;
        case Beh::Watch:
            // Capivara acompanha devagar; gato, atento, um pouco mais rápido.
            if (now - bugStep > (strcmp(def().id, "capy") == 0 ? 420u : 320u)) {
                bugStep = now;
                bugX += bugDir;
                if (bugX < -1 || bugX > MATRIX_W) behUntil = now;
            }
            faceRight = bugX >= petX;
            break;
        case Beh::Chase:
            if (now - bugStep > 260) {
                bugStep = now;
                bugX += (int)rnd(3) - 1;
                bugX = bugX < 0 ? 0 : (bugX >= MATRIX_W ? MATRIX_W - 1 : bugX);
                bugY = (int)rndRange(0, 2);
            }
            stepToward(clampX(bugX), now, stepMs - 60);
            break;
        default: break;
    }
    if (now >= behUntil) pickBehavior(now);
}

// ============================================================ desenho da vida
void setFree(int x, int y, Rgb color);
void blitFree(const Sprite &sprite, int x, int y);
void drawPoop(uint32_t now, int sweepX = -1) {
    static const int8_t slots[POOP_MAX] = {5, 0, 3};
    for (uint8_t i = 0; i < sim.s().poop && i < POOP_MAX; i++) {
        int x = slots[i];
        if (sweepX >= 0 && x <= sweepX) continue;
        blitFree(SPR_fx_poop, x, MATRIX_H - SPR_fx_poop.h);
        if ((now / 400 + i) % 3 == 0) setFree(x + 1, MATRIX_H - SPR_fx_poop.h - 2, {70, 90, 60}); // "cheirinho"
    }
}

// Item do menu que resolve a necessidade mais urgente, ou -1 se está tudo bem.
// Mesma ordem do item sugerido (segurar o BOOT já abre nele).
int urgentNeed() {
    if (!sim.needsAttention()) return -1;
    const PetState &s = sim.s();
    if (s.sick) return 3;
    if (s.poop) return 2;
    if (s.hunger < NEED_LOW) return 0;
    if (s.energy < NEED_LOW) return 4;
    return 1;
}

// Pedido de ajuda, em dois jeitos:
//  - de tempos em tempos a tela mostra o ícone do menu que resolve
//    (entre poses: nunca por cima do rosto em 64 LEDs);
//  - no resto do tempo, um pontinho pisca no canto com a cor desse ícone.
void drawNeed(uint32_t now) {
    static const Rgb COLORS[] = {C_HUNGER, C_BORED, C_DIRTY, C_SICK, C_ENERGY};
    int need = urgentNeed();
    if (need < 0) return;
    if ((now % NEED_BUBBLE_EVERY_MS) < NEED_BUBBLE_MS) {
        cv.clear();
        const Sprite &icon = *MENU_ICONS[need];
        cv.blit(icon, (MATRIX_W - icon.w) / 2, (MATRIX_H - icon.h) / 2);
        return;
    }
    if ((now / 700) % 2) cv.set(7, 0, COLORS[need]);
}

void drawRising(const Sprite &s, int x, uint32_t t, uint16_t period, int from = 2) {
    int y = from - (int)((t % period) * (from + s.h + 1) / period);
    cv.blit(s, x, y);
}

bool emptyPixel(int x, int y) {
    if (x < 0 || x >= 8 || y < 0 || y >= 8) return false;
    const Rgb c = cv.get(x, y);
    return !(c.r || c.g || c.b);
}

void setFree(int x, int y, Rgb color) {
    if (emptyPixel(x, y)) cv.set(x, y, color);
}

// Efeitos pequenos procuram espaço livre e preservam a anatomia do sprite.
void tinyHeart(Rgb color = C_HAPPY) {
    for (int y = 0; y < 3; ++y) for (int x = 1; x < 5; ++x) {
        if (emptyPixel(x, y) && emptyPixel(x + 2, y) && emptyPixel(x + 1, y + 1)) {
            cv.set(x, y, color); cv.set(x + 2, y, color); cv.set(x + 1, y + 1, color);
            return;
        }
    }
    setFree(0, 0, color);
}

// ---- boca e comida
// Boca = linha mais baixa em que o frame de comer difere do idle (centro dos
// pixels diferentes). Coluna em coordenadas do idle; linha contada de baixo.
struct Mouth { int col, rowFromBottom; };
Mouth mouthOf(const PetDef &d) {
    const Sprite &a = *d.idle->frames[0], &b = *d.eat->frames[d.eat->count - 1];
    auto at = [](const Sprite &s, int rel, int ry) -> uint8_t {
        const int sx = rel + s.w / 2, sy = s.h - 1 - ry;
        return sx < 0 || sx >= s.w || sy < 0 ? 0 : s.px[sy * s.w + sx];
    };
    const int maxH = a.h > b.h ? a.h : b.h;
    for (int ry = 0; ry < maxH; ++ry) {
        int sum = 0, n = 0;
        for (int rel = -MATRIX_W; rel < MATRIX_W; ++rel)
            if (at(a, rel, ry) != at(b, rel, ry)) sum += rel + a.w / 2, ++n;
        if (n) return {sum / n, ry};
    }
    return {a.w - 1, a.h / 2};
}

// Posição da boca na tela com o pet ancorado em cx.
void mouthAt(int cx, bool flip, int &x, int &y) {
    const Sprite &idle = frame(*def().idle, 0);
    const Mouth m = mouthOf(def());
    x = cx - idle.w / 2 + (flip ? idle.w - 1 - m.col : m.col);
    y = MATRIX_H - 1 - m.rowFromBottom;
}

// As duas cores mais usadas no sprite da comida (principal, detalhe).
void foodColors(Rgb &main, Rgb &detail) {
    const Sprite &f = *def().food;
    uint8_t count[16]{};
    for (int i = 0; i < f.w * f.h; ++i) if (f.px[i] < 16) ++count[f.px[i]];
    uint8_t first = 0, second = 0;
    for (uint8_t i = 1; i < 16; ++i) {
        if (!count[i]) continue;
        if (!first || count[i] > count[first]) second = first, first = i;
        else if (!second || count[i] > count[second]) second = i;
    }
    main = rgb(f.pal[first ? first : 1]);
    detail = second ? rgb(f.pal[second]) : main;
}

// Planeja a refeição: o pet olha para o lado do focinho, a comida aparece no
// primeiro pixel livre à frente da boca e o pet dá um passo até ela quando cabe.
void planMeal() {
    const PetDef &d = def();
    const Sprite &idle = frame(*d.idle, 0);
    const int home = centerCx(idle.w);
    const int left = home - idle.w / 2, right = left + idle.w - 1;
    meal.dir = d.side ? 1 : (MATRIX_W - 1 - right >= left ? 1 : -1);
    faceRight = meal.dir > 0;
    meal.fromX = home;
    const bool flip = petFlip();
    Canvas body;
    auto freeAt = [&](int x, int y) {
        if (x < 0 || x >= MATRIX_W || y < 0 || y >= MATRIX_H) return false;
        const Rgb c = body.get(x, y);
        return !(c.r || c.g || c.b);
    };
    int mx = 0, my = 0;
    // Dá um passo até a comida só se ainda sobrar espaço à frente da boca
    // (a capivara, de focinho comprido, mastiga sem sair do lugar).
    const int targets[] = {clampX(home + meal.dir), home};
    for (int to : targets) {
        meal.toX = to;
        body.clear();
        body.blitAnchored(idle, to, 7, flip);
        for (uint8_t i = 0; i < d.eat->count; ++i) body.blitAnchored(*d.eat->frames[i], to, 7, flip);
        mouthAt(to, flip, mx, my);
        meal.x1 = meal.y1 = meal.x2 = meal.y2 = -1;
        for (int x = mx; x >= 0 && x < MATRIX_W; x += meal.dir)
            if (freeAt(x, my)) { meal.x1 = x; meal.y1 = my; break; }
        if (meal.x1 >= 0) break;
    }
    if (meal.x1 < 0) { // sem espaço na linha da boca: pixel livre mais próximo
        int best = 1 << 20;
        for (int y = 0; y < MATRIX_H; ++y) for (int x = 0; x < MATRIX_W; ++x) {
            const int dist = (x - mx) * (x - mx) + (y - my) * (y - my);
            if (freeAt(x, y) && dist < best) best = dist, meal.x1 = x, meal.y1 = y;
        }
    }
    if (meal.x1 < 0) return;
    if (freeAt(meal.x1 + meal.dir, meal.y1)) meal.x2 = meal.x1 + meal.dir, meal.y2 = meal.y1;
    else if (freeAt(meal.x1, meal.y1 - 1)) meal.x2 = meal.x1, meal.y2 = meal.y1 - 1;
}

// Pixel livre no chão à frente do focinho, pra farejar.
bool sniffSpot(int &sx, int &sy) {
    int mx, my;
    mouthAt(petX, petFlip(), mx, my);
    const int dir = def().side ? (faceRight ? 1 : -1) : (petX <= MATRIX_W / 2 ? 1 : -1);
    for (int dx = 0; dx < 4; ++dx)
        for (int y = MATRIX_H - 1; y >= my; --y) {
            const int x = mx + dir * dx;
            if (emptyPixel(x, y)) { sx = x; sy = y; return true; }
        }
    return false;
}

// Borboleta: asas abertas (2 px) e fechadas (1 px) alternando.
void drawButterfly(int x, int y, uint32_t now) {
    const bool open = (now / 200) % 2;
    setFree(x, y, open ? Rgb{255, 176, 48} : Rgb{255, 226, 140});
    if (open) setFree(x + 1, y, {255, 176, 48});
}

void blitFree(const Sprite &sprite, int x, int y) {
    Canvas effect; effect.clear(); effect.blit(sprite, x, y);
    for (int sy = 0; sy < 8; ++sy) for (int sx = 0; sx < 8; ++sx) {
        const Rgb c = effect.get(sx, sy);
        if (c.r || c.g || c.b) setFree(sx, sy, c);
    }
}

uint32_t dreamHash(uint32_t value) {
    value ^= value >> 16; value *= 0x7FEB352Du;
    value ^= value >> 15; value *= 0x846CA68Bu;
    return value ^ (value >> 16);
}

// Capítulos do sono: o DNA escolhe por qual tipo começa e o contador varia.
// Sonho agitado (fome, sujeira, doença...) repete mais o campo turbulento.
Dream::Kind sleepChapterKind(uint32_t chapter) {
    static const Dream::Kind CALM[] = {Dream::Kind::Gliders, Dream::Kind::Pulse, Dream::Kind::Soup};
    static const Dream::Kind RESTLESS[] = {Dream::Kind::Soup, Dream::Kind::Gliders, Dream::Kind::Soup,
                                           Dream::Kind::Pulse};
    const uint32_t i = dreamBaseSeed % 4 + chapter;
    return dreamIsCalm ? CALM[i % 3] : RESTLESS[i % 4];
}

void startDreamChapter(uint32_t now, bool fade) {
    dreamPrevGrid = dreamGrid;
    dreamPrevBright = dreamBright;
    dreamPrevDim = dreamDim;
    dreamFading = fade && dreamPrevGrid.population();
    dreamSeedValue = dreamHash(dreamBaseSeed + dreamChapter * 0x9E3779B9u);
    dreamKind = sleepChapterKind(dreamChapter);
    dreamGrid.seedChapter(dreamKind, dreamSeedValue);
    // Paleta pequena, duas cores bem separadas por capítulo.
    const Rgb calmBright[] = {{90, 210, 245}, {120, 235, 110}, {195, 138, 245}};
    const Rgb calmDim[] = {{28, 82, 135}, {28, 90, 45}, {75, 36, 130}};
    const Rgb warmBright[] = {{255, 112, 96}, {255, 155, 45}, {245, 95, 165}};
    const Rgb warmDim[] = {{118, 35, 85}, {105, 42, 15}, {90, 25, 70}};
    const uint8_t palette = dreamChapter % 3;
    dreamBright = dreamIsCalm ? calmBright[palette] : warmBright[palette];
    dreamDim = dreamIsCalm ? calmDim[palette] : warmDim[palette];
    dreamChapterAt = lastDreamStep = now;
    dreamChapterMs = DREAM_CHAPTER_MIN_MS + dreamSeedValue % (DREAM_CHAPTER_MAX_MS - DREAM_CHAPTER_MIN_MS + 1);
    dreamStillSteps = dreamOscSteps = 0;
    dreamSig1 = dreamSig2 = ~0ull;
}

void seedDream(uint32_t now) {
    const PetState &s = sim.s();
    Dna dna = sim.dna();
    dreamBaseSeed = dna.bits ^ ((uint32_t)s.hunger << 24) ^ ((uint32_t)s.energy << 16) ^
                    ((uint32_t)s.happy << 8) ^ s.ageMin ^ ((uint32_t)s.poop << 4);
    dreamIsCalm = !s.sick && !s.wild && s.poop == 0 && s.hunger >= 50 &&
                  s.happy >= 50 && s.energy >= 50;
    dreamAmbient = false;
    dreamPlaceAmbient = false;
    dreamChapter = 0;
    dreamGrid.clear();
    startDreamChapter(now, false);
    dreamSeeded = true;
}

// Visita acordada: um só elemento com intenção clara. Visitas pares trazem um
// glider (deslocamento); ímpares, um blinker (pulsação) e um passarinho.
void seedAmbient(uint32_t now, uint32_t visit) {
    dreamBaseSeed = dreamHash(sim.dna().bits ^ (visit * 0x9E3779B9u));
    dreamIsCalm = true;
    dreamAmbient = true;
    dreamPlaceAmbient = true; // posição escolhida no desenho, olhando o espaço livre
    dreamChapter = visit;
    dreamKind = visit % 2 ? Dream::Kind::Pulse : Dream::Kind::Gliders;
    dreamSeedValue = dreamBaseSeed & ~(7u << 9); // um glider só / blinker simples
    dreamGrid.seedChapter(dreamKind, dreamSeedValue);
    dreamBright = {90, 210, 245};
    dreamDim = {28, 82, 135};
    dreamChapterAt = lastDreamStep = now;
    dreamFading = false;
    dreamSeeded = true;
}

void advanceDream(uint32_t now) {
    if (!dreamSeeded) seedDream(now);
    if (dreamFading && now - dreamChapterAt >= DREAM_FADE_MS) dreamFading = false;
    if (now - lastDreamStep < DREAM_STEP_MS) return;
    lastDreamStep = now;
    const uint64_t before = dreamGrid.signature();
    dreamGrid.step();
    if (dreamAmbient) return; // visitas curtas não trocam de capítulo
    const uint64_t after = dreamGrid.signature();
    dreamStillSteps = after == before ? (dreamStillSteps < 255 ? dreamStillSteps + 1 : 255) : 0;
    const bool oscillating = after != before && (after == dreamSig1 || after == dreamSig2);
    dreamOscSteps = oscillating ? (dreamOscSteps < 255 ? dreamOscSteps + 1 : 255) : 0;
    dreamSig2 = dreamSig1;
    dreamSig1 = before;
    const uint32_t elapsed = now - dreamChapterAt;
    // Pulsação é o tema do capítulo Pulse; nos outros, um oscilador que sobrou
    // pulsa só por algum tempo antes da próxima mudança.
    const bool staleOsc = dreamKind != Dream::Kind::Pulse &&
                          (uint32_t)dreamOscSteps * DREAM_STEP_MS >= DREAM_OSC_MAX_MS;
    if (!dreamGrid.population() || dreamStillSteps >= DREAM_STILL_STEPS || staleOsc ||
        elapsed >= dreamChapterMs) {
        ++dreamChapter;
        startDreamChapter(now, true);
    }
}

bool untilActive(uint32_t now, uint32_t until) { return (int32_t)(until - now) > 0; }

void updateDreamView(uint32_t now, bool activity) {
    const bool wasDreamView = dreamView;
    dreamView = false;
    if (sim.s().asleep) {
        if (!sleepWasActive) {
            sleepWasActive = true;
            sleepStartedAt = now;
            sleepPetUntil = now + SLEEP_DREAM_AFTER_MS;
            seedDream(now);
        }
        if (activity) sleepPetUntil = now + SLEEP_PET_REVEAL_MS;
        const bool initialPet = now - sleepStartedAt < SLEEP_DREAM_AFTER_MS;
        dreamView = !initialPet && !untilActive(now, sleepPetUntil);
        if (dreamView && !wasDreamView) dreamViewAt = now;
        // O mundo só evolui depois que a bolha tomou a tela.
        if (dreamView && now - dreamViewAt >= DREAM_BUBBLE_MS) advanceDream(now);
        else if (dreamView) lastDreamStep = now;
        return;
    }

    if (sleepWasActive) {
        sleepWasActive = false;
        dreamSeeded = false;
    }
    if (scene != Scene::Life || act != Act::None || activity ||
        sim.s().energy < NEED_LOW || sim.needsAttention()) {
        if (activity || scene != Scene::Life) dreamSeeded = false;
        return;
    }
    const uint32_t idle = now - lastInputAt;
    if (idle < IDLE_DREAM_AFTER_MS) {
        dreamSeeded = false;
        return;
    }
    const uint32_t visit = (idle - IDLE_DREAM_AFTER_MS) / IDLE_DREAM_CYCLE_MS;
    const uint32_t phase = (idle - IDLE_DREAM_AFTER_MS) % IDLE_DREAM_CYCLE_MS;
    dreamView = phase < IDLE_DREAM_SHOW_MS;
    if (dreamView && (!dreamSeeded || !dreamAmbient || visit != ambientVisit)) {
        ambientVisit = visit;
        seedAmbient(now, visit);
    }
    if (dreamView) advanceDream(now);
}

bool ambientBird() { return dreamAmbient && dreamKind == Dream::Kind::Pulse; }
int birdX(uint32_t now) { return 8 - (int)((now / 150) % 12); }

// Máscara de renderização: a célula só aparece em pixel livre que não encosta
// no pet (1 px de respiro protege olhos, focinho e silhueta). O autômato
// continua completo por baixo.
bool ambientPixel(const Canvas &base, int x, int y) {
    auto lit = [&](int px, int py) {
        if (px < 0 || px >= MATRIX_W || py < 0 || py >= MATRIX_H) return false;
        const Rgb c = base.get(px, py);
        return c.r || c.g || c.b;
    };
    return !lit(x, y) && !lit(x - 1, y) && !lit(x + 1, y) && !lit(x, y - 1) && !lit(x, y + 1);
}

// Escolhe, entre algumas orientações/posições, a que mais aparece fora do pet
// nas próximas gerações. Só muda onde o padrão nasce; as regras seguem iguais.
void placeAmbient(const Canvas &base) {
    uint32_t bestSeed = dreamSeedValue;
    int best = -1;
    for (uint32_t k = 0; k < 16; ++k) {
        // Bits 0..8: orientação e posição; 9..11 zerados (padrão simples).
        const uint32_t candidate = (dreamSeedValue & ~0xFFFu) | ((dreamSeedValue + k * 37u) & 0x1FFu);
        Dream::Automaton probe;
        probe.seedChapter(dreamKind, candidate);
        int visible = 0;
        for (int g = 0; g < 6; ++g) {
            for (int y = 0; y < MATRIX_H; ++y) for (int x = 0; x < MATRIX_W; ++x)
                if (probe.alive(x, y) && ambientPixel(base, x, y)) ++visible;
            probe.step();
        }
        if (visible > best) best = visible, bestSeed = candidate;
    }
    dreamSeedValue = bestSeed;
    dreamGrid.seedChapter(dreamKind, dreamSeedValue);
    dreamPlaceAmbient = false;
}

// Mundo inteiro do sonho, com a troca de capítulo feita por substituição de
// pixels (nunca passa por uma tela toda apagada).
void drawDreamWorld(uint32_t now) {
    const uint32_t t = now - dreamChapterAt;
    const uint8_t stage = dreamFading ? 1 + t * 4 / DREAM_FADE_MS : 4;
    for (int y = 0; y < MATRIX_H; ++y) {
        for (int x = 0; x < MATRIX_W; ++x) {
            const bool fresh = (x + 2 * y) % 4 < stage;
            const Dream::Automaton &grid = fresh ? dreamGrid : dreamPrevGrid;
            if (grid.alive(x, y))
                cv.set(x, y, ((x + y) & 1) ? (fresh ? dreamBright : dreamPrevBright)
                                           : (fresh ? dreamDim : dreamPrevDim));
        }
    }
}

// Conway ao redor do pet acordado (o pet já está desenhado no canvas).
void drawAmbient(uint32_t now) {
    const Canvas base = cv;
    if (dreamPlaceAmbient) placeAmbient(base);
    for (int y = 0; y < MATRIX_H; ++y)
        for (int x = 0; x < MATRIX_W; ++x)
            if (dreamGrid.alive(x, y) && ambientPixel(base, x, y))
                cv.set(x, y, ((x + y) & 1) ? dreamBright : dreamDim);
    if (ambientBird()) {
        // Um passarinho de três pixels cruza o alto da tela.
        const int x = birdX(now), y = 1 + (int)((now / 700) % 2);
        const Rgb color{205, 228, 255};
        setFree(x, y + 1, color); setFree(x + 1, y, color); setFree(x + 2, y + 1, color);
    }
}

// Para onde o pet olha durante a visita: o passarinho ou o centro do padrão.
int ambientFocusX(uint32_t now) {
    if (ambientBird()) return birdX(now) + 1;
    int sum = 0, n = 0;
    for (int y = 0; y < MATRIX_H; ++y)
        for (int x = 0; x < MATRIX_W; ++x)
            if (dreamGrid.alive(x, y)) sum += x, ++n;
    return n ? sum / n : petX;
}

void drawAction(uint32_t now) {
    const PetDef &d = def();
    uint32_t t = now - actAt;
    switch (act) {
        case Act::Eat: {
            // 1) olha para o lado do focinho; 2) a comida aparece num pixel
            // livre; 3) aproxima o focinho e mastiga enquanto ela diminui;
            // 4) satisfeito, um coraçãozinho. O pet nunca some.
            petX = t < EAT_APPROACH_MS ? meal.fromX : meal.toX;
            faceRight = meal.dir > 0;
            const Anim &pose = t < EAT_APPROACH_MS ? *d.idle : t < actDur - 600 ? *d.eat : *d.happy;
            drawPet(pose, t, petX, 7, now);
            Rgb main, detail;
            foodColors(main, detail);
            if (t >= EAT_FOOD_MS && t < EAT_LAST_BITE_MS && meal.x1 >= 0) setFree(meal.x1, meal.y1, main);
            if (t >= EAT_FOOD_MS + 100 && t < EAT_FIRST_BITE_MS && meal.x2 >= 0) setFree(meal.x2, meal.y2, detail);
            if (t >= actDur - 600) tinyHeart();
            break;
        }
        case Act::Play: {
            petX = centerCx(idleW());
            drawPet(t < 650 ? *d.idle : *d.happy, t, petX,
                    t < 650 || t >= actDur - 450 ? 7 : 7 - (int)((t / 350) % 2), now);
            if (t < 650) setFree(1 + (t / 130) % 5, 0, {255, 155, 45});
            if (t >= actDur - 450) tinyHeart();
            break;
        }
        case Act::Clean: {
            drawPet(*d.idle, t, petX, 7, now);
            int sweep = (int)(t / 140) - 1;
            drawPoop(now, sweep);
            blitFree(frame(ANIM_fx_sparkle_anim, t), sweep - 1, MATRIX_H - 3);
            break;
        }
        case Act::Medicine: {
            drawPet(t < 800 ? *d.idle : *d.happy, t, petX, 7, now);
            tinyHeart(C_SICK);
            break;
        }
        case Act::Pet:
            drawPet(*d.happy, t, petX, 7, now);
            if ((t / 300) % 2 == 0) tinyHeart();
            break;
        case Act::Refuse:
            drawPet(*d.sad, t, petX + (((t / 120) % 2) ? 1 : -1), 7, now);
            break;
        case Act::Flee: {
            int target = petX <= 3 ? maxCx() : minCx();
            stepToward(target, now, 80);
            drawPet(*d.walk, t, petX, 7, now);
            break;
        }
        case Act::Forage:
            drawPet(*d.eat, t, petX, 7, now);
            if (t < actDur / 2) cv.set(clampX(petX + 2), 7, {200, 200, 200});
            break;
        case Act::Wild:
        case Act::Grumpy:
            drawPet(*d.sad, t, petX, 7, now);
            break;
        default: break;
    }
}

// Entrada no sonho: olhos fechados, bolhinhas saem da cabeça, a bolha cresce
// e o mundo de Conway substitui o pet pixel a pixel.
constexpr uint16_t BUBBLE_GROW_MS = 900; // antes disso, só as bolhinhas
void drawSleepingDream(uint32_t now) {
    const uint32_t elapsed = now - dreamViewAt;
    if (elapsed >= DREAM_BUBBLE_MS) { drawDreamWorld(now); return; }
    const Sprite &sleeping = frame(*def().sleep, now);
    drawPet(*def().sleep, now, petX, 7, now);
    const int dir = faceRight ? 1 : -1;
    const int headTop = MATRIX_H - sleeping.h;
    const int bx = petX + dir * 2 < 1 ? 1 : (petX + dir * 2 > MATRIX_W - 2 ? MATRIX_W - 2 : petX + dir * 2);
    const int by = headTop > 3 ? 1 : 0;
    const Rgb ring{120, 150, 210};
    if (elapsed < BUBBLE_GROW_MS) {
        nightFrame = true; // ainda é o pet dormindo, na luz de dormir
        setFree(petX + dir, headTop - 1, ring);
        if (elapsed >= 300) setFree(bx - dir, by + 1, ring);
        if (elapsed >= 600) setFree(bx, by, dreamBright);
        return;
    }
    // A bolha cresce do alto da cabeça até cobrir a matriz inteira.
    const Canvas pet = cv;
    cv.clear();
    drawDreamWorld(now);
    const Canvas world = cv;
    const int r = 1 + (int)(elapsed - BUBBLE_GROW_MS) * 10 / (DREAM_BUBBLE_MS - BUBBLE_GROW_MS); // 1..10
    const int r2 = r * r, inner2 = (r - 1) * (r - 1);
    for (int y = 0; y < MATRIX_H; ++y) {
        for (int x = 0; x < MATRIX_W; ++x) {
            const int d2 = (x - bx) * (x - bx) + (y - by) * (y - by);
            const Rgb w = world.get(x, y), p = pet.get(x, y);
            const bool worldLit = w.r || w.g || w.b;
            if (d2 < inner2) cv.set(x, y, w);                   // dentro: o sonho
            else if (d2 <= r2) cv.set(x, y, worldLit ? w : ring); // borda da bolha
            else cv.set(x, y, dim(p, 110));                     // fora: pet dormindo
        }
    }
}

void drawLife(uint32_t now) {
    const PetDef &d = def();
    const PetState &s = sim.s();
    uint32_t t = now - behAt;

    if (s.asleep) {
        if (dreamView) {
            drawSleepingDream(now);
            return;
        }
        drawPet(*d.sleep, now, petX, 7, now);
        drawPoop(now);
        drawRising(SPR_fx_z, 0, now, 2400, 0);
        nightFrame = true; // luz apagada: Display manda tudo no brilho mínimo
        return;
    }
    if (act != Act::None) {
        drawAction(now);
        if (act != Act::Clean) drawPoop(now);
        return;
    }

    // Visita acordada: o pet para e olha para o que passa.
    if (dreamView && def().side) faceRight = ambientFocusX(now) >= petX;

    if (s.sick) {
        drawPet(*d.sad, now, petX, 7, now);
    } else if (s.hunger < NEED_LOW) {
        drawPet(*d.hungry, now, petX, 7, now);
    } else if (s.energy < NEED_LOW) {
        drawPet(*d.tired, now, petX, 7, now); // cabeceando: ou você põe pra dormir, ou ele cochila
    } else if (s.happy < NEED_LOW) {
        drawPet(*d.sad, now, petX, 7, now);
    } else {
        switch (dreamView ? Beh::Stand : beh) {
            case Beh::Walk: drawPet(*d.walk, t, petX, 7, now); break;
            case Beh::Sniff: {
                // Fareja um ponto no chão à frente do focinho.
                drawPet((t / 700) % 3 == 2 ? *d.idle : *d.eat, t, petX, 7, now);
                int sx, sy;
                if ((t / 450) % 2 == 0 && sniffSpot(sx, sy)) cv.set(sx, sy, {130, 175, 100});
                break;
            }
            case Beh::Look:
                if (!dreamView) faceRight = ((t / 900) + sim.dna().bits) % 2;
                drawPet(*d.idle, t, petX, 7, now);
                break;
            case Beh::Watch:
                // Olhar atento: pisca pouco enquanto a coisa passa.
                drawPet(*d.idle, t, petX, 7, now);
                setFree(bugX, bugY, {205, 228, 255});
                break;
            case Beh::Hop: drawPet(*d.happy, t, petX, 7 - (int)((t / 250) % 2), now); break;
            case Beh::Nap:
                if (t < NAP_SETTLE_MS) {
                    drawPet(*d.tired, t, petX, 7, now); // boceja e se ajeita
                } else {
                    drawPet(*d.sleep, t, petX, 7, now);
                    drawRising(SPR_fx_z, 0, t - NAP_SETTLE_MS, 2400, 0);
                }
                break;
            case Beh::Chase:
                drawPet(*d.walk, t, petX, 7, now);
                drawButterfly(bugX, bugY, now);
                break;
            default: {
                if (now >= blinkAt && now < blinkAt + 150) {
                    drawPet(*d.blink, 0, petX, 7, now);
                } else {
                    if (now >= blinkAt + 150) blinkAt = now + rndRange(2500, 5000);
                    drawPet(*d.idle, now, petX, 7, now);
                }
            }
        }
    }
    if (dreamView) drawAmbient(now); // máscara somente visual; o autômato permanece completo
    drawPoop(now);
    drawNeed(now);
}

// ============================================================ cenas: lógica + desenho
void sceneSelect(uint32_t now, Ev e) {
    const uint8_t count = speciesCount();
    if (selIdx >= count) selIdx = 0;
    if (e == Ev::None && !Input::heldMs()) tiltStep(selIdx, count);
    if (e == Ev::Short) selIdx = (selIdx + 1) % count;
    if (e == Ev::Long) {
        // "Semente" do DNA: MAC da placa + instante do clique + RNG de hardware.
        uint64_t mac = ESP.getEfuseMac();
        uint32_t seed = (uint32_t)mac ^ (uint32_t)(mac >> 32) ^ (now * 2654435761UL) ^ esp_random();
        sim.choose(selIdx, seed);
        Serial.printf("[Game] escolheu %s, dna=%08lx\n", petAt(selIdx).name, (unsigned long)seed);
        lastEggMs = now;
        go(Scene::Egg);
        return;
    }

    const PetDef &d = petAt(selIdx);
    int x0 = (MATRIX_W - count) / 2;
    for (uint8_t i = 0; i < count; i++) cv.set(x0 + i, 0, i == selIdx ? C_WHITE : C_DIM);
    const Sprite &f = frame(*d.idle, now);
    cv.blitAnchored(f, centerCx(f.w), 7, false);
}

// ============================================================ ovo: cor e eclosão
uint32_t lerpHex(uint32_t a, uint32_t b, float t) {
    auto ch = [&](int s) {
        int va = (a >> s) & 0xFF, vb = (b >> s) & 0xFF;
        return (uint32_t)(va + (vb - va) * t + 0.5f) << s;
    };
    return ch(16) | ch(8) | ch(0);
}

// Paleta do ovo "esquentando": a casca vai do creme pra cor do bicho que
// está dentro, e na reta final pulsa cada vez mais rápido. Ordem das cores
// nas paletas egg_*: [0]=nada, E casca, S pintas, H brilho, C rachadura.
uint32_t eggPal[5];
Look eggLook(float p, uint32_t now, float flash = 0) {
    const uint32_t *base = frame(*def().egg, 0).pal;
    for (int i = 0; i < 5; i++) eggPal[i] = base[i];
    eggPal[1] = lerpHex(base[1], base[2], p * 0.55f);
    if (p > 0.7f) {
        uint16_t period = p > 0.9f ? 500 : 1100;
        float phase = (float)(now % period) / period;
        float pulse = (phase < 0.5f ? phase : 1 - phase) * 2 * (p - 0.7f) / 0.3f;
        eggPal[1] = lerpHex(eggPal[1], 0xFFFFFF, pulse * 0.35f);
        eggPal[2] = lerpHex(eggPal[2], 0xFFFFFF, pulse * 0.25f);
    }
    if (flash > 0)
        for (int i = 1; i < 5; i++) eggPal[i] = lerpHex(eggPal[i], 0xFFFFFF, flash);
    Look l;
    l.pal = eggPal;
    return l;
}

// Desenha só um pedaço (sx0, sy0, w, h) de um sprite — pra quebrar a casca.
void blitPart(const Sprite &s, int x, int y, int sx0, int sy0, int w, int h, const Look &look) {
    const uint32_t *pal = look.pal ? look.pal : s.pal;
    for (int sy = sy0; sy < sy0 + h && sy < s.h; sy++)
        for (int sx = sx0; sx < sx0 + w && sx < s.w; sx++)
            if (uint8_t idx = s.px[sy * s.w + sx]) cv.set(x + sx - sx0, y + sy - sy0, rgb(pal[idx]));
}

void sceneEgg(uint32_t now, Ev e) {
    uint32_t dt = now - lastEggMs;
    lastEggMs = now;
    bool moving = Imu::lastMotionMs() != 0 && now - Imu::lastMotionMs() < INCUBATION_IDLE_GRACE_MS;
    if (moving) sim.addIncubation(dt);
    if (e == Ev::Short || e == Ev::Long) progressUntil = now + 2000;

    float p = sim.incubationProgress();
    if (p >= 1.0f) {
        go(Scene::Hatch);
        return;
    }
    static const Sprite *const STAGES[] = {&SPR_egg0, &SPR_egg1, &SPR_egg2, &SPR_egg3};
    int stage = (int)(p * 4);
    stage = stage > 3 ? 3 : stage;
    bool wobbling = now - Imu::lastMotionMs() < 400;
    int off = wobbling ? (((now / 90) % 2) ? 1 : -1) : 0;
    cv.blitAnchored(*STAGES[stage], centerCx(STAGES[stage]->w) + off, 7, false, eggLook(p, now));

    if (now - Imu::lastMotionMs() > 20000 && (now / 400) % 2) {
        cv.blit(SPR_fx_arrow_l, 0, 0); // "me chacoalha!"
        cv.blit(SPR_fx_arrow_r, MATRIX_W - 2, 0);
    }
    if (now < progressUntil) {
        int n = (int)(p * MATRIX_W + 0.5f);
        for (int x = 0; x < MATRIX_W; x++) cv.set(x, 0, x < n ? Rgb{255, 200, 40} : C_DIM);
    }
}

void sparkles(uint32_t t, uint8_t count) {
    // brilhinhos que acendem e apagam em lugares "aleatórios" mas estáveis
    static const uint8_t spots[][2] = {{0, 1}, {7, 2}, {1, 4}, {6, 0}, {7, 5}, {0, 6}, {2, 0}, {5, 3}};
    for (uint8_t i = 0; i < count && i < 8; i++)
        if (((t / 130) + i * 3) % 5 < 2) cv.set(spots[i][0], spots[i][1], {255, 240, 150});
}

void sceneHatch(uint32_t now) {
    uint32_t t = now - sceneAt;
    const PetDef &d = def();
    const Sprite &egg = SPR_egg3;
    const int ex = centerCx(egg.w) - egg.w / 2, ey = MATRIX_H - egg.h; // canto do ovo
    petX = centerCx(idleW());
    char name[16];
    sim.dna().name(name, sizeof(name));
    const uint32_t nameMs = scrollMs(name);

    if (t < 1600) {
        // 1) tremendo cada vez mais rápido e brilhando
        uint32_t period = 200 - t / 10;
        int off = ((t / period) % 2) ? 1 : -1;
        cv.blit(egg, ex + off, ey, false, eggLook(1.0f, now));
    } else if (t < 2000) {
        // 2) clarão
        float flash = 1.0f - (float)(t - 1600) / 400 * 0.5f;
        cv.blit(egg, ex, ey, false, eggLook(1.0f, now, flash));
    } else if (t < 2900) {
        // 3) a tampa voa pra cima e o bicho espia de dentro da casca
        uint32_t k = t - 2000;
        Look shell = eggLook(1.0f, now);
        int rise = k < 600 ? 2 - (int)(k / 300) : 0; // 2 -> 0: vai saindo
        drawPet(*d.idle, t, petX, 7 + rise, now);
        blitPart(egg, ex, ey + 3, 0, 3, egg.w, egg.h - 3, shell);     // metade de baixo, na frente
        blitPart(egg, ex, ey - (int)(k / 110), 0, 0, egg.w, 3, shell); // tampa subindo
        sparkles(t, 2);
    } else if (t < 3500) {
        // 4) a casca se abre pros lados
        int k = (t - 2900) / 150;
        Look shell = eggLook(1.0f, now);
        drawPet(*d.happy, t, petX, 7, now);
        blitPart(egg, ex - k, ey + 3 + k / 2, 0, 3, 3, egg.h - 3, shell);
        blitPart(egg, ex + 3 + k, ey + 3 + k / 2, 3, 3, 3, egg.h - 3, shell);
        sparkles(t, 4);
    } else if (t < 5200) {
        // 5) comemoração
        drawPet(*d.happy, t, petX, 7 - (int)((t / 250) % 2), now);
        sparkles(t, 8);
    } else if (t < 5200 + nameMs) {
        // 6) se apresenta: o nome próprio passa rolando na cor dele
        scrollText(name, t - 5200, 1, petColor());
        sparkles(t, 3);
    } else {
        sim.hatch();
        petX = centerCx(idleW());
        beh = Beh::Stand;
        behUntil = now + 2000;
        go(Scene::Life);
    }
}

void doMenu(uint8_t item, uint32_t now) {
    go(Scene::Life);
    act = Act::None;
    Result r = Result::Refused;
    switch (item) {
        case 0:
            r = sim.feed(); // ganho fixo por refeição; a animação não muda a quantidade
            if (r == Result::Ok) {
                planMeal();
                startAct(Act::Eat, EAT_MS);
            }
            break;
        case 1:
            r = sim.play();
            if (r == Result::Ok) startAct(Act::Play, 3000);
            break;
        case 2:
            r = sim.clean();
            if (r == Result::Ok) startAct(Act::Clean, 1600);
            break;
        case 3:
            r = sim.medicine();
            if (r == Result::Ok) startAct(Act::Medicine, 2000);
            break;
        case 4:
            gestureSleep = false;
            if (sim.s().asleep) {
                bool grumpy = sim.s().energy < 50;
                r = sim.wakeUp();
                if (grumpy) startAct(Act::Grumpy, 1000);
            } else {
                r = sim.lightsOff();
            }
            break;
        case 5:
            r = sim.pet();
            if (r == Result::Ok) startAct(Act::Pet, 1600);
            break;
        case 6:
            statusPage = 0;
            go(Scene::Status);
            return;
        case 7: return;
    }
    if (r == Result::Refused && !sim.s().asleep) startAct(Act::Refuse, 900);
    (void)now;
}

void sceneLife(uint32_t now, Ev e) {
    uint8_t n = sim.takeNotices();
    if (n & N_DIED) {
        go(Scene::Dead);
        return;
    }
    if (n & N_WENT_WILD) startAct(Act::Wild, 2400);
    else if ((n & N_FORAGED) && act == Act::None) startAct(Act::Forage, 1800);
    if (n) Serial.printf("[Game] aviso 0x%02x\n", n);

    const PetState &s = sim.s();
    if (act != Act::None && now - actAt >= actDur) act = Act::None;
    if (!s.asleep) gestureSleep = false;
    switch (e) {
        case Ev::Short:
            if (s.asleep) {
                gestureSleep = false;
                doMenu(4, now); // um clique acorda, sem alimentar junto
            } else if (act == Act::None) doMenu(0, now);
            break;
        case Ev::Long:
            menuIdx = suggestedCare();
            tiltArmed = false;
            menuAt = now;
            go(Scene::Menu);
            return;
        case Ev::Shake:
            if (s.asleep || act != Act::None || Input::heldMs()) break;
            if (s.wild) {
                startAct(Act::Flee, 1000);
            } else if (!playedOnce || now - lastPlayAt >= PLAY_COOLDOWN_MS) {
                lastPlayAt = now;
                playedOnce = true;
                doMenu(1, now);
            }
            break;
        case Ev::FaceDown:
            if (!Input::heldMs() && !s.asleep) {
                gestureSleep = sim.lightsOff() == Result::Ok;
                act = Act::None;
            }
            break;
        case Ev::FaceUp:
            if (gestureSleep && s.asleep) sim.wakeUp();
            gestureSleep = false;
            break;
        default: break;
    }

    // Dorme por conta própria (e recupera até acordar cheio): logo, se estiver
    // cansado; depois de alguns minutos sozinho, se a energia já estiver baixando.
    const uint32_t alone = now - lastInputAt;
    const bool tired = s.energy < NEED_LOW && alone > AUTO_SLEEP_IDLE_MS;
    const bool drowsy = s.energy < NAP_ENERGY && alone > NAP_IDLE_MS;
    if (!s.asleep && act == Act::None && (tired || drowsy) && sim.lightsOff() == Result::Ok) {
        gestureSleep = false;
        Serial.println(tired ? "[Game] cansado e sozinho: dormiu" : "[Game] sozinho: cochilou");
    }

    // Durante a visita de Conway ele para e observa; cuidados continuam por cima.
    if (act == Act::None && !s.asleep && !dreamView) updateBehavior(now);
    drawLife(now);
}

// Os quatro atributos do status, na ordem das páginas (e das barrinhas do ícone).
struct StatusPage { const Sprite *icon; const char *label; uint8_t v; Rgb c; };
StatusPage statusPageInfo(uint8_t page) {
    const PetState &s = sim.s();
    switch (page) {
        case 0: return {&SPR_stat_hunger, "COMIDA", s.hunger, C_HUNGER};
        case 1: return {&SPR_stat_happy, "ALEGRIA", s.happy, C_HAPPY};
        case 2: return {&SPR_stat_energy, "ENERGIA", s.energy, C_ENERGY};
        default: return {&SPR_stat_care, s.sick ? "DOENTE" : s.poop ? "SUJO" : "SAUDE", sim.care(), C_CARE};
    }
}

// Ícone do status "ao vivo": uma barrinha por atributo, na cor da página dele
// (comida, alegria, energia, saúde) e com a altura do valor atual.
void drawStatusIcon() {
    for (uint8_t i = 0; i < 4; i++) {
        const StatusPage p = statusPageInfo(i);
        int h = (p.v * 7 + 99) / 100; // 1..7 linhas; zero fica apagado
        for (int y = 0; y < 7; y++) cv.set(1 + i * 2, y, 6 - y < h ? p.c : Rgb{0, 0, 0});
    }
}

void sceneMenu(uint32_t now, Ev e) {
    if (e == Ev::None && !Input::heldMs() && tiltStep(menuIdx, MENU_COUNT)) menuAt = now;
    if (e == Ev::Short) {
        menuIdx = (menuIdx + 1) % MENU_COUNT;
        menuAt = now;
    } else if (e == Ev::Long) {
        doMenu(menuIdx, now);
        return;
    } else if (!Input::heldMs() && now - menuAt > MENU_TIMEOUT_MS) {
        go(Scene::Life);
        return;
    }
    const Sprite &icon = *MENU_ICONS[menuIdx];
    if (menuIdx == MENU_STATUS) drawStatusIcon();
    else cv.blit(icon, (MATRIX_W - icon.w) / 2, (MATRIX_H - 1 - icon.h) / 2);
    for (uint8_t i = 0; i < MENU_COUNT; i++) cv.set(i, 7, i == menuIdx ? C_WHITE : C_DIM);
}

// Status em páginas: primeiro o nome do bicho; depois uma página por atributo,
// com barra de 8 LEDs embaixo e, em cima, o ícone seguido do nome rolando
// (COMIDA, ALEGRIA, ENERGIA, SAUDE — ou DOENTE/SUJO); por fim a idade.
// Passa sozinho; clique adianta, segurar sai.
constexpr uint8_t STATUS_BARS = 4;
constexpr uint8_t STATUS_NAME_PAGE = 0, STATUS_AGE_PAGE = STATUS_BARS + 1;
constexpr uint16_t STATUS_ICON_MS = 1000;

uint32_t statusBarMs(const char *label) {
    return STATUS_ICON_MS + scrollMs(label);
}

// Texto das páginas de nome/idade; devolve quanto dura uma passada dele.
uint32_t statusText(uint8_t page, char *buf, size_t n) {
    if (page == STATUS_NAME_PAGE) {
        sim.dna().name(buf, n);
    } else {
        uint16_t days = sim.ageDays();
        snprintf(buf, n, "%u %s%s", days, days == 1 ? "DIA" : "DIAS", sim.s().wild ? " SELVAGEM" : "");
    }
    return scrollMs(buf);
}

bool statusIsBar(uint8_t page) { return page > STATUS_NAME_PAGE && page < STATUS_AGE_PAGE; }

void sceneStatus(uint32_t now, Ev e) {
    uint32_t t = now - sceneAt;
    char buf[32];
    uint32_t pageMs = statusIsBar(statusPage) ? statusBarMs(statusPageInfo(statusPage - 1).label)
                                              : statusText(statusPage, buf, sizeof(buf));
    if (e == Ev::Long) {
        go(Scene::Life);
        return;
    }
    if (e == Ev::Short || (!Input::heldMs() && t > pageMs)) {
        if (++statusPage > STATUS_AGE_PAGE) {
            go(Scene::Life);
            return;
        }
        sceneAt = now;
        t = 0;
    }
    const PetState &s = sim.s();
    if (statusIsBar(statusPage)) {
        const StatusPage p = statusPageInfo(statusPage - 1);
        // Atributo baixo: o ícone pisca pedindo atenção. Depois entra o nome.
        if (t >= STATUS_ICON_MS) scrollText(p.label, t - STATUS_ICON_MS, 0, p.c);
        else if (p.v >= NEED_LOW || (now / 300) % 2) cv.blit(*p.icon, (MATRIX_W - p.icon->w) / 2, 0);
        // A barra "enche" na entrada da página, como um medidor.
        int full = (p.v * MATRIX_W + 50) / 100;
        int len = t < 400 ? full * (int)t / 400 : full;
        for (int x = 0; x < MATRIX_W; x++)
            for (int y = 6; y < MATRIX_H; y++) cv.set(x, y, x < len ? p.c : dim(p.c, 30));
    } else if (statusPage == STATUS_NAME_PAGE) {
        statusText(statusPage, buf, sizeof(buf));
        scrollText(buf, t, 1, s.wild ? Rgb{200, 160, 90} : petColor());
    } else {
        statusText(statusPage, buf, sizeof(buf));
        scrollText(buf, t, 1, s.wild ? Rgb{200, 160, 90} : C_WHITE);
    }
}

void sceneDead(uint32_t now, Ev e) {
    if (e == Ev::Long) {
        sim.restart();
        go(Scene::Select);
        return;
    }
    // Lápide com fantasminha por 6 s, depois "RIP <nome>" passa uma vez.
    char name[16], buf[24];
    sim.dna().name(name, sizeof(name));
    snprintf(buf, sizeof(buf), "RIP %s", name);
    uint32_t p = (now - sceneAt) % (6000 + scrollMs(buf));
    if (p < 6000) {
        cv.blit(SPR_grave, 0, 3);
        int bob = ((now / 600) % 2) ? 1 : 0;
        cv.blit(frame(ANIM_ghost, now), 3, bob);
    } else {
        scrollText(buf, p - 6000, 1, {200, 210, 230});
    }
}

void drawResetBar() {
    uint32_t held = Input::heldMs();
    if (held >= BUTTON_LONG_MS && held < BUTTON_RESET_SHOW_MS) {
        // Pequena confirmação no canto livre: já pode soltar o BOOT.
        cv.set(7, 0, {62, 214, 76});
    }
    if (held < BUTTON_RESET_SHOW_MS) return;
    cv.scale(80);
    int n = (int)((held - BUTTON_RESET_SHOW_MS) * MATRIX_W / (BUTTON_RESET_MS - BUTTON_RESET_SHOW_MS));
    for (int x = 0; x < MATRIX_W; x++) cv.set(x, MATRIX_H - 1, x < n ? C_RESET : Rgb{50, 8, 8});
}

void enterFromSaved() {
    switch (sim.s().phase) {
        case Phase::Select: go(Scene::Select); break;
        case Phase::Egg: go(Scene::Egg); break;
        case Phase::Alive:
            petX = centerCx(idleW());
            go(Scene::Life);
            break;
        case Phase::Dead: go(Scene::Dead); break;
    }
}

uint32_t lastFrame = 0;
uint32_t lastLog = 0;
uint32_t swatch[4];
uint32_t swatchUntil = 0;

} // namespace

namespace Game {

void begin() {
    CustomPet::begin();
    sim.begin();
    lastInputAt = millis();
    observedMotionAt = Imu::lastMotionMs();
    sleepWasActive = false;
    dreamSeeded = false;
    dreamView = false;
    lastEggMs = millis();
    enterFromSaved();
    const PetState &s = sim.s();
    char name[16];
    sim.dna().name(name, sizeof(name));
    Serial.printf("[Game] fase=%d bicho=%s nome=%s dna=%08lx fome=%u alegria=%u energia=%u idade=%lu min\n",
                  (int)s.phase, def().name, name, (unsigned long)s.dna, s.hunger, s.happy, s.energy,
                  (unsigned long)s.ageMin);
}

void update() {
    uint32_t now = millis();
    sim.update();

    Ev e = Events::pop();
    bool activity = e != Ev::None;
    const uint32_t motionAt = Imu::lastMotionMs();
    if (motionAt != observedMotionAt) {
        observedMotionAt = motionAt;
        activity = true;
    }
    if (activity) {
        lastInputAt = now;
        if (scene == Scene::Life && !sim.s().asleep) reactToInput(now);
    }
    updateDreamView(now, activity);
    // O gesto de desvirar também funciona enquanto um menu está aberto.
    if (e == Ev::FaceUp && gestureSleep) {
        if (sim.s().asleep) sim.wakeUp();
        gestureSleep = false;
    }
    if (e == Ev::Reset) {
        Serial.println("[Game] reset: recomecando do zero");
        sim.restart();
        act = Act::None;
        gestureSleep = false;
        playedOnce = false;
        selIdx = 0;
        tiltArmed = false;
        progressUntil = 0;
        Events::clear();
        go(Scene::Select);
        return;
    }

    if (now - lastFrame < FRAME_MS && e == Ev::None) return;
    lastFrame = now;

    cv.clear();
    if ((int32_t)(swatchUntil - now) > 0) {
        for (int y = 0; y < MATRIX_H; y++)
            for (int x = 0; x < MATRIX_W; x++) cv.set(x, y, rgb(swatch[(y >= 4) * 2 + (x >= 4)]));
        Display::show(cv, false);
        return;
    }
    nightFrame = false;
    switch (scene) {
        case Scene::Select: sceneSelect(now, e); break;
        case Scene::Egg: sceneEgg(now, e); break;
        case Scene::Hatch: sceneHatch(now); break;
        case Scene::Life: sceneLife(now, e); break;
        case Scene::Menu: sceneMenu(now, e); break;
        case Scene::Status: sceneStatus(now, e); break;
        case Scene::Dead: sceneDead(now, e); break;
    }
    drawResetBar();
    Display::show(cv, nightFrame && Input::heldMs() < BUTTON_RESET_SHOW_MS);

    if (now - lastLog > 30000 && sim.s().phase == Phase::Alive) {
        lastLog = now;
        const PetState &s = sim.s();
        Serial.printf("[Game] fome=%u alegria=%u energia=%u coco=%u doente=%d dormindo=%d descuido=%u selvagem=%d idade=%lumin\n",
                      s.hunger, s.happy, s.energy, s.poop, s.sick, s.asleep, s.neglect, s.wild,
                      (unsigned long)s.ageMin);
    }
}

bool customPetActive() {
    return sim.s().phase != Phase::Select && sim.s().species == PET_COUNT && CustomPet::available();
}

// Arte nova do bichinho do editor: se ele é o pet atual, a troca aparece na
// hora; se o pacote foi apagado, o pet dele não tem mais desenho e recomeça.
void customPetChanged() {
    const PetState &s = sim.s();
    if (s.phase == Phase::Select || s.species != PET_COUNT) return;
    if (!CustomPet::available()) {
        sim.restart();
        act = Act::None;
        selIdx = 0;
        go(Scene::Select);
        return;
    }
    act = Act::None;
    petX = centerCx(idleW());
    beh = Beh::Stand;
    behUntil = millis() + 2000;
}

// Troca o bichinho atual por um ovo do bichinho do editor.
void adoptCustomPet() {
    if (!CustomPet::available()) return;
    const uint32_t now = millis();
    const uint64_t mac = ESP.getEfuseMac();
    const uint32_t seed = (uint32_t)mac ^ (uint32_t)(mac >> 32) ^ (now * 2654435761UL) ^ esp_random();
    sim.choose(PET_COUNT, seed);
    act = Act::None;
    gestureSleep = false;
    lastEggMs = now;
    progressUntil = 0;
    go(Scene::Egg);
}

void showSwatches(const uint32_t colors[4], uint32_t ms) {
    memcpy(swatch, colors, sizeof(swatch));
    swatchUntil = millis() + ms;
}

} // namespace Game
