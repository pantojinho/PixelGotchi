#!/bin/sh
# Abre o PixelGotchi no navegador (simulador, guia e instalador) em localhost.
# Precisa só do Python 3. Ctrl+C para parar.
cd "$(dirname "$0")" || exit 1
if command -v python3 >/dev/null 2>&1; then PY=python3
elif command -v python >/dev/null 2>&1; then PY=python
else
  echo "Python 3 não encontrado. Instale em https://www.python.org/downloads/ e tente de novo."
  exit 1
fi
exec "$PY" tools/preview.py "$@"
