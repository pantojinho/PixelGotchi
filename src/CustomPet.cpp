#include "CustomPet.h"
#include "Game.h"
#include "Storage.h"
#include "art/ArtData.h"
#include <stdio.h>
#include <string.h>

using namespace Art;

namespace CustomPet {
namespace {

const char *KEY = "custompet";
const Sprite *const FOODS[FOOD_COUNT] = {&SPR_food_melon, &SPR_food_fish, &SPR_food_fly,
                                         &SPR_food_worm, &SPR_food_carrot, &SPR_food_shrimp};

// Pacote ativo, decodificado em estruturas iguais às da arte de fábrica.
bool loaded = false;
char name[MAX_NAME + 1];
uint32_t pal[MAX_COLORS + 1], wildPal[MAX_COLORS + 1], eggPal[5];
uint8_t pixels[MAX_FRAMES * 64];
Sprite sprites[MAX_FRAMES];
Sprite eggSprite;
const Sprite *animFrames[ANIM_COUNT][MAX_ANIM_FRAMES];
const Sprite *eggFrames[1];
Anim anims[ANIM_COUNT + 1];
PetDef pet;

// Envio em andamento.
uint8_t incoming[MAX_BYTES];
size_t expected = 0, received = 0;
bool receiving = false;

struct Reader {
    const uint8_t *p;
    size_t left;
    bool ok;
    Reader(const uint8_t *data, size_t n) : p(data), left(n), ok(true) {}
    uint8_t u8() {
        if (!left) { ok = false; return 0; }
        --left;
        return *p++;
    }
    uint16_t u16() { const uint8_t lo = u8(); return (uint16_t)(lo | (u8() << 8)); }
    uint32_t rgb() { const uint32_t r = u8(), g = u8(), b = u8(); return (r << 16) | (g << 8) | b; }
    const uint8_t *take(size_t n) {
        if (n > left) { ok = false; left = 0; return p; }
        const uint8_t *at = p; p += n; left -= n;
        return at;
    }
};

// Versão "selvagem" derivada das cores: mais escura e terrosa (mesma conta
// em preview/petpack.js).
uint32_t wildColor(uint32_t c) {
    const uint32_t r = ((c >> 16) & 0xFF) * 3 / 5, g = ((c >> 8) & 0xFF) / 2, b = (c & 0xFF) * 2 / 5;
    return (r << 16) | (g << 8) | b;
}

// Percorre o pacote. Com apply = false só confere; com true preenche o pet.
const char *parse(const uint8_t *data, size_t len, bool apply) {
    if (len < 8 || len > MAX_BYTES) return "size";
    if (memcmp(data, "PGP1", 4) != 0) return "magic";
    const uint32_t stored = (uint32_t)data[len - 4] | ((uint32_t)data[len - 3] << 8) |
                            ((uint32_t)data[len - 2] << 16) | ((uint32_t)data[len - 1] << 24);
    if (crc32(data, len - 4) != stored) return "crc";
    Reader r{data + 4, len - 8};
    const uint8_t flags = r.u8(), food = r.u8(), nameLen = r.u8();
    if (food >= FOOD_COUNT) return "food";
    if (nameLen > MAX_NAME) return "name";
    const uint8_t *nameBytes = r.take(nameLen);
    for (uint8_t i = 0; r.ok && i < nameLen; ++i)
        if (nameBytes[i] < 32 || nameBytes[i] > 126) return "name";
    const uint8_t colors = r.u8();
    if (!r.ok || colors < 1 || colors > MAX_COLORS) return "colors";
    uint32_t palette[MAX_COLORS + 1] = {0};
    for (uint8_t i = 1; i <= colors; ++i) palette[i] = r.rgb();
    uint32_t egg[5] = {0};
    for (uint8_t i = 1; i <= 4; ++i) egg[i] = r.rgb();
    const uint8_t frames = r.u8();
    if (!r.ok || frames < 1 || frames > MAX_FRAMES) return "frames";
    uint8_t dims[MAX_FRAMES][2];
    const uint8_t *px[MAX_FRAMES];
    for (uint8_t f = 0; f < frames; ++f) {
        const uint8_t w = r.u8(), h = r.u8();
        if (!r.ok || w < 1 || w > 8 || h < 1 || h > 8) return "frame_size";
        px[f] = r.take((size_t)w * h);
        if (!r.ok) return "truncated";
        for (int i = 0; i < w * h; ++i) if (px[f][i] > colors) return "color_index";
        dims[f][0] = w; dims[f][1] = h;
    }
    uint16_t ms[ANIM_COUNT];
    uint8_t counts[ANIM_COUNT], refs[ANIM_COUNT][MAX_ANIM_FRAMES];
    for (uint8_t a = 0; a < ANIM_COUNT; ++a) {
        ms[a] = r.u16();
        counts[a] = r.u8();
        if (!r.ok) return "truncated";
        if (ms[a] < 40 || ms[a] > 5000) return "anim_ms";
        if (counts[a] < 1 || counts[a] > MAX_ANIM_FRAMES) return "anim_frames";
        for (uint8_t i = 0; i < counts[a]; ++i) {
            refs[a][i] = r.u8();
            if (refs[a][i] >= frames) return "frame_ref";
        }
    }
    if (!r.ok) return "truncated";
    if (r.left) return "trailing";
    if (!apply) return nullptr;

    memcpy(name, nameBytes, nameLen);
    name[nameLen] = 0;
    memcpy(pal, palette, sizeof(pal));
    for (uint8_t i = 0; i <= MAX_COLORS; ++i) wildPal[i] = wildColor(pal[i]);
    memcpy(eggPal, egg, sizeof(eggPal));
    size_t at = 0;
    for (uint8_t f = 0; f < frames; ++f) {
        const size_t n = (size_t)dims[f][0] * dims[f][1];
        memcpy(pixels + at, px[f], n);
        sprites[f] = {dims[f][0], dims[f][1], pixels + at, pal};
        at += n;
    }
    for (uint8_t a = 0; a < ANIM_COUNT; ++a) {
        for (uint8_t i = 0; i < counts[a]; ++i) animFrames[a][i] = &sprites[refs[a][i]];
        anims[a] = {ms[a], counts[a], animFrames[a]};
    }
    eggSprite = {SPR_egg0.w, SPR_egg0.h, SPR_egg0.px, eggPal};
    eggFrames[0] = &eggSprite;
    anims[ANIM_COUNT] = {700, 1, eggFrames};
    pet = {"custom", name, &anims[0], &anims[1], &anims[2], &anims[3], &anims[4], &anims[5],
           &anims[6], &anims[7], &anims[8], &anims[ANIM_COUNT], FOODS[food], wildPal,
           (flags & 1) != 0};
    return nullptr;
}

int hexNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

} // namespace

uint32_t crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

void begin() {
    loaded = false;
    const size_t len = Storage::loadBlob(KEY, incoming, sizeof(incoming));
    if (len && !parse(incoming, len, false)) loaded = !parse(incoming, len, true);
}

bool available() { return loaded; }

const PetDef &def() { return pet; }

const char *validate(const uint8_t *data, size_t len) { return parse(data, len, false); }

const char *install(const uint8_t *data, size_t len) {
    if (const char *err = parse(data, len, false)) return err;
    if (!Storage::saveBlob(KEY, data, len)) return "storage";
    parse(data, len, true);
    loaded = true;
    return nullptr;
}

void remove() {
    Storage::eraseBlob(KEY);
    loaded = false;
}

bool handleLine(const char *line, char *reply, size_t n) {
    if (strncmp(line, "PG", 2) != 0) return false;
    const char *cmd = line + 2;
    if (strcmp(cmd, "?") == 0) {
        // O nome é o último campo: pode ter espaços.
        snprintf(reply, n, "PG HELLO %u %u %d %d %s", PROTOCOL, (unsigned)MAX_BYTES, loaded ? 1 : 0,
                 Game::customPetActive() ? 1 : 0, loaded && name[0] ? name : "-");
    } else if (strncmp(cmd, "PUT ", 4) == 0) {
        unsigned long size = 0;
        if (sscanf(cmd + 4, "%lu", &size) != 1 || size < 8 || size > MAX_BYTES) {
            receiving = false;
            snprintf(reply, n, "PG ERR size");
        } else {
            expected = size; received = 0; receiving = true;
            snprintf(reply, n, "PG READY");
        }
    } else if (strncmp(cmd, "D ", 2) == 0) {
        const char *hex = cmd + 2;
        const size_t digits = strlen(hex);
        if (!receiving) snprintf(reply, n, "PG ERR noput");
        else if (digits % 2 || digits > 128) { receiving = false; snprintf(reply, n, "PG ERR hex"); }
        else if (received + digits / 2 > expected) { receiving = false; snprintf(reply, n, "PG ERR overflow"); }
        else {
            for (size_t i = 0; i < digits; i += 2) {
                const int hi = hexNibble(hex[i]), lo = hexNibble(hex[i + 1]);
                if (hi < 0 || lo < 0) { receiving = false; snprintf(reply, n, "PG ERR hex"); return true; }
                incoming[received++] = (uint8_t)(hi << 4 | lo);
            }
            snprintf(reply, n, "PG ACK %u", (unsigned)received);
        }
    } else if (strcmp(cmd, "END") == 0) {
        if (!receiving || received != expected) {
            receiving = false;
            snprintf(reply, n, "PG ERR incomplete");
        } else {
            receiving = false;
            const char *err = install(incoming, received);
            if (err) snprintf(reply, n, "PG ERR %s", err);
            else {
                snprintf(reply, n, "PG SAVED %s", name[0] ? name : "-");
                Game::customPetChanged();
            }
        }
    } else if (strcmp(cmd, "ADOPT") == 0) {
        if (!loaded) snprintf(reply, n, "PG ERR nopet");
        else { Game::adoptCustomPet(); snprintf(reply, n, "PG ADOPTED"); }
    } else if (strcmp(cmd, "DEL") == 0) {
        remove();
        Game::customPetChanged();
        snprintf(reply, n, "PG DELETED");
    } else {
        snprintf(reply, n, "PG ERR command");
    }
    return true;
}

} // namespace CustomPet
