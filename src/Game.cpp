#include "Game.h"
#include "Canvas.h"
#include "Config.h"
#include "Display.h"
#include "Events.h"
#include "Imu.h"
#include "Input.h"
#include "PetSim.h"
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

const PetDef &def() { return PETS[sim.s().species % PET_COUNT]; }

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
enum class Beh : uint8_t { Stand, Walk, Sniff, Look, Hop, Nap, Chase, COUNT };
Beh beh = Beh::Stand;
uint32_t behAt = 0, behUntil = 0;
int petX = 3, targetX = 3;
bool faceRight = true;
uint32_t lastStep = 0;
uint32_t blinkAt = 0;
int bugX = 0, bugY = 1;
uint32_t bugStep = 0;

// ---- vida: ações com animação própria
enum class Act : uint8_t { None, Eat, Play, Clean, Medicine, Pet, Refuse, Flee, Forage, Wild, Grumpy };
Act act = Act::None;
uint32_t actAt = 0, actDur = 0;
uint32_t lastPlayAt = 0;
bool playedOnce = false;
bool gestureSleep = false;
bool nightFrame = false; // quadro atual é de "luz apagada" (brilho mínimo)
uint32_t lastInputAt = 0; // último clique/gesto (cansado + muito tempo sem isso = dorme)

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
    w[(int)Beh::Hop] = s.happy > 60 ? 3 + d.activity() / 32 : 0;
    w[(int)Beh::Nap] = s.energy < 40 ? 20 : 3;
    w[(int)Beh::Chase] = s.energy > 40 ? 3 + d.activity() / 32 : 0;
    // Capivara tranquila; gato curioso e caçador. O DNA ainda varia cada pet.
    if (strcmp(def().id, "capy") == 0) {
        w[(int)Beh::Stand] += 20;
        w[(int)Beh::Sniff] += 16;
        w[(int)Beh::Nap] += 8;
        w[(int)Beh::Chase] = 0;
        w[(int)Beh::Hop] /= 2;
    } else if (strcmp(def().id, "cat") == 0) {
        w[(int)Beh::Look] += 12;
        w[(int)Beh::Chase] += s.energy > 40 ? 14 : 0;
    }
    if (beh == Beh::Walk) w[(int)Beh::Stand] *= 2;
    if (beh == Beh::Stand) w[(int)Beh::Walk] = w[(int)Beh::Walk] * 3 / 2;
    if (beh != Beh::Stand && beh != Beh::Walk) w[(int)beh] = 0; // não repete o "especial"
    if (s.wild) {
        w[(int)Beh::Look] *= 3;
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
        case Beh::Stand: behUntil = now + rndRange(2000, 5000); break;
        case Beh::Sniff: behUntil = now + rndRange(1800, 3000); break;
        case Beh::Look: behUntil = now + rndRange(1500, 3000); break;
        case Beh::Hop: behUntil = now + 1500; break;
        case Beh::Nap: behUntil = now + rndRange(5000, 9000); break;
        case Beh::Chase:
            bugX = rnd(MATRIX_W);
            bugY = rndRange(0, 2);
            behUntil = now + 6000;
            break;
        default: break;
    }
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
void drawPoop(uint32_t now, int sweepX = -1) {
    static const int8_t slots[POOP_MAX] = {5, 0, 3};
    for (uint8_t i = 0; i < sim.s().poop && i < POOP_MAX; i++) {
        int x = slots[i];
        if (sweepX >= 0 && x <= sweepX) continue;
        cv.blit(SPR_fx_poop, x, MATRIX_H - SPR_fx_poop.h);
        if ((now / 400 + i) % 3 == 0) cv.set(x + 1, MATRIX_H - SPR_fx_poop.h - 2, {70, 90, 60}); // "cheirinho"
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

void drawAction(uint32_t now) {
    const PetDef &d = def();
    uint32_t t = now - actAt;
    switch (act) {
        case Act::Eat: {
            const Sprite &f = *d.food;
            petX = centerCx(idleW());
            faceRight = true;
            if (t < 600) {
                cv.blit(f, (MATRIX_W - f.w) / 2, (MATRIX_H - f.h) / 2);
            } else if (t < actDur - 500) {
                drawPet(*d.eat, t, petX, 7, now);
            } else {
                cv.blit(SPR_fx_heart, 2, 2);
            }
            break;
        }
        case Act::Play: {
            petX = centerCx(idleW());
            if (t < 650) cv.blit(SPR_fx_ball, 1 + (t / 130) % 5, 2);
            else if (t < actDur - 450) drawPet(*d.happy, t, petX, 7 - (int)((t / 350) % 2), now);
            else cv.blit(SPR_fx_heart, 2, 2);
            break;
        }
        case Act::Clean: {
            drawPet(*d.idle, t, petX, 7, now);
            int sweep = (int)(t / 140) - 1;
            drawPoop(now, sweep);
            cv.blit(frame(ANIM_fx_sparkle_anim, t), sweep - 1, MATRIX_H - 3);
            break;
        }
        case Act::Medicine: {
            if (t < 800) {
                cv.blit(SPR_fx_pill, 2, 3);
            } else {
                drawPet(*d.happy, t, petX, 7, now);
            }
            break;
        }
        case Act::Pet:
            if (t < 450) cv.blit(SPR_fx_heart, 2, 2);
            else drawPet(*d.happy, t, petX, 7, now);
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

void drawLife(uint32_t now) {
    const PetDef &d = def();
    const PetState &s = sim.s();
    uint32_t t = now - behAt;

    if (s.asleep) {
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

    if (s.sick) {
        drawPet(*d.sad, now, petX, 7, now);
    } else if (s.hunger < NEED_LOW) {
        drawPet(*d.hungry, now, petX, 7, now);
    } else if (s.energy < NEED_LOW) {
        drawPet(*d.tired, now, petX, 7, now); // cabeceando: ou você põe pra dormir, ou ele cochila
    } else if (s.happy < NEED_LOW) {
        drawPet(*d.sad, now, petX, 7, now);
    } else {
        switch (beh) {
            case Beh::Walk: drawPet(*d.walk, t, petX, 7, now); break;
            case Beh::Sniff: drawPet(*d.eat, t, petX, 7, now); break;
            case Beh::Hop: drawPet(*d.happy, t, petX, 7 - (int)((t / 250) % 2), now); break;
            case Beh::Nap:
                drawPet(*d.sleep, t, petX, 7, now);
                drawRising(SPR_fx_z, 0, t, 2400, 0);
                break;
            case Beh::Chase:
                drawPet(*d.walk, t, petX, 7, now);
                if ((now / 150) % 3) cv.set(bugX, bugY, C_WHITE);
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
    drawPoop(now);
    drawNeed(now);
}

// ============================================================ cenas: lógica + desenho
void sceneSelect(uint32_t now, Ev e) {
    if (e == Ev::None && !Input::heldMs()) tiltStep(selIdx, PET_COUNT);
    if (e == Ev::Short) selIdx = (selIdx + 1) % PET_COUNT;
    if (e == Ev::Long) {
        // "Semente" do DNA: MAC da placa + instante do clique + RNG de hardware.
        uint64_t mac = ESP.getEfuseMac();
        uint32_t seed = (uint32_t)mac ^ (uint32_t)(mac >> 32) ^ (now * 2654435761UL) ^ esp_random();
        sim.choose(selIdx, seed);
        Serial.printf("[Game] escolheu %s, dna=%08lx\n", PETS[selIdx].name, (unsigned long)seed);
        lastEggMs = now;
        go(Scene::Egg);
        return;
    }

    const PetDef &d = PETS[selIdx];
    int x0 = (MATRIX_W - PET_COUNT) / 2;
    for (uint8_t i = 0; i < PET_COUNT; i++) cv.set(x0 + i, 0, i == selIdx ? C_WHITE : C_DIM);
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
            r = sim.feed();
            if (r == Result::Ok) startAct(Act::Eat, 3000);
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

    // Cansado e sozinho: dorme por conta própria (e recupera até acordar cheio).
    if (!s.asleep && act == Act::None && s.energy < NEED_LOW && now - lastInputAt > AUTO_SLEEP_IDLE_MS &&
        sim.lightsOff() == Result::Ok) {
        gestureSleep = false;
        Serial.println("[Game] cansado e sozinho: dormiu");
    }

    if (act == Act::None && !s.asleep) updateBehavior(now);
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
    sim.begin();
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
    if (e != Ev::None) lastInputAt = now;
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

void showSwatches(const uint32_t colors[4], uint32_t ms) {
    memcpy(swatch, colors, sizeof(swatch));
    swatchUntil = millis() + ms;
}

} // namespace Game
