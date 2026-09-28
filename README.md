# PixelGochi

Um bichinho virtual (tipo Tamagotchi) rodando standalone na **Waveshare
ESP32-S3-Matrix**: nasce de um ovo que você precisa "chocar" chacoalhando
a placa por uns minutos, depois vive na matriz 8x8 em pixel art. Sem
Wi-Fi, sem botões extras — só o giroscópio/acelerômetro embutido e o
botão BOOT.

## Hardware

[Waveshare ESP32-S3-Matrix](https://docs.waveshare.com/ESP32-S3-Matrix)
(chip **ESP32-S3FH4R2**): matriz 8x8 de 64 LEDs WS2812B, IMU QMI8658
(acelerômetro + giroscópio), botões BOOT/RESET, USB-C.

| Componente        | Pino(s)                    |
|-------------------|-----------------------------|
| Matriz LED WS2812B | GPIO14 (dado)              |
| IMU QMI8658 (I2C) | SDA=GPIO11, SCL=GPIO12, INT=GPIO13 (não usado no v1) |
| Botão BOOT        | GPIO0                       |

Esses pinos não constam no pinout público da Waveshare (só aparecem os
GPIOs expostos nos headers laterais); vieram confirmados de um projeto
de terceiros feito especificamente para esta placa
([shantanugoel/pomodoro_cube](https://github.com/shantanugoel/pomodoro_cube)).

⚠️ **A Waveshare avisa**: não deixe o brilho da matriz muito alto — ela
esquenta rápido e pode danificar a placa. O firmware já limita o
brilho em `MAX_BRIGHTNESS` (`src/Config.h`); não aumente sem cuidado.

## Como funciona (v1)

1. **Ovo** — ao ligar pela primeira vez, começa como ovo. Chacoalhe a
   placa: o ovo só "esquenta" enquanto há movimento recente (parar por
   mais de 15s pausa o progresso, não zera). São necessários **10
   minutos acumulados de movimento** pra eclodir. A casca vai rachando
   em 4 estágios conforme o progresso.
2. **Eclosão** — ao nascer, sorteia uma espécie (cor/formato) entre as
   cadastradas em `SPECIES_TABLE` (`src/Pet.h`).
3. **Vida** — fome, felicidade e energia decaem 1 ponto por minuto.
   A carinha muda sozinha: faminto, triste, sonolento (olhos fechados),
   feliz.
4. **Interações**:
   - **Chacoalhar** a placa → brincar (felicidade↑, energia↓ um pouco)
   - **Virar de cabeça pra baixo** e segurar ~1s → dormir (energia
     regenera mais rápido); voltar à posição normal acorda
   - **Clique curto no BOOT** → alimentar (fome↑)
   - **Clique longo no BOOT** (~1,2s) → alternar dormir/acordar na mão,
     caso o gesto de virar não funcione bem na sua unidade

Não há mecânica de "morte" no v1 — se descuidar, o bichinho só fica
triste/faminto até você voltar a cuidar dele.

## Compilar e gravar

Recomendado: **VS Code + extensão PlatformIO**.

1. Abra a pasta `PixelGochi` no VS Code com a extensão PlatformIO
   instalada.
2. Conecte a placa via USB-C.
3. PlatformIO → Build, depois Upload (ou o atalho da barra inferior).
4. Abra o Monitor Serial (115200 baud) pra ver os logs.

Alternativa: Arduino IDE, selecionando a placa "ESP32S3 Dev Module" e
instalando manualmente as libs listadas em `platformio.ini` (`Adafruit
NeoPixel`, `Adafruit GFX Library`, `Adafruit NeoMatrix`, `SensorLib`).

Não tenho como compilar/testar isso sem a placa física em mãos — o
código foi escrito com base na documentação oficial da Waveshare e em
projetos de terceiros para o mesmo hardware, mas é bem possível que a
primeira compilação precise de pequenos ajustes (versão de lib,
assinatura de função). Me manda o erro que eu ajusto.

## Primeira execução: calibração

Duas coisas dependem da unidade física e não dá pra confirmar sem
testar:

1. **Ordem de varredura da matriz** — segure o botão **BOOT** durante
   o boot (não durante um reset pra gravação!) pra rodar um teste que
   acende os 4 cantos em sequência (vermelho, verde, azul, branco),
   1s cada. Se a posição/ordem não bater com o desenho físico da
   matriz, ajuste as flags do `Adafruit_NeoMatrix` em `Display.cpp`
   (comentário no topo do arquivo explica).
2. **Endereço/eixo do IMU** — o firmware já tenta os dois endereços
   I2C possíveis do QMI8658 automaticamente. Se o "chacoalhar" ou o
   "virar" não estiverem respondendo bem, descomente `#define
   DEBUG_IMU` em `src/Config.h`, olhe os valores no serial monitor e
   ajuste os limiares (`SHAKE_THRESHOLD_MPS2`, `FLIP_Z_THRESHOLD_MPS2`)
   ou troque o eixo usado no flip, se o IMU estiver montado em outra
   orientação.

## Estrutura do código

```
src/
  Config.h     pinos, limiares, constantes ajustáveis
  PixelArt.h   sprites 8x8 (ovo + criatura) como grades de paleta
  Pet.h/.cpp   estado e regras do bichinho (a única parte que muda o estado)
  Display.h/.cpp  desenha na matriz via Adafruit_NeoMatrix
  Imu.h/.cpp   leitura do QMI8658, detecção de gestos
  Input.h/.cpp botão BOOT (clique curto/longo)
  Storage.h/.cpp  salva/carrega estado na NVS (sobrevive a reset/queda de energia)
  main.cpp     liga tudo
```

## Roadmap (ideias pra v2+)

- App web (o ESP32 vira AP + servidor) pra ver status/interagir pelo
  celular
- Mais espécies com formas diferentes (hoje só variam de cor)
- Mini-jogos simples usando os gestos
- Mecânica de "doente"/revive opcional

## Licença

MIT — veja [LICENSE](LICENSE).
