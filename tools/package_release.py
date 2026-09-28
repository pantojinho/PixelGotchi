#!/usr/bin/env python3
"""Monta o pacote de uma release em dist/PixelGotchi-<versão>/ (não publica nada).

    python tools/package_release.py v0.1.0

Compila o firmware, junta bootloader + partições + app numa imagem única
(gravável em 0x0), copia os binários separados, os STLs da case, gera
SHA256SUMS.txt e um zip. Os offsets vêm do próprio build (PlatformIO), não
de valores fixos. Publicar: enviar uma tag v* (.github/workflows/release.yml).
"""
import hashlib
import json
import os
import shutil
import subprocess
import sys
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ENV = "esp32-s3-matrix"
PIO_HOME = os.path.join(os.path.expanduser("~"), ".platformio")


def pio(*args, capture=False):
    cmd = [sys.executable, "-m", "platformio", *args]
    if capture:
        return subprocess.run(cmd, cwd=ROOT, check=True, capture_output=True, text=True).stdout
    subprocess.run(cmd, cwd=ROOT, check=True)


def esptool_cmd():
    # O esptool do PlatformIO roda no Python dele (já com as dependências).
    py = os.path.join(PIO_HOME, "penv", "Scripts" if os.name == "nt" else "bin", "python")
    tool = os.path.join(PIO_HOME, "packages", "tool-esptoolpy", "esptool.py")
    if not os.path.exists(tool):
        sys.exit("esptool do PlatformIO não encontrado; rode um 'pio run' antes.")
    return [py if os.path.exists(py) or os.path.exists(py + ".exe") else sys.executable, tool]


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 16), b""):
            h.update(chunk)
    return h.hexdigest()


def main():
    version = sys.argv[1] if len(sys.argv) > 1 else "dev"
    out = os.path.join(ROOT, "dist", f"PixelGotchi-{version}")
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(os.path.join(out, "firmware"))
    os.makedirs(os.path.join(out, "case"))

    pio("run", "-e", ENV)
    meta = json.loads(pio("project", "metadata", "-e", ENV, "--json-output", capture=True))[ENV]
    build_dir = os.path.dirname(meta["prog_path"])
    images = [(img["offset"], img["path"]) for img in meta["extra"]["flash_images"]]
    images.append((meta["extra"]["application_offset"], os.path.join(build_dir, "firmware.bin")))

    parts = []
    for offset, path in images:
        dst = os.path.join(out, "firmware", os.path.basename(path))
        shutil.copy2(path, dst)
        parts += [offset, dst]
    merged = os.path.join(out, "firmware", f"PixelGotchi-{version}-esp32s3-4MB-merged.bin")
    subprocess.run(esptool_cmd() + ["--chip", "esp32s3", "merge_bin", "-o", merged,
                                    "--flash_mode", "dio", "--flash_size", "4MB", *parts], check=True)

    with open(os.path.join(out, "firmware", "offsets.txt"), "w", encoding="utf-8") as f:
        f.write("INSTALAÇÃO DO ZERO (apaga o pet salvo e o bichinho do editor — a imagem cobre a NVS)\n"
                "FRESH INSTALL (erases the saved pet and the editor pet — the image covers NVS):\n"
                f"  esptool.py --chip esp32s3 write_flash 0x0 {os.path.basename(merged)}\n\n"
                "ATUALIZAR MANTENDO O PET (grava só o programa)\n"
                "UPDATE KEEPING THE PET (writes only the program):\n"
                f"  esptool.py --chip esp32s3 write_flash {images[-1][0]} firmware.bin\n\n"
                "Arquivos separados e onde vão / Separate files and their offsets:\n")
        for offset, path in images:
            f.write(f"  {offset}  {os.path.basename(path)}\n")

    case_src = os.path.join(ROOT, "hardware", "case")
    for name in os.listdir(os.path.join(case_src, "stl")):
        shutil.copy2(os.path.join(case_src, "stl", name), os.path.join(out, "case", name))
    for doc in ("README.md", "README.en.md"):
        shutil.copy2(os.path.join(case_src, doc), os.path.join(out, "case", doc))

    notes = os.path.join(ROOT, "docs", "releases", f"{version}.md")
    if os.path.exists(notes):
        shutil.copy2(notes, os.path.join(out, "RELEASE_NOTES.md"))

    sums = []
    for base, _, files in os.walk(out):
        for name in sorted(files):
            p = os.path.join(base, name)
            sums.append(f"{sha256(p)}  {os.path.relpath(p, out).replace(os.sep, '/')}")
    with open(os.path.join(out, "SHA256SUMS.txt"), "w", encoding="utf-8") as f:
        f.write("\n".join(sums) + "\n")

    zip_path = out + ".zip"
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as z:
        for base, _, files in os.walk(out):
            for name in files:
                p = os.path.join(base, name)
                z.write(p, os.path.relpath(p, os.path.dirname(out)))
    print(f"[release] {os.path.relpath(out, ROOT)} e {os.path.relpath(zip_path, ROOT)}")


if __name__ == "__main__":
    main()
