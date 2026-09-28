#!/usr/bin/env python3
"""Compila art/*.art em dados C++ (src/art/) e no preview do navegador (preview/art.js).

Formato do .art (uma fonte da verdade pra firmware e preview):

    # comentário
    palette capy
      B #C06A2B
      D #7A3510
    end

    sprite capy_idle0 capy
    ..BBBB..
    .BBBBBB.
    end

    anim capy_idle 600 capy_idle0 capy_idle1
    anim capy_egg  700 egg0@egg_capy          # sprite@paleta troca a paleta

    pet capy "Capivara" food=food_melon

'.' é sempre transparente. Todas as linhas de um sprite têm a mesma largura.
"""
import json
import math
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ART_DIR = os.path.join(ROOT, "art")
OUT_H = os.path.join(ROOT, "src", "art", "ArtData.h")
OUT_CPP = os.path.join(ROOT, "src", "art", "ArtData.cpp")
OUT_JS = os.path.join(ROOT, "preview", "art.js")
OUT_LED = os.path.join(ROOT, "src", "art", "LedProfile.h")

PET_ANIMS = ["idle", "blink", "walk", "eat", "sleep", "happy", "sad", "hungry", "tired"]
PET_EGG_ANIMS = ["egg"]


class ArtError(Exception):
    pass


def strip_comment(line):
    # Comentário = linha começando com '#', ou " # " no meio da linha.
    # Cores (#RRGGBB) não têm espaço depois do '#', então não colidem.
    s = line.strip()
    if s.startswith("#"):
        return ""
    return re.sub(r"\s+#\s.*$", "", s)


def parse(paths):
    palettes, sprites, anims, pets = {}, {}, {}, []
    for path in paths:
        with open(path, encoding="utf-8") as f:
            lines = f.read().splitlines()
        i = 0

        def err(msg):
            raise ArtError(f"{os.path.basename(path)}:{i + 1}: {msg}")

        while i < len(lines):
            line = strip_comment(lines[i])
            if not line:
                i += 1
                continue
            parts = line.split()
            kw = parts[0]

            if kw == "palette":
                # "palette nome : base" = variante: herda as cores e a ORDEM
                # dos chars da base (o firmware troca a paleta em tempo de
                # execução, então os índices precisam bater).
                m = re.fullmatch(r"palette\s+(\w+)(?:\s*:\s*(\w+))?", line)
                if not m:
                    err("sintaxe: palette nome [: base]")
                name, base = m.group(1), m.group(2)
                if name in palettes:
                    err(f"paleta duplicada: {name}")
                if base and base not in palettes:
                    err(f"paleta base inexistente (defina antes): {base}")
                pal = dict(palettes[base]) if base else {}
                i += 1
                while i < len(lines) and lines[i].strip() != "end":
                    p = strip_comment(lines[i]).split()
                    if p:
                        if len(p) != 2 or len(p[0]) != 1 or not re.fullmatch(r"#[0-9A-Fa-f]{6}", p[1]):
                            err(f"linha de paleta inválida: {lines[i]!r}")
                        if p[0] == ".":
                            err("'.' é reservado pra transparente")
                        if base and p[0] not in pal:
                            err(f"variante {name}: cor '{p[0]}' não existe na base {base}")
                        pal[p[0]] = p[1].upper()
                    i += 1
                palettes[name] = pal

            elif kw == "sprite":
                name, pal = parts[1], parts[2]
                if name in sprites:
                    err(f"sprite duplicado: {name}")
                rows = []
                i += 1
                while i < len(lines) and lines[i].strip() != "end":
                    r = strip_comment(lines[i])
                    if r:
                        rows.append(r)
                    i += 1
                if not rows:
                    err(f"sprite vazio: {name}")
                w = len(rows[0])
                if any(len(r) != w for r in rows):
                    err(f"sprite {name}: linhas com larguras diferentes")
                if w > 8 or len(rows) > 8:
                    err(f"sprite {name}: maior que 8x8 ({w}x{len(rows)})")
                sprites[name] = {"pal": pal, "rows": rows, "w": w, "h": len(rows)}

            elif kw == "anim":
                name, ms, frames = parts[1], int(parts[2]), parts[3:]
                if name in anims:
                    err(f"anim duplicada: {name}")
                if not frames:
                    err(f"anim {name} sem frames")
                anims[name] = {"ms": ms, "frames": frames}

            elif kw == "pet":
                m = re.match(r'pet\s+(\w+)\s+"([^"]+)"(.*)$', line)
                if not m:
                    err("sintaxe: pet id \"Nome\" food=sprite")
                opts = dict(kv.split("=", 1) for kv in m.group(3).split())
                pets.append({"id": m.group(1), "name": m.group(2), "food": opts.get("food"),
                             "wild": opts.get("wild"), "side": opts.get("side") == "1"})
            else:
                err(f"palavra-chave desconhecida: {kw}")
            i += 1
    return palettes, sprites, anims, pets


def resolve_frame(ref, sprites, palettes):
    spr, _, pal = ref.partition("@")
    if spr not in sprites:
        raise ArtError(f"frame referencia sprite inexistente: {spr}")
    pal = pal or sprites[spr]["pal"]
    if pal not in palettes:
        raise ArtError(f"paleta inexistente: {pal}")
    missing = {c for r in sprites[spr]["rows"] for c in r if c != "."} - set(palettes[pal])
    if missing:
        raise ArtError(f"{spr}@{pal}: cores sem definição na paleta: {''.join(sorted(missing))}")
    return spr, pal


def validate(palettes, sprites, anims, pets):
    for name, s in sprites.items():
        resolve_frame(name, sprites, palettes)
    for name, a in anims.items():
        for f in a["frames"]:
            resolve_frame(f, sprites, palettes)
    for p in pets:
        for suffix in PET_ANIMS + PET_EGG_ANIMS:
            if f"{p['id']}_{suffix}" not in anims:
                raise ArtError(f"pet {p['id']}: falta a anim {p['id']}_{suffix}")
        if p["food"] and p["food"] not in sprites:
            raise ArtError(f"pet {p['id']}: comida inexistente {p['food']}")
        if p["wild"]:
            base = pet_palette(p, anims, sprites)
            if p["wild"] not in palettes:
                raise ArtError(f"pet {p['id']}: paleta selvagem inexistente {p['wild']}")
            if list(palettes[p["wild"]]) != list(palettes[base]):
                raise ArtError(f"pet {p['id']}: {p['wild']} precisa ser 'palette {p['wild']} : {base}'")


# Glifos da fonte: sprites "font_X". Nomes especiais pros símbolos.
FONT_SPECIAL = {"dash": "-", "dot": ".", "excl": "!", "colon": ":", "quest": "?", "heart": "*"}


def font_glyphs(sprites):
    out = {}
    for name in sprites:
        m = re.fullmatch(r"font_(\w+)", name)
        if not m:
            continue
        key = m.group(1)
        ch = FONT_SPECIAL.get(key, key if len(key) == 1 else None)
        if ch is None:
            raise ArtError(f"glifo com nome desconhecido: {name}")
        out[ch] = name
    return out


def pet_palette(p, anims, sprites):
    first = anims[f"{p['id']}_idle"]["frames"][0]
    return first.partition("@")[2] or sprites[first.partition("@")[0]]["pal"]


def cid(s):
    return re.sub(r"\W", "_", s)


def emit_cpp(palettes, sprites, anims, pets):
    # Cada combinação sprite@paleta usada vira um Sprite C++ próprio.
    variants = {}
    for name, s in sprites.items():
        variants[(name, s["pal"])] = None
    for a in anims.values():
        for f in a["frames"]:
            variants[resolve_frame(f, sprites, palettes)] = None

    def vname(spr, pal):
        return f"SPR_{cid(spr)}" if pal == sprites[spr]["pal"] else f"SPR_{cid(spr)}__{cid(pal)}"

    h = [
        "// GERADO por tools/gen_art.py a partir de art/*.art -- não edite à mão.",
        "#pragma once",
        '#include "ArtTypes.h"',
        "",
        "namespace Art {",
    ]
    for (spr, pal) in variants:
        h.append(f"extern const Sprite {vname(spr, pal)};")
    for name in anims:
        h.append(f"extern const Anim ANIM_{cid(name)};")
    h += [
        f"constexpr uint8_t PET_COUNT = {len(pets)};",
        "extern const PetDef PETS[PET_COUNT];",
        "const Sprite *glyph(char c); // nullptr se não houver na fonte",
        "} // namespace Art",
        "",
    ]

    c = [
        "// GERADO por tools/gen_art.py a partir de art/*.art -- não edite à mão.",
        '#include "ArtData.h"',
        "",
        "namespace Art {",
        "namespace {",
    ]
    for pname, pal in palettes.items():
        chars = list(pal)
        vals = ", ".join(f"0x{pal[ch][1:]}" for ch in chars)
        c.append(f"const uint32_t PAL_{cid(pname)}[] = {{0x000000, {vals}}};")
    # Índices de pixel dependem da ordem dos chars na paleta, então cada
    # variante sprite@paleta tem seu próprio array de pixels.
    for (spr, pal) in variants:
        s = sprites[spr]
        chars = list(palettes[pal])
        idx = [0 if ch == "." else chars.index(ch) + 1 for r in s["rows"] for ch in r]
        c.append(f"const uint8_t PX_{cid(spr)}__{cid(pal)}[] = {{{', '.join(map(str, idx))}}};")
    c.append("} // namespace")
    c.append("")
    for (spr, pal) in variants:
        s = sprites[spr]
        c.append(
            f"const Sprite {vname(spr, pal)} = {{{s['w']}, {s['h']}, PX_{cid(spr)}__{cid(pal)}, PAL_{cid(pal)}}};"
        )
    c.append("")
    for name, a in anims.items():
        frames = ", ".join(f"&{vname(*resolve_frame(f, sprites, palettes))}" for f in a["frames"])
        c.append(f"static const Sprite *const FR_{cid(name)}[] = {{{frames}}};")
        c.append(f"const Anim ANIM_{cid(name)} = {{{a['ms']}, {len(a['frames'])}, FR_{cid(name)}}};")
    c.append("")
    c.append("const PetDef PETS[PET_COUNT] = {")
    for p in pets:
        refs = ", ".join(f"&ANIM_{cid(p['id'] + '_' + s)}" for s in PET_ANIMS + PET_EGG_ANIMS)
        food = f"&{vname(p['food'], sprites[p['food']]['pal'])}" if p["food"] else "nullptr"
        wild = f"PAL_{cid(p['wild'])}" if p["wild"] else "nullptr"
        side = "true" if p["side"] else "false"
        c.append(f'    {{"{p["id"]}", "{p["name"]}", {refs}, {food}, {wild}, {side}}},')
    c.append("};")
    c.append("")
    c.append("const Sprite *glyph(char c) {")
    c.append("    switch (c) {")
    for ch, name in sorted(font_glyphs(sprites).items()):
        lit = "'\\''" if ch == "'" else f"'{ch}'"
        c.append(f"        case {lit}: return &{vname(name, sprites[name]['pal'])};")
    c.append("        default: return nullptr;")
    c.append("    }")
    c.append("}")
    c.append("} // namespace Art")
    c.append("")
    return "\n".join(h), "\n".join(c)


def load_led_profile():
    with open(os.path.join(ART_DIR, "led-profile.json"), encoding="utf-8") as f:
        profile = json.load(f)
    brightness, gamma = profile.get("brightness"), profile.get("gamma")
    if type(brightness) is not int or not 1 <= brightness <= 30:
        raise ArtError("led-profile.json: brightness deve ser inteiro de 1 a 30")
    if type(gamma) not in (int, float) or not math.isfinite(gamma) or not 1 <= gamma <= 2.2:
        raise ArtError("led-profile.json: gamma deve estar entre 1 e 2.2")
    glare_cap = profile.get("glare_cap", 765)
    blue_gain = profile.get("blue_gain", 1.0)
    min_peak = profile.get("min_peak", 0)
    if type(glare_cap) is not int or not 255 <= glare_cap <= 765:
        raise ArtError("led-profile.json: glare_cap deve ser inteiro de 255 a 765")
    if type(blue_gain) not in (int, float) or not 0.3 <= blue_gain <= 1.0:
        raise ArtError("led-profile.json: blue_gain deve estar entre 0.3 e 1.0")
    if type(min_peak) is not int or not 0 <= min_peak <= brightness:
        raise ArtError("led-profile.json: min_peak deve ser inteiro de 0 até o brightness")
    return {"brightness": brightness, "gamma": gamma, "glareCap": glare_cap,
            "blueGain": blue_gain, "minPeak": min_peak,
            "gammaLut": [int(255 * (v / 255) ** gamma + 0.5) for v in range(256)]}


def emit_led_profile(profile):
    lines = ["// GERADO de art/led-profile.json por tools/gen_art.py -- não edite à mão.",
             "#pragma once", "#include <stdint.h>", "", "namespace LedProfile {",
             f"constexpr uint8_t BRIGHTNESS = {profile['brightness']};",
             f"// Curva suave gamma {profile['gamma']}; mantém preto/primárias e separa meios-tons.",
             "constexpr uint8_t GAMMA_LUT[256] = {"]
    for i in range(0, 256, 16):
        lines.append("    " + ", ".join(map(str, profile["gammaLut"][i:i+16])) + ",")
    lines += [
        "};",
        f"constexpr uint16_t GLARE_CAP = {profile['glareCap']};",
        f"constexpr uint8_t BLUE_GAIN = {round(profile['blueGain'] * 255)}; // /255",
        f"constexpr uint8_t MIN_PEAK = {profile['minPeak']};",
        "inline uint8_t channel(uint8_t value) { return GAMMA_LUT[value]; }",
        "",
        "// Brilho por cor, antes do teto global do FastLED (BRIGHTNESS):",
        "// 1) gama; 2) azul atenuado; 3) soma R+G+B limitada a GLARE_CAP (branco e",
        "// tons claros ofuscam menos, cores puras intactas); 4) cor acesa nunca fica",
        "// abaixo de MIN_PEAK níveis na saída (escuras não somem), mantendo o tom.",
        "// Espelhado em preview/index.html (ledColor).",
        "inline void color(uint8_t r, uint8_t g, uint8_t b, uint8_t &outR, uint8_t &outG, uint8_t &outB) {",
        "    float fr = GAMMA_LUT[r], fg = GAMMA_LUT[g], fb = GAMMA_LUT[b] * BLUE_GAIN / 255.0f;",
        "    float sum = fr + fg + fb;",
        "    if (sum > GLARE_CAP) {",
        "        float k = GLARE_CAP / sum;",
        "        fr *= k; fg *= k; fb *= k;",
        "    }",
        "    float peak = fr > fg ? (fr > fb ? fr : fb) : (fg > fb ? fg : fb);",
        "    // saída do FastLED = v * (BRIGHTNESS + 1) / 256; arredonda pra cima",
        "    const float floorV = (MIN_PEAK * 256 + BRIGHTNESS) / (BRIGHTNESS + 1);",
        "    if (peak > 0 && peak < floorV) {",
        "        float k = floorV / peak;",
        "        fr *= k; fg *= k; fb *= k;",
        "    }",
        "    auto q = [](float v) -> uint8_t { return v >= 255 ? 255 : (uint8_t)(v + 0.5f); };",
        "    outR = q(fr); outG = q(fg); outB = q(fb);",
        "}",
        "} // namespace LedProfile",
        "",
    ]
    return "\n".join(lines)


def emit_js(palettes, sprites, anims, pets, led_profile):
    data = {"palettes": palettes, "sprites": sprites, "anims": anims, "pets": pets,
            "font": font_glyphs(sprites), "ledProfile": led_profile}
    return "// GERADO por tools/gen_art.py -- não edite à mão.\nwindow.ART = " + json.dumps(data, ensure_ascii=False, indent=1) + ";\n"


def write_if_changed(path, content):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            if f.read() == content:
                return False
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(content)
    return True


def main():
    paths = sorted(os.path.join(ART_DIR, f) for f in os.listdir(ART_DIR) if f.endswith(".art"))
    try:
        parsed = parse(paths)
        validate(*parsed)
        led_profile = load_led_profile()
    except ArtError as e:
        print(f"[gen_art] ERRO: {e}", file=sys.stderr)
        sys.exit(1)
    h, cpp = emit_cpp(*parsed)
    outputs = ((OUT_H, h), (OUT_CPP, cpp), (OUT_LED, emit_led_profile(led_profile)),
               (OUT_JS, emit_js(*parsed, led_profile=led_profile)))
    changed = [p for p, content in outputs if write_if_changed(p, content)]
    palettes, sprites, anims, pets = parsed
    print(f"[gen_art] {len(sprites)} sprites, {len(anims)} anims, {len(pets)} pets"
          + (f" -> atualizado: {', '.join(os.path.relpath(p, ROOT) for p in changed)}" if changed else " (sem mudanças)"))


if __name__ == "__main__":
    main()
