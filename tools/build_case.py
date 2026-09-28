#!/usr/bin/env python3
"""Gera os STLs e imagens da case a partir de hardware/case/pixelgochi_case.scad.

    python tools/build_case.py

Precisa do OpenSCAD instalado (procura no PATH e em C:/Program Files/OpenSCAD).
Também confere colisões: a placa simplificada não pode encostar em nenhuma peça.
"""
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CASE = os.path.join(ROOT, "hardware", "case")
SCAD = os.path.join(CASE, "pixelgochi_case.scad")
STL_DIR = os.path.join(CASE, "stl")
IMG_DIR = os.path.join(CASE, "images")

STLS = {
    "front_open.stl": {"PART": "front", "STYLE": "open"},
    "front_diffuser.stl": {"PART": "front", "STYLE": "diffuser"},
    "back.stl": {"PART": "back"},
    "pin.stl": {"PART": "pin"},
}
IMAGES = {
    "assembly_front.png": ({"PART": "assembly"}, "0,0,0,200,0,20,0"),
    "assembly_back.png": ({"PART": "assembly"}, "0,0,0,-25,0,0,0"),
    "exploded.png": ({"PART": "exploded"}, "0,0,0,55,0,25,0"),
    "front_open.png": ({"PART": "front", "STYLE": "open"}, "0,0,0,200,0,20,0"),
    "front_diffuser.png": ({"PART": "front", "STYLE": "diffuser"}, "0,0,0,160,0,20,0"),
    "back_outside.png": ({"PART": "back"}, "0,0,0,200,0,160,0"),
}
COLLISIONS = ["collide_front", "collide_back", "collide_pins"]


def openscad():
    exe = shutil.which("openscad") or shutil.which("openscad.com")
    for c in (r"C:\Program Files\OpenSCAD\openscad.com", r"C:\Program Files\OpenSCAD\openscad.exe"):
        if not exe and os.path.exists(c):
            exe = c
    if not exe:
        sys.exit("OpenSCAD não encontrado. Instale em https://openscad.org/downloads.html")
    return exe


def run(exe, out, params, extra=()):
    args = [exe, "-o", out]
    for k, v in params.items():
        args += ["-D", f'{k}="{v}"']
    args += list(extra) + [SCAD]
    r = subprocess.run(args, capture_output=True, text=True)
    return r.returncode, r.stdout + r.stderr


def main():
    exe = openscad()
    os.makedirs(STL_DIR, exist_ok=True)
    os.makedirs(IMG_DIR, exist_ok=True)
    tmp = os.path.join(CASE, ".tmp_collide.stl")

    ok = True
    for name in COLLISIONS:
        for style in ("open", "diffuser"):
            if os.path.exists(tmp):
                os.remove(tmp)
            code, log = run(exe, tmp, {"PART": name, "STYLE": style})
            # Encostar (placa apoiada na borda) gera interseção sem volume:
            # só conta como colisão se sobrar alguma face de verdade.
            facets = 0
            if os.path.exists(tmp):
                with open(tmp, encoding="utf-8", errors="ignore") as f:
                    facets = f.read().count("facet normal")
            empty = "Current top level object is empty" in log or facets == 0
            if code != 0 and not empty:
                print(log)
                sys.exit(f"[case] erro ao checar {name}")
            print(f"[case] {name:14s} {style:8s}: {'ok (sem contato)' if empty else 'COLISÃO!'}")
            ok &= empty
    if os.path.exists(tmp):
        os.remove(tmp)
    if not ok:
        sys.exit("[case] a placa encosta em alguma peça — ajuste as medidas antes de imprimir")

    for fname, params in STLS.items():
        code, log = run(exe, os.path.join(STL_DIR, fname), params)
        if code != 0:
            print(log)
            sys.exit(f"[case] falhou: {fname}")
        size = [ln for ln in log.splitlines() if "Case externa" in ln]
        print(f"[case] {fname}" + (f"  {size[0].split('ECHO: ')[-1]}" if size else ""))

    for fname, (params, cam) in IMAGES.items():
        code, log = run(exe, os.path.join(IMG_DIR, fname), params,
                        ["--imgsize=900,700", "--viewall", "--autocenter", f"--camera={cam}",
                         "--colorscheme=Tomorrow Night", "--render"])
        if code != 0:
            print(log)
            sys.exit(f"[case] falhou: {fname}")
    print(f"[case] imagens em {os.path.relpath(IMG_DIR, ROOT)}")


if __name__ == "__main__":
    main()
