#!/usr/bin/env python3
"""Gera os STLs e imagens das cases a partir de hardware/case/*.scad.

    python tools/build_case.py            # as duas cases
    python tools/build_case.py bateria    # só a com bateria (ou: simples)

Precisa do OpenSCAD instalado (procura no PATH e em C:/Program Files/OpenSCAD).
Antes de exportar, confere colisões: modelos simplificados da placa (e da
bateria, carregador e chave) não podem encostar em nenhuma peça.
"""
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CASE = os.path.join(ROOT, "hardware", "case")
STL_DIR = os.path.join(CASE, "stl")
IMG_DIR = os.path.join(CASE, "images")

FRONT_CAM = "0,0,0,200,0,20,0"
BACK_CAM = "0,0,0,-25,0,0,0"
EXPLODED_CAM = "0,0,0,55,0,25,0"

CASES = {
    "simples": {
        "scad": "pixelgochi_case.scad",
        "collisions": ["collide_front", "collide_back", "collide_pins"],
        "stls": {
            "front_open.stl": {"PART": "front", "STYLE": "open"},
            "front_diffuser.stl": {"PART": "front", "STYLE": "diffuser"},
            "back.stl": {"PART": "back"},
            "pin.stl": {"PART": "pin"},
        },
        "images": {
            "assembly_front.png": ({"PART": "assembly"}, FRONT_CAM),
            "assembly_back.png": ({"PART": "assembly"}, BACK_CAM),
            "exploded.png": ({"PART": "exploded"}, EXPLODED_CAM),
            "front_open.png": ({"PART": "front", "STYLE": "open"}, FRONT_CAM),
            "front_diffuser.png": ({"PART": "front", "STYLE": "diffuser"}, "0,0,0,160,0,20,0"),
            "back_outside.png": ({"PART": "back"}, "0,0,0,200,0,160,0"),
        },
    },
    "bateria": {
        "scad": "pixelgochi_case_bateria.scad",
        "collisions": ["collide_front", "collide_mid", "collide_lid", "collide_pins", "collide_pins_parts"],
        "stls": {
            "bateria_front_open.stl": {"PART": "front", "STYLE": "open"},
            "bateria_front_diffuser.stl": {"PART": "front", "STYLE": "diffuser"},
            "bateria_mid.stl": {"PART": "mid"},
            "bateria_lid.stl": {"PART": "lid"},
            "bateria_pin.stl": {"PART": "pin"},
        },
        "images": {
            "bateria_assembly_front.png": ({"PART": "assembly"}, FRONT_CAM),
            "bateria_assembly_back.png": ({"PART": "assembly"}, BACK_CAM),
            "bateria_exploded.png": ({"PART": "exploded"}, EXPLODED_CAM),
        },
    },
}


def openscad():
    exe = shutil.which("openscad") or shutil.which("openscad.com")
    for c in (r"C:\Program Files\OpenSCAD\openscad.com", r"C:\Program Files\OpenSCAD\openscad.exe"):
        if not exe and os.path.exists(c):
            exe = c
    if not exe:
        sys.exit("OpenSCAD não encontrado. Instale em https://openscad.org/downloads.html")
    return exe


def run(exe, scad, out, params, extra=()):
    args = [exe, "-o", out]
    for k, v in params.items():
        args += ["-D", f'{k}="{v}"']
    args += list(extra) + [scad]
    r = subprocess.run(args, capture_output=True, text=True)
    return r.returncode, r.stdout + r.stderr


def build(exe, name, cfg):
    scad = os.path.join(CASE, cfg["scad"])
    tmp = os.path.join(CASE, ".tmp_collide.stl")
    ok = True
    for part in cfg["collisions"]:
        for style in ("open", "diffuser"):
            if os.path.exists(tmp):
                os.remove(tmp)
            code, log = run(exe, scad, tmp, {"PART": part, "STYLE": style})
            if "ERROR" in log or "Assertion" in log:
                print(log)
                sys.exit(f"[case:{name}] erro ao checar {part}")
            # Encostar (placa apoiada na borda) gera interseção sem volume:
            # só conta como colisão se sobrar alguma face de verdade.
            facets = 0
            if os.path.exists(tmp):
                with open(tmp, encoding="utf-8", errors="ignore") as f:
                    facets = f.read().count("facet normal")
            empty = "Current top level object is empty" in log or facets == 0
            if code != 0 and not empty:
                print(log)
                sys.exit(f"[case:{name}] erro ao checar {part}")
            print(f"[case:{name}] {part:18s} {style:8s}: {'ok (sem contato)' if empty else 'COLISÃO!'}")
            ok &= empty
    if os.path.exists(tmp):
        os.remove(tmp)
    if not ok:
        sys.exit(f"[case:{name}] alguma peça encosta onde não devia — ajuste as medidas antes de imprimir")

    for fname, params in cfg["stls"].items():
        code, log = run(exe, scad, os.path.join(STL_DIR, fname), params)
        if code != 0:
            print(log)
            sys.exit(f"[case:{name}] falhou: {fname}")
        size = [ln for ln in log.splitlines() if "Case externa" in ln]
        print(f"[case:{name}] {fname}" + (f"  {size[0].split('ECHO: ')[-1]}" if size else ""))

    for fname, (params, cam) in cfg["images"].items():
        code, log = run(exe, scad, os.path.join(IMG_DIR, fname), params,
                        ["--imgsize=900,700", "--viewall", "--autocenter", f"--camera={cam}",
                         "--colorscheme=Tomorrow Night", "--render"])
        if code != 0:
            print(log)
            sys.exit(f"[case:{name}] falhou: {fname}")


def main():
    exe = openscad()
    os.makedirs(STL_DIR, exist_ok=True)
    os.makedirs(IMG_DIR, exist_ok=True)
    wanted = sys.argv[1:] or list(CASES)
    for name in wanted:
        if name not in CASES:
            sys.exit(f"case desconhecida: {name} (opções: {', '.join(CASES)})")
        build(exe, name, CASES[name])
    print(f"[case] STLs em {os.path.relpath(STL_DIR, ROOT)}, imagens em {os.path.relpath(IMG_DIR, ROOT)}")


if __name__ == "__main__":
    main()
