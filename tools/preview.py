#!/usr/bin/env python3
"""Abre o PixelGotchi no navegador (página "Comece aqui", simulador e instalador)
e atualiza sozinho ao salvar art/*.art.

    python tools/preview.py            (porta 8765)
    python tools/preview.py 9000       (outra porta)

Ctrl+C pra parar.
"""
import functools
import http.server
import os
import subprocess
import sys
import threading
import time
import webbrowser

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ART_DIR = os.path.join(ROOT, "art")
PREVIEW_DIR = os.path.join(ROOT, "preview")
GEN = os.path.join(ROOT, "tools", "gen_art.py")


def generate():
    subprocess.run([sys.executable, GEN])


def art_mtimes():
    return {f: os.path.getmtime(os.path.join(ART_DIR, f)) for f in os.listdir(ART_DIR) if f.endswith(".art")}


def watch():
    last = art_mtimes()
    while True:
        time.sleep(0.5)
        now = art_mtimes()
        if now != last:
            last = now
            generate()  # a página percebe o art.js novo e recarrega


class QuietHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *args):
        pass

    def end_headers(self):
        self.send_header("Cache-Control", "no-store")
        super().end_headers()


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8765
    generate()
    threading.Thread(target=watch, daemon=True).start()
    handler = functools.partial(QuietHandler, directory=PREVIEW_DIR)
    server = http.server.ThreadingHTTPServer(("127.0.0.1", port), handler)
    url = f"http://localhost:{port}/"
    print(f"[preview] {url}guia.html  (comece aqui)")
    print(f"[preview] {url}  (simulador; salve um arquivo em art/ e a página atualiza)")
    print("[preview] Ctrl+C pra sair")
    webbrowser.open(url + "guia.html")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
