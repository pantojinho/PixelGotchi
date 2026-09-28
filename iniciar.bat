@echo off
rem Abre o PixelGotchi no navegador (simulador, guia e instalador) em localhost.
rem Precisa so do Python 3. Feche esta janela ou use Ctrl+C para parar.
cd /d "%~dp0"
where py >nul 2>nul && (py -3 tools\preview.py %* & goto fim)
where python >nul 2>nul && (python tools\preview.py %* & goto fim)
echo Python 3 nao encontrado. Instale em https://www.python.org/downloads/
echo e marque "Add Python to PATH" durante a instalacao.
:fim
pause
