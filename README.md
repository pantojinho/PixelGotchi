# PixelGochi

Um bichinho virtual (tipo Tamagotchi) em pixel art numa matriz 8x8 de
LEDs, rodando na **Waveshare ESP32-S3-Matrix**. Você escolhe o bicho,
choca o ovo mexendo na placa, e cuida dele com o botão BOOT e gestos
(chacoalhar, inclinar, virar de cara pra baixo).

## Editar a arte e ver no simulador

Toda a arte fica em texto, em `art/*.art`, e é a mesma fonte usada pelo
firmware e pelo simulador do navegador — o que você vê no simulador é o
que vai pra matriz.

```bash
python tools/preview.py
```

Abre o simulador em `http://localhost:8765` e fica observando a pasta
`art/`: salvou um `.art`, a página atualiza sozinha. `Ctrl+C` pra parar.

Atalhos de URL do simulador:

| URL | Mostra |
|---|---|
| `?view=scenes` | só as cenas (comendo, com fome, dormindo, feliz, triste, doente, selvagem, RIP) |
| `?view=sprites&only=cat` | todos os frames de um bicho, parados |
| `?only=axo&cell=30` | um bicho só, com células maiores |
| `?mode=flat` | cores "de design", sem simular o LED |

O controle de **brilho** no topo simula o `MAX_BRIGHTNESS` do firmware
(padrão 30): cores escuras perdem tom na matriz real, então confira nele.

### Formato do `.art`

```
palette capy            # paleta: um caractere por cor
  B #B25A22
  D #6E300C
end

palette capy_wild : capy   # variante: mesmas letras, cores trocadas
  B #6E4424
end

sprite capy_idle0 capy  # '.' = LED apagado
...D..
.BBLLL
end

anim capy_idle 450 capy_idle0 capy_idle0 capy_ear   # ms por frame + frames
pet capy "Capivara" food=food_melon wild=capy_wild side=1
```

Cada bicho precisa das animações `_idle _blink _walk _eat _sleep
_happy _sad _hungry _egg`. O gerador (`tools/gen_art.py`) valida tudo e
avisa o que faltar; ele roda sozinho a cada build do PlatformIO.

Dicas de pixel art pra LED: preto é LED apagado (não use contorno
escuro), olhos funcionam melhor como "buraco", use 2–4 cores saturadas
por bicho e deixe o bicho com ~5–6 px de largura pra sobrar espaço pra
comida e efeitos.

## Como se joga

| Tela | Ação |
|---|---|
| Escolha do bicho | clique ou inclinar = próximo · segurar = escolher |
| Ovo | mexer/chacoalhar pra chocar (10 min de movimento; parado pausa) · clique = barra de progresso |
| Vida | clique = menu (clique = próximo, segurar = confirmar): comer, brincar, limpar, remédio, luz, status · segurar = status/idade · chacoalhar = carinho · virar de cara pra baixo = apagar a luz · inclinar = ele anda pro lado mais baixo |
| RIP | segurar = ovo novo |
| Qualquer tela | segurar 8 s = recomeçar do zero (barra vermelha aos 3 s) |

Regras (ajustáveis em `src/Config.h`):
- fome, alegria e energia caem com o tempo; cocô sem limpar e fome zerada
  deixam doente; bem cuidado, vive pra sempre;
- descuido acumula em estágios; passou do limite, vira **selvagem**
  (arisco, se vira sozinho, cores de terra); 5 dias selvagem = **RIP**;
- cada ovo nasce com um **DNA** (MAC da placa + instante do clique) que
  muda metabolismo, carência, jeito de agir sozinho e um leve tom de cor;
- com a placa desligada o tempo fica pausado (não há relógio com bateria);
- `TIME_SCALE` acelera o relógio do jogo pra testar.

## Compilar e gravar

Com [PlatformIO](https://platformio.org/) (extensão do VS Code, ou
`pip install platformio`):

```bash
pio run -t upload
```

## Hardware

[Waveshare ESP32-S3-Matrix](https://docs.waveshare.com/ESP32-S3-Matrix)
(ESP32-S3FH4R2: 4 MB de flash, 2 MB de PSRAM), matriz de 64 WS2812B,
IMU QMI8658, botões BOOT/RESET, USB-C nativo.

| Componente | Pino |
|---|---|
| Matriz WS2812B | GPIO14 (fiação progressiva: linha×8 + coluna) |
| IMU QMI8658 (I2C, endereço 0x6B) | SDA=GPIO11, SCL=GPIO12 |
| Botão BOOT | GPIO0 |

Os pinos internos não constam no pinout público; vieram de
[shantanugoel/pomodoro_cube](https://github.com/shantanugoel/pomodoro_cube),
feito pra essa placa.

⚠️ **Brilho:** a Waveshare avisa que brilho alto esquenta e pode
danificar a placa — já aconteceu aqui com uma versão do FastLED que caiu
num driver por software e mandou tudo branco no máximo. Por isso o
FastLED está fixo na 3.6.0, e há teto de brilho, limite de corrente e
taxa de quadros limitada em `src/Config.h`. Não suba esses valores sem
cuidado.

## Estrutura

```
art/            pixel art em texto (fonte única da verdade)
preview/        simulador no navegador (art.js é gerado)
tools/          gen_art.py (gerador), preview.py (simulador com auto-atualização)
src/
  Config.h      pinos, segurança dos LEDs, regras do jogo
  Game.cpp      cenas, menu, vida autônoma, desenho
  PetSim.cpp    regras: stats, cocô, doença, sono, descuido, selvagem, RIP
  Dna.h         traços de personalidade a partir do DNA
  Canvas.cpp    framebuffer 8x8, sprites, texto
  Display.cpp   FastLED + orientação da matriz
  Imu.cpp       gestos (mexer, chacoalhar, inclinar, virar)
  Input.cpp     botão BOOT (curto, longo, reset)
  Storage.cpp   salva o estado na flash (NVS)
  art/          dados gerados a partir de art/*.art (não editar)
```

## Roadmap

- **Fase 2:** configuração pelo celular (a placa cria uma rede Wi-Fi):
  nome do bicho, escolha, Wi-Fi de casa pra pegar a hora da internet →
  vida offline e ciclo dia/noite de verdade.
- **Fase 3:** evolução Ovo → Bebê → Adulto com ramos (dócil, atlético,
  selvagem) conforme o tipo de interação.
- Importar PNGs desenhados no LibreSprite/Aseprite pro `art/`.

## Licença

MIT — veja [LICENSE](LICENSE).
