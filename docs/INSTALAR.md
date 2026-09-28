# Instalar o firmware

[README](../README.md) · [Montar o seu](GUIA-MONTAGEM.md) · [Como jogar](JOGO.md) · [Editor](EDITOR.md) · [Hardware](HARDWARE.md) · [Desenvolvimento](DESENVOLVIMENTO.md) · [Prompts para IA](PROMPTS-IA.md) · [English](en/INSTALL.md)

Há dois jeitos de gravar o PixelGotchi na placa. **Para a maioria das pessoas,
o instalador pelo navegador basta**: não precisa instalar nada no computador.
A instalação manual serve para quem vai alterar o código ou a arte, ou para
quem não tem Chrome/Edge.

## Pelo navegador (recomendado)

Precisa de: a placa, um **cabo USB-C que transmita dados** (alguns cabos só
carregam) e **Chrome ou Edge em um computador** (Windows, macOS, Linux ou
ChromeOS). Celular e Firefox/Safari não têm acesso à porta USB (Web Serial).

1. Abra o [instalador do PixelGotchi](https://pantojinho.github.io/PixelGotchi/install.html).
2. Conecte a placa no computador.
3. Marque que entendeu o aviso: a instalação **apaga a placa**, inclusive um
   pet já salvo nela.
4. Clique no botão, escolha a porta da placa na janela do navegador e depois
   **Install**. Acompanhe o progresso até o fim.
5. Toque em **RESET** na placa (com BOOT solto). A seleção de bichinhos aparece.

Antes de liberar o botão, a página confere versão, tamanho e SHA-256 do
pacote, e a gravação usa exatamente esses bytes. Se o arquivo estiver ausente
ou inconsistente, a instalação fica bloqueada e a página oferece uma nova
tentativa. O firmware do site é sempre o da versão mais recente do `master`,
compilado automaticamente pelo GitHub Actions.

**A porta não aparece?** Feche outros programas que usam a placa (monitor
serial, Arduino IDE), troque o cabo e tente o modo de gravação: segure
**BOOT**, toque em **RESET**, solte **BOOT** e escolha a porta de novo.

O instalador grava o jogo PixelGotchi como ele está no repositório e **apaga
a placa inteira**, inclusive o pet salvo e o bichinho enviado pelo editor.
Para mandar um bichinho seu depois da instalação, use o [editor](EDITOR.md):
ele envia só a arte, sem regravar o firmware.

## Instalação manual via USB

Você precisa da placa (veja [Hardware](HARDWARE.md)), de um **cabo USB-C com dados**, acesso à internet
para baixar as ferramentas e um computador Windows, macOS ou Linux.
A gravação substitui o programa de demonstração que veio na placa.

### 1. Baixe o projeto e as ferramentas

Instale [Git](https://git-scm.com/downloads) e [Python 3.12](https://www.python.org/downloads/).
No Windows, habilite o Python no PATH durante a instalação. Abra o terminal
na pasta onde deseja guardar o projeto e execute:

```sh
git clone https://github.com/pantojinho/PixelGotchi.git
cd PixelGotchi
```

Alternativa sem Git: no GitHub, clique **Code → Download ZIP**, extraia tudo
e abra o terminal na pasta que contém `platformio.ini`.

Instale o PlatformIO em um ambiente Python **ao lado** do repositório.
Assim as dependências não entram nos arquivos do projeto.

**Windows — PowerShell:**

```powershell
py -3.12 -m venv ..\pixelgotchi-env
..\pixelgotchi-env\Scripts\python.exe -m pip install platformio==6.2.0
```

**macOS / Linux:**

```sh
python3 -m venv ../pixelgotchi-env
../pixelgotchi-env/bin/python -m pip install platformio==6.2.0
```

Nos próximos exemplos, use o caminho do Python do seu ambiente. Não precisa
ativá-lo. A instalação pode levar alguns minutos na primeira execução.
Referência: [instalação do PlatformIO Core](https://docs.platformio.org/en/stable/core/installation/methods/installer-script.html).

### 2. Conecte e identifique a porta

Ligue o USB-C da placa ao computador. Feche monitores seriais que estejam
usando a placa. Liste os dispositivos:

```powershell
# Windows
..\pixelgotchi-env\Scripts\python.exe -m platformio device list
```

```sh
# macOS / Linux
../pixelgotchi-env/bin/python -m platformio device list
```

Anote a porta que aparece ao conectar e desaparece ao desconectar a placa.
Exemplos: `COM5` no Windows, `/dev/cu.usbmodem...` no macOS ou
`/dev/ttyACM0` no Linux. **São exemplos: use a sua porta.** Se houver várias
placas, identifique esta antes de gravar.
Referências: [listar dispositivos](https://docs.platformio.org/en/stable/core/userguide/device/cmd_list.html)
e [porta de upload](https://docs.platformio.org/en/stable/projectconf/sections/env/options/upload/upload_port.html).

### 3. Compile e grave

**Windows — substitua `COM5` pela sua porta:**

```powershell
..\pixelgotchi-env\Scripts\python.exe -m platformio run -e esp32-s3-matrix
..\pixelgotchi-env\Scripts\python.exe -m platformio run -e esp32-s3-matrix -t upload --upload-port COM5
```

**macOS / Linux — substitua `/dev/ttyACM0` pela sua porta:**

```sh
../pixelgotchi-env/bin/python -m platformio run -e esp32-s3-matrix
../pixelgotchi-env/bin/python -m platformio run -e esp32-s3-matrix -t upload --upload-port /dev/ttyACM0
```

Espere **`[SUCCESS]`** na compilação e na gravação. O PlatformIO baixa o
compilador e as bibliotecas e gera a arte automaticamente. Use o
`platformio.ini` do projeto: o ambiente já configura **flash de 4 MB**,
FastLED 3.6.0 e USB CDC. O nome genérico `esp32-s3-devkitc-1` dentro desse
arquivo é intencional; as adaptações da Matrix estão no próprio projeto.

Se a gravação não conectar, coloque a placa no modo de download:

1. Segure **BOOT**.
2. Pressione e solte **RESET**, mantendo BOOT pressionado.
3. Solte **BOOT**.
4. Liste as portas novamente — o nome pode mudar — e repita o upload.
5. Ao terminar, pressione **RESET** com BOOT solto para iniciar o jogo.

Esse é o modo de gravação do ESP32-S3 descrito pela
[Espressif](https://docs.espressif.com/projects/esptool/en/latest/esp32s3/advanced-topics/boot-mode-selection.html).
Durante o jogo, use BOOT depois da inicialização.

### 4. Confira que iniciou

Abra o monitor e, se necessário, toque RESET para ver a inicialização:

```powershell
# Windows — sua porta pode mudar depois do upload
..\pixelgotchi-env\Scripts\python.exe -m platformio device monitor --port COM5 --baud 115200
```

```sh
# macOS / Linux
../pixelgotchi-env/bin/python -m platformio device monitor --port /dev/ttyACM0 --baud 115200
```

Saída esperada: `[PixelGochi] iniciando...`, `[Imu] QMI8658 ok` e
`[Game] fase=...`. Use `Ctrl+C` para fechar o monitor antes de outro upload.
Na matriz, escolha o pet com cliques; segure BOOT por 0,6 s e solte para
começar o ovo. Movimentos suaves acumulam os cinco minutos de incubação.

| Problema | O que conferir |
|---|---|
| Nenhuma porta aparece | Troque por um cabo com dados e outra porta USB; tente BOOT + RESET e liste de novo |
| Porta ocupada | Feche o monitor serial, outras IDEs e aplicativos que usam essa porta |
| Upload não conecta | Confira a porta atual e use o modo de download acima |
| Linux retorna permissão negada | Ajuste a permissão/grupo da porta conforme sua distribuição; reconecte após a mudança |
| Monitor vazio | Use 115200, confira a porta após o reset e deixe BOOT solto |
| IMU não respondeu | Confirme o modelo exato da placa; o BOOT funciona, mas os gestos exigem o sensor |
| Imagem ou inclinação invertida | Veja `DISPLAY_ROTATION`, `DISPLAY_MIRROR_X` e a calibração em [Hardware](HARDWARE.md) |

Essa placa usa USB nativo do ESP32-S3; não presuma que precisa de um driver
CH340 de outra placa. Para problemas de reconhecimento, consulte a
[documentação da sua Matrix](https://docs.waveshare.com/ESP32-S3-Matrix/Arduino).

**Alternativa gráfica:** abra a pasta do projeto no
[VS Code](https://code.visualstudio.com/) com a extensão
[PlatformIO IDE](https://platformio.org/install/ide?install=vscode).
Em **Project Tasks → esp32-s3-matrix**, execute **Build** e **Upload**;
use o monitor em 115200. Com mais de uma porta disponível, prefira os
comandos acima para escolher explicitamente a placa.

### Atualizar uma instalação existente

Com uma cópia obtida por Git, confira suas alterações antes de atualizar:

```sh
git status
git pull --ff-only
```

Se há alterações locais, guarde-as antes do pull. Depois repita a compilação
e o upload com a porta atual. Na cópia ZIP, baixe e extraia a nova versão
separadamente. O upload normal não manda apagar toda a flash. A retenção
do pet depende da compatibilidade do formato salvo e das partições entre
versões; não é garantia de migração para qualquer firmware futuro.

**Verificado no software:** build, testes C++ e preview; veja as evidências
em [TESTES.md](TESTES.md). Gravação USB, gestos e aparência real ainda
precisam da execução dos testes na placa física.
