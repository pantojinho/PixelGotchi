#!/usr/bin/env python3
"""Testes C++ sem placa: python tools/test_controls.py (g++/clang++ ou CXX).

Alternativa: ZIG_BINARY=/caminho/zig usa o compilador C++ do Zig.
Os mocks substituem somente relógio, GPIO, sensor, NVS e saída LED.
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parent.parent
zig = os.environ.get('ZIG_BINARY')
compiler = [zig, 'cc', '-x', 'c++'] if zig else [os.environ.get('CXX') or shutil.which('g++') or shutil.which('clang++') or 'g++']
sources = ['test/test_controls.cpp', 'src/PetSim.cpp', 'src/Events.cpp', 'src/Canvas.cpp',
           'src/Display.cpp', 'src/art/ArtData.cpp']
with tempfile.TemporaryDirectory(prefix='pixelgotchi-tests-') as scratch:
    exe = Path(scratch) / ('controls.exe' if os.name == 'nt' else 'controls')
    subprocess.run(compiler + ['-std=c++17', '-Wall', '-Wextra', '-Itest/stubs', '-Isrc'] + sources + ['-o', str(exe)], cwd=root, check=True)
    subprocess.run([str(exe)], check=True)
