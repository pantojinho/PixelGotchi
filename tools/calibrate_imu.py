#!/usr/bin/env python3
"""Calibra o acelerômetro (QMI8658) em relação à tela e gera src/ImuCalib.h.

Com a placa conectada e o firmware gravado:

    python tools/calibrate_imu.py            # guiado: pede cada posição e aperta Enter
    python tools/calibrate_imu.py --port COM9

Também dá pra capturar uma posição por vez (útil pra automatizar):

    python tools/calibrate_imu.py --step flat    # tela pra cima, deitada
    python tools/calibrate_imu.py --step right   # deitada, lado DIREITO da imagem mais baixo
    python tools/calibrate_imu.py --step left    # deitada, lado ESQUERDO mais baixo
    python tools/calibrate_imu.py --step down    # tela virada pra mesa
    python tools/calibrate_imu.py --solve        # calcula e grava src/ImuCalib.h

Depois é só gravar o firmware de novo (pio run -t upload).
"""
import argparse
import json
import os
import sys
import time

import serial
import serial.tools.list_ports

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SAMPLES = os.path.join(ROOT, "tools", ".imu_samples.json")
OUT_H = os.path.join(ROOT, "src", "ImuCalib.h")
AXES = "xyz"

STEPS = {
    "flat": "Deite a placa na mesa com a TELA PRA CIMA, a imagem de pé pra você.",
    "right": "Ainda deitada, levante o lado ESQUERDO uns 30-45 graus (lado DIREITO da imagem mais baixo).",
    "left": "Agora o contrário: levante o lado DIREITO (lado ESQUERDO da imagem mais baixo).",
    "down": "Vire a placa com a TELA PRA BAIXO, encostada na mesa.",
}


def find_port():
    for p in serial.tools.list_ports.comports():
        if p.vid == 0x303A:  # Espressif
            return p.device
    sys.exit("Placa não encontrada (nenhuma porta Espressif). Use --port COMx.")


def open_board(port):
    s = serial.Serial()
    s.port, s.baudrate, s.timeout = port, 115200, 0.2
    s.dtr = s.rts = False  # não resetar a placa ao abrir
    s.open()
    return s


def capture(port, seconds=1.5):
    s = open_board(port)
    try:
        s.write(b"imu on\n")
        s.reset_input_buffer()
        vals, deadline, buf = [], time.time() + seconds + 3, b""
        start = None
        while time.time() < deadline:
            buf += s.read(512)
            *lines, buf = buf.split(b"\n")
            for ln in lines:
                parts = ln.decode(errors="ignore").strip().split(",")
                if len(parts) == 5 and parts[0] == "A":
                    start = start or time.time()
                    vals.append([float(v) for v in parts[2:]])
            if start and time.time() - start >= seconds:
                break
        s.write(b"imu off\n")
    finally:
        s.close()
    if len(vals) < 5:
        sys.exit("Não chegou leitura do sensor. O firmware atual está gravado? (precisa do comando 'imu on')")
    avg = [sum(v[i] for v in vals) / len(vals) for i in range(3)]
    spread = max(max(v[i] for v in vals) - min(v[i] for v in vals) for i in range(3))
    return avg, spread


def load():
    if os.path.exists(SAMPLES):
        with open(SAMPLES, encoding="utf-8") as f:
            return json.load(f)
    return {}


def save(data):
    with open(SAMPLES, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=1)


def do_step(port, name):
    avg, spread = capture(port)
    data = load()
    data[name] = avg
    save(data)
    warn = "  (a placa mexeu durante a leitura — repita parada)" if spread > 3 else ""
    print(f"{name}: x={avg[0]:6.2f} y={avg[1]:6.2f} z={avg[2]:6.2f} m/s²{warn}")


def solve():
    d = load()
    missing = [k for k in ("flat", "right") if k not in d]
    if missing:
        sys.exit(f"Faltam as posições: {', '.join(missing)}")
    flat, right = d["flat"], d["right"]
    screen = max(range(3), key=lambda i: abs(flat[i]))
    screen_sign = 1.0 if flat[screen] > 0 else -1.0
    if abs(flat[screen]) < 7:
        print("Aviso: com a tela pra cima a gravidade não caiu num eixo só; a placa estava bem deitada?")
    if "down" in d and d["down"][screen] * screen_sign > -5:
        print("Aviso: 'tela pra baixo' não inverteu o eixo da tela como esperado.")

    diff = [right[i] - flat[i] for i in range(3)]
    tilt = max((i for i in range(3) if i != screen), key=lambda i: abs(diff[i]))
    tilt_sign = 1.0 if diff[tilt] > 0 else -1.0
    if abs(diff[tilt]) < 2:
        print("Aviso: a inclinação pra direita mudou pouco; incline mais (30-45 graus) e repita.")
    if "left" in d and (d["left"][tilt] - flat[tilt]) * tilt_sign > 0:
        print("Aviso: esquerda e direita deram o mesmo sinal — alguma das duas foi feita ao contrário?")

    content = f"""// GERADO por tools/calibrate_imu.py a partir das leituras da placa.
// Pra refazer: python tools/calibrate_imu.py
#pragma once
#include <stdint.h>

// Eixo (0=x, 1=y, 2=z) em que a gravidade aparece com a tela pra cima, e o
// sinal que deixa esse valor positivo nessa posição.
constexpr uint8_t SCREEN_AXIS = {screen};  // {AXES[screen]}
constexpr float SCREEN_SIGN = {screen_sign:+.1f}f;

// Eixo que muda ao inclinar pra direita/esquerda, e o sinal que deixa
// "lado direito pra baixo" positivo.
constexpr uint8_t TILT_AXIS = {tilt};  // {AXES[tilt]}
constexpr float TILT_SIGN = {tilt_sign:+.1f}f;
"""
    with open(OUT_H, "w", encoding="utf-8", newline="\n") as f:
        f.write(content)
    print(f"Tela: eixo {AXES[screen]} (sinal {screen_sign:+.0f}) · Inclinação: eixo {AXES[tilt]} (sinal {tilt_sign:+.0f})")
    print(f"Gravado em {os.path.relpath(OUT_H, ROOT)}. Grave o firmware de novo: pio run -t upload")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port")
    ap.add_argument("--step", choices=STEPS)
    ap.add_argument("--solve", action="store_true")
    a = ap.parse_args()

    if a.solve:
        return solve()
    port = a.port or find_port()
    if a.step:
        return do_step(port, a.step)

    print(f"Calibrando o acelerômetro na {port}. Em cada passo, posicione e aperte Enter (deixe parada).\n")
    for name, text in STEPS.items():
        input(f"{text}\n  [Enter] ")
        do_step(port, name)
        print()
    solve()


if __name__ == "__main__":
    main()
