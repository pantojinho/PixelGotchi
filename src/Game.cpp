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

using namespace Art;

namespace {

PetSim sim;
Canvas cv;

// ============================================================ utilidades
uint32_t rnd(uint32_t n) { return n ? esp_random() % n : 0; }
uint32_t rndRange(uint32_t a, uint32_t b) { return a + rnd(b - a + 1); }

const Rgb C_WHITE{255, 255, 255};
const Rgb C_DIM{40, 40, 52};
const Rgb C_BUBBLE{150, 150, 175};
const Rgb C_RESET{255, 40, 40};

const Sprite &frame(const Anim &a, uint32_t t) { return frameAt(a, t); }

// cx que centraliza um sprite de largura w na matriz.
int centerCx(int w) { return (MATRIX_W - w) / 2 + w / 2; }

const PetDef &def() { return PETS[sim.s().species % PET_COUNT]; }

void scrollText(const char *str, uint32_t t, int y, Rgb color, uint16_t stepMs = 100) {
    int w = Canvas::textWidth(str);
    int span = w + MATRIX_W + 2;
    cv.text(str, MATRIX_W - (int)((t / stepMs) % span), y, color);
}

void upperName(const char *src, char *dst, size_t n) {
    size_t i = 0;
    for (; src[i] && i + 1 < n; i++) dst[i] = (src[i] >= 'a' && src[i] <= 'z') ? src[i] - 32 : src[i];
    dst[i] = 0;
}

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
uint32_t lastPetAt = 0;

// ---- menu / status
const Sprite *const MENU_ICONS[] = {&SPR_icon_food, &SPR_icon_play, &SPR_icon_clean,
                                    &SPR_icon_medicine, &SPR_icon_sleep, &SPR_icon_status};
constexpr uint8_t MENU_COUNT = sizeof(MENU_ICONS) / sizeof(MENU_ICONS[0]);
uint8_t menuIdx = 0;
uint32_t menuAt = 0;
uint8_t statusPage = 0;

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

void drawHungerBubble(uint32_t now) {
    if ((now / 400) % 5 == 0) return; // pisca de leve
    const Sprite &f = *def().food;
    int fx = MATRIX_W - f.w;
    cv.blit(f, fx, 0);
    cv.set(fx - 1, f.h, C_BUBBLE);
}

void drawAlert(uint32_t now) {
    if (sim.needsAttention() && (now / 500) % 2) cv.blit(SPR_fx_alert, 0, 0);
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
            uint16_t biteMs = 450;
            uint32_t eatMs = f.w * biteMs + 300;
            petX = minCx();
            faceRight = true;
            if (t < eatMs) {
                drawPet(*d.eat, t, petX, 7, now);
                int fx = MATRIX_W - f.w, fy = MATRIX_H - f.h;
                cv.blit(f, fx, fy);
                int bitten = t / biteMs;
                for (int c = 0; c < bitten && c < f.w; c++)
                    for (int y = fy; y < MATRIX_H; y++) cv.set(fx + c, y, {0, 0, 0});
            } else {
                drawPet(*d.happy, t, petX, 7, now);
                drawRising(SPR_fx_heart, MATRIX_W - 3, t - eatMs, 800);
            }
            break;
        }
        case Act::Play: {
            petX = centerCx(idleW());
            drawPet(*d.happy, t, petX, 7 - (int)((t / 250) % 2), now);
            int bx = 1 + (int)((t / 160) % 12);
            bx = bx > 6 ? 12 - bx : bx;
            int ph = (t / 90) % 8;
            int by = ph < 4 ? ph : 7 - ph;
            cv.blit(SPR_fx_ball, bx, by - 1);
            if (t > actDur - 800) drawRising(SPR_fx_heart, 5, t, 800);
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
            drawPet(*d.sad, t, petX, 7, now);
            if (t < 800) {
                int px = 6 - (int)(t * (6 - petX) / 800);
                int py = (int)(t * 4 / 800);
                cv.blit(SPR_fx_pill, px, py);
            } else {
                cv.blit(frame(ANIM_fx_sparkle_anim, t), petX - 1, 2);
            }
            break;
        }
        case Act::Pet:
            drawPet(*d.happy, t, petX, 7 - (int)((t / 250) % 2), now);
            drawRising(SPR_fx_heart, petX + 1, t, 750);
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
        drawRising(SPR_fx_z, clampX(petX) + 2, now, 2400, 3);
        cv.scale(NIGHT_DIM);
        return;
    }
    if (act != Act::None) {
        drawAction(now);
        if (act != Act::Clean) drawPoop(now);
        return;
    }

    if (s.sick) {
        drawPet(*d.sad, now, petX, 7, now);
        if ((now / 500) % 2) cv.blit(SPR_fx_sick, MATRIX_W - 3, 0);
    } else if (s.hunger < NEED_LOW) {
        drawPet(*d.hungry, now, petX, 7, now);
        drawHungerBubble(now);
    } else if (s.happy < NEED_LOW) {
        drawPet(*d.sad, now, petX, 7, now);
    } else {
        switch (beh) {
            case Beh::Walk: drawPet(*d.walk, t, petX, 7, now); break;
            case Beh::Sniff: drawPet(*d.eat, t, petX, 7, now); break;
            case Beh::Hop: drawPet(*d.happy, t, petX, 7 - (int)((t / 250) % 2), now); break;
            case Beh::Nap:
                drawPet(*d.sleep, t, petX, 7, now);
                drawRising(SPR_fx_z, clampX(petX) + 2, t, 2400, 3);
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
    drawAlert(now);
}

// ============================================================ cenas: lógica + desenho
void sceneSelect(uint32_t now, Ev e) {
    float tilt = Imu::tilt();
    if (tiltArmed && (tilt > TILT_STEP || tilt < -TILT_STEP)) {
        selIdx = (selIdx + (tilt > 0 ? 1 : PET_COUNT - 1)) % PET_COUNT;
        tiltArmed = false;
    } else if (tilt < TILT_REARM && tilt > -TILT_REARM) {
        tiltArmed = true;
    }
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

void sceneEgg(uint32_t now, Ev e) {
    uint32_t dt = now - lastEggMs;
    lastEggMs = now;
    bool moving = now - Imu::lastMotionMs() < INCUBATION_IDLE_GRACE_MS;
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
    Look l;
    l.pal = frame(*def().egg, 0).pal;
    bool wobbling = now - Imu::lastMotionMs() < 400;
    int off = wobbling ? (((now / 90) % 2) ? 1 : -1) : 0;
    cv.blitAnchored(*STAGES[stage], centerCx(STAGES[stage]->w) + off, 7, false, l);

    if (now - Imu::lastMotionMs() > 20000 && (now / 400) % 2) {
        cv.blit(SPR_fx_arrow_l, 0, 0); // "me chacoalha!"
        cv.blit(SPR_fx_arrow_r, MATRIX_W - 2, 0);
    }
    if (now < progressUntil) {
        int n = (int)(p * MATRIX_W + 0.5f);
        for (int x = 0; x < MATRIX_W; x++) cv.set(x, 0, x < n ? Rgb{255, 200, 40} : C_DIM);
    }
}

void sceneHatch(uint32_t now) {
    uint32_t t = now - sceneAt;
    const PetDef &d = def();
    Look egg;
    egg.pal = frame(*d.egg, 0).pal;
    if (t < 1200) {
        int off = ((t / 60) % 2) ? 1 : -1;
        cv.blitAnchored(SPR_egg3, centerCx(SPR_egg3.w) + off, 7, false, egg);
    } else if (t < 1700) {
        // casca estourando: pedaços voando pra fora + brilhos
        int k = (t - 1200) / 100;
        Rgb shell = rgb(egg.pal[1]);
        cv.set(3 - k, 4 - k, shell);
        cv.set(4 + k, 4 - k, shell);
        cv.set(2 - k, 6, shell);
        cv.set(5 + k, 6, shell);
        cv.blit(SPR_fx_sparkle, 2, 3);
    } else if (t < 3400) {
        petX = centerCx(idleW());
        drawPet(*d.happy, t, petX, 7 - (int)((t / 250) % 2), now);
        if ((t / 200) % 2) cv.set(0, 1, C_WHITE), cv.set(7, 2, C_WHITE);
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
    Result r = Result::Refused;
    switch (item) {
        case 0:
            r = sim.feed();
            if (r == Result::Ok) startAct(Act::Eat, def().food->w * 450 + 300 + 900);
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
            if (sim.s().asleep) {
                bool grumpy = sim.s().energy < 50;
                r = sim.wakeUp();
                if (grumpy) startAct(Act::Grumpy, 1000);
            } else {
                r = sim.lightsOff();
            }
            break;
        case 5:
            statusPage = 0;
            go(Scene::Status);
            return;
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
    switch (e) {
        case Ev::Short:
            menuAt = now;
            go(Scene::Menu);
            return;
        case Ev::Long:
            statusPage = 0;
            go(Scene::Status);
            return;
        case Ev::Shake:
            if (s.asleep) {
                bool grumpy = s.energy < 50;
                sim.wakeUp();
                if (grumpy) startAct(Act::Grumpy, 1000);
            } else if (s.wild) {
                startAct(Act::Flee, 1000);
            } else if (act == Act::None && now - lastPetAt > PET_COOLDOWN_MS) {
                lastPetAt = now;
                if (sim.pet() == Result::Ok) startAct(Act::Pet, 1500);
            }
            break;
        case Ev::FaceDown:
            if (sim.lightsOff() == Result::Refused && !s.asleep) startAct(Act::Refuse, 900);
            break;
        default: break;
    }

    if (act != Act::None && now - actAt >= actDur) act = Act::None;
    if (act == Act::None && !s.asleep) updateBehavior(now);
    drawLife(now);
}

void sceneMenu(uint32_t now, Ev e) {
    if (e == Ev::Short) {
        menuIdx = (menuIdx + 1) % MENU_COUNT;
        menuAt = now;
    } else if (e == Ev::Long) {
        doMenu(menuIdx, now);
        return;
    } else if (now - menuAt > MENU_TIMEOUT_MS) {
        go(Scene::Life);
        return;
    }
    cv.blit(*MENU_ICONS[menuIdx], 0, 0);
}

void sceneStatus(uint32_t now, Ev e) {
    uint32_t t = now - sceneAt;
    if (e == Ev::Short || e == Ev::Long) {
        if (statusPage == 0) {
            statusPage = 1;
            sceneAt = now;
        } else {
            go(Scene::Life);
        }
        return;
    }
    if (t > (statusPage == 0 ? STATUS_TIMEOUT_MS : 9000UL)) {
        go(Scene::Life);
        return;
    }
    const PetState &s = sim.s();
    if (statusPage == 0) {
        struct Bar { uint8_t v; Rgb c; };
        const Bar bars[] = {{s.hunger, {255, 138, 26}}, {s.happy, {255, 70, 140}},
                            {s.energy, {255, 214, 46}}, {sim.care(), {62, 214, 76}}};
        for (uint8_t i = 0; i < 4; i++) {
            int len = (bars[i].v * MATRIX_W + 50) / 100;
            for (int x = 0; x < MATRIX_W; x++) cv.set(x, 1 + i * 2, x < len ? bars[i].c : dim(bars[i].c, 30));
        }
    } else {
        char buf[32];
        uint16_t days = sim.ageDays();
        snprintf(buf, sizeof(buf), "%u %s%s", days, days == 1 ? "DIA" : "DIAS", s.wild ? " SELVAGEM" : "");
        scrollText(buf, t, 1, s.wild ? Rgb{200, 160, 90} : C_WHITE);
    }
}

void sceneDead(uint32_t now, Ev e) {
    if (e == Ev::Long) {
        sim.restart();
        go(Scene::Select);
        return;
    }
    uint32_t p = (now - sceneAt) % 9000;
    if (p < 6000) {
        cv.blit(SPR_grave, 0, 3);
        int bob = ((now / 600) % 2) ? 1 : 0;
        cv.blit(frame(ANIM_ghost, now), 3, bob);
    } else {
        char name[16], buf[24];
        upperName(def().name, name, sizeof(name));
        snprintf(buf, sizeof(buf), "RIP %s", name);
        scrollText(buf, p - 6000, 1, {200, 210, 230}, 90);
    }
}

void drawResetBar() {
    uint32_t held = Input::heldMs();
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

} // namespace

namespace Game {

void begin() {
    sim.begin();
    lastEggMs = millis();
    enterFromSaved();
    const PetState &s = sim.s();
    Serial.printf("[Game] fase=%d bicho=%s dna=%08lx fome=%u alegria=%u energia=%u idade=%lu min\n",
                  (int)s.phase, def().name, (unsigned long)s.dna, s.hunger, s.happy, s.energy,
                  (unsigned long)s.ageMin);
}

void update() {
    uint32_t now = millis();
    sim.update();

    Ev e = Events::pop();
    if (e == Ev::Reset) {
        Serial.println("[Game] reset: recomecando do zero");
        sim.restart();
        Events::clear();
        go(Scene::Select);
        return;
    }

    if (now - lastFrame < FRAME_MS && e == Ev::None) return;
    lastFrame = now;

    cv.clear();
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
    Display::show(cv);

    if (now - lastLog > 30000 && sim.s().phase == Phase::Alive) {
        lastLog = now;
        const PetState &s = sim.s();
        Serial.printf("[Game] fome=%u alegria=%u energia=%u coco=%u doente=%d dormindo=%d descuido=%u selvagem=%d idade=%lumin\n",
                      s.hunger, s.happy, s.energy, s.poop, s.sick, s.asleep, s.neglect, s.wild,
                      (unsigned long)s.ageMin);
    }
}

} // namespace Game
