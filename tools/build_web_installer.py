#!/usr/bin/env python3
"""Build the merged image and ESP Web Tools manifest for the USB installer."""
import argparse
import hashlib
import json
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ENV = "esp32-s3-matrix"
PIO_HOME = os.environ.get("PLATFORMIO_CORE_DIR", os.path.join(os.path.expanduser("~"), ".platformio"))


def pio(*args, capture=False):
    installed = os.path.join(PIO_HOME, "penv", "Scripts" if os.name == "nt" else "bin", "pio")
    if os.name == "nt":
        installed += ".exe"
    command = [installed, *args] if os.path.isfile(installed) else [sys.executable, "-m", "platformio", *args]
    result = subprocess.run(command, cwd=ROOT, check=True, capture_output=capture, text=True)
    return result.stdout if capture else ""


def esptool_cmd():
    tool = os.path.join(PIO_HOME, "packages", "tool-esptoolpy", "esptool.py")
    if not os.path.isfile(tool):
        raise SystemExit("esptool do PlatformIO não encontrado; compile o firmware primeiro.")
    py = os.path.join(PIO_HOME, "penv", "Scripts" if os.name == "nt" else "bin", "python")
    py += ".exe" if os.name == "nt" else ""
    # pip install platformio (CI/Linux) não cria o penv do instalador Core.
    return [py if os.path.isfile(py) else sys.executable, tool]


def write_package(out, merged, version):
    with open(merged, "rb") as f:
        data = f.read()
    if not data or data[0] != 0xE9 or len(data) > 4 * 1024 * 1024:
        raise ValueError("Imagem ESP32-S3 inválida ou maior que a flash de 4 MB.")
    relative = os.path.relpath(merged, out).replace(os.sep, "/")
    manifest = {
        "name": "PixelGotchi — Waveshare ESP32-S3-Matrix",
        "version": version,
        "new_install_prompt_erase": False,
        "new_install_improv_wait_time": 0,
        "builds": [{"chipFamily": "ESP32-S3", "serialType": "cdc",
                    "parts": [{"path": relative, "offset": 0}]}],
    }
    info = {"version": version, "path": relative, "size": len(data),
            "sha256": hashlib.sha256(data).hexdigest(), "chipFamily": "ESP32-S3",
            "flashSize": 4 * 1024 * 1024}
    for filename, payload in (("manifest.json", manifest), ("firmware-info.json", info)):
        with open(os.path.join(out, filename), "w", encoding="utf-8", newline="\n") as f:
            json.dump(payload, f, ensure_ascii=False, indent=2)
            f.write("\n")
    return info


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", default=os.path.join(ROOT, "preview"), help="raiz do site (padrão: preview/)")
    parser.add_argument("--version", help="versão que aparece no instalador; padrão: revisão Git curta")
    args = parser.parse_args()
    out = os.path.abspath(args.output)
    version = args.version or subprocess.check_output(["git", "rev-parse", "--short", "HEAD"], cwd=ROOT, text=True).strip()
    if not args.version and subprocess.run(["git", "diff", "--quiet", "HEAD"], cwd=ROOT).returncode:
        version += "-dev"
    if not re.fullmatch(r"[A-Za-z0-9._-]+", version):
        raise SystemExit("A versão deve conter apenas letras, números, ponto, hífen e sublinhado.")

    pio("run", "-e", ENV)
    meta = json.loads(pio("project", "metadata", "-e", ENV, "--json-output", capture=True))[ENV]
    build_dir = os.path.dirname(meta["prog_path"])
    images = [(img["offset"], img["path"]) for img in meta["extra"]["flash_images"]]
    images.append((meta["extra"]["application_offset"], os.path.join(build_dir, "firmware.bin")))
    firmware_dir = os.path.join(out, "firmware")
    os.makedirs(firmware_dir, exist_ok=True)
    merged = os.path.join(firmware_dir, f"PixelGotchi-esp32s3-{version}.bin")
    parts = [item for offset, path in images for item in (offset, path)]
    subprocess.run(esptool_cmd() + ["--chip", "esp32s3", "merge_bin", "-o", merged,
                                   "--flash_mode", "dio", "--flash_size", "4MB", *parts], check=True)

    write_package(out, merged, version)
    print(f"[web-installer] {os.path.relpath(merged, ROOT)} ({os.path.getsize(merged)} bytes), versão {version}")


if __name__ == "__main__":
    main()
