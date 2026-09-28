# PixelGotchi

Bichinho virtual na **Waveshare ESP32-S3-Matrix**, com matriz RGB de
**8×8 LEDs**, botão **BOOT** e acelerômetro **QMI8658**. Funciona sozinho,
sem Wi-Fi, celular ou botões adicionais.

## Pets e visual

Capivara, gato, sapo, pintinho, coelho e axolote. Cada espécie tem
animações de descanso, caminhada, piscar, comer, dormir, felicidade,
tristeza e fome, além de uma paleta de ovo e uma variante selvagem.

- **Gato:** sentado, orelhas triangulares com interior rosa, olhos
  apagados, focinho e peito creme, patas e cauda que se mexe.
- **Capivara:** de perfil, corpo arredondado, orelha pequena, focinho
  comprido e largo, nariz separado e pernas curtas. Quando feliz,
  equilibra uma tangerina na cabeça.
- Comida, corações e avisos aparecem **entre poses**, preservando a
  silhueta e o rosto mesmo com apenas 64 pixels. Dormir usa uma pose
  mais baixa e reduz o brilho.
- A capivara descansa e fareja mais; o gato observa e persegue mais.
  O DNA continua variando personalidade e tonalidade de cada indivíduo.

Os desenhos são originais, feitos diretamente na grade de pixels.
Referências de forma: [perfil de capivara (WWF)](https://www.wwf.or.jp/staffblog/news/5510.html)
e [silhueta de gato sentado](https://freesvg.org/black-cat-vector-image).
As imagens de referência não são distribuídas no projeto.

## Controles

**Uma regra para os menus: clique troca, segurar e soltar confirma.**
Após **0,6 s**, um ponto verde no canto superior direito indica que já
pode soltar. A ação longa só acontece ao soltar o botão.

| Situação | Entrada | Resultado |
|---|---|---|
| Seleção inicial | Clique no BOOT ou incline para um lado | Próxima espécie; inclinar à esquerda volta |
| Seleção inicial | Segure BOOT por 0,6 s e solte | Escolhe a espécie e começa o ovo |
| Ovo | Movimente suavemente a placa | Acumula incubação: 10 minutos; parar por 15 s pausa, sem zerar |
| Ovo | Clique no BOOT | Mostra o progresso no topo por 2 s |
| Pet acordado | Clique no BOOT | Alimenta, com animação de comida → mastigação → coração |
| Pet dormindo | Clique no BOOT | Acorda; não alimenta junto |
| Vida | Segure BOOT por 0,6 s e solte | Abre o menu no cuidado mais urgente |
| Menu | Clique ou incline | Troca o ícone; volte ao centro antes da próxima inclinação |
| Menu | Segure BOOT por 0,6 s e solte | Executa o cuidado selecionado |
| Vida | Chacoalhe | Brinca: alegria +20, energia −8, saciedade −3; pausa de 5 s entre gestos |
| Vida | Incline lateralmente | O pet acompanha o lado mais baixo |
| Vida | Vire a matriz para baixo por 1,5 s | Dorme |
| Sono iniciado pelo gesto | Desvire | Acorda; o sono escolhido no menu continua até BOOT/menu ou energia cheia |
| Status | Clique | Barras → idade → volta ao pet |
| Qualquer cena | Segure BOOT por 8 s | Recomeça na seleção; barra vermelha a partir de 3 s |
| Após a morte | Segure BOOT por 0,6 s e solte | Recomeça na seleção |

### Menu em 8×8

Ordem dos oito ícones/pontos: **comida · bola · limpeza · remédio · lua ·
coração · barras · voltar**. O ponto branco na última linha indica a posição.
O menu fecha após 8 s sem entrada. Sem IMU, todos os cuidados continuam
acessíveis pelo BOOT.

O menu sugere acordar se estiver dormindo; caso contrário, prioriza doença,
sujeira, fome, cansaço e tristeza. Quando está tudo bem, sugere carinho.
Carinho aumenta a alegria em 5 pontos sem gastar energia.

Sacudidas não interrompem uma animação de cuidado nem acordam o pet.
Comida é recusada se ele já estiver cheio; brincadeira, se faltar energia.
Segurar para reset não executa uma confirmação intermediária.

## Preview no navegador

Para abrir automaticamente e recarregar ao salvar a arte:

```sh
python tools/preview.py
```

Esse comando serve `preview/` em `http://localhost:8765/`. Para escolher
outra porta: `python tools/preview.py 9000`. Pare com `Ctrl+C`.

Para servir o repositório inteiro sem recarga automática:

```sh
python tools/gen_art.py
python -m http.server 8765 --bind 127.0.0.1
```

Abra [o preview local](http://127.0.0.1:8765/preview/). Também é possível
abrir `preview/index.html` diretamente no navegador.

- **Maquete interativa:** escolha a espécie e experimente BOOT, inclinação,
  chacoalhada e sono. **Espaço** funciona como BOOT; as **setas** inclinam.
  O seletor de necessidade permite experimentar fome, tristeza, cansaço,
  sujeira e doença.
- **Galeria:** cenas e todas as poses animadas; clique na matriz para ampliar.
- **Modo LED:** aproxima a aparência com brilho baixo; modo de design
  mostra a paleta original. O brilho do preview não altera o firmware.
- Filtros: `?only=cat`, `?only=capy`, `?view=scenes`, `?view=sprites`.

A maquete usa os mesmos sprites e reproduz os controles dos cuidados.
Ela não executa o firmware nem simula seu relógio, DNA, nascimento,
descuido ou persistência. A aparência óptica real depende dos LEDs.

## Vida e persistência

O estado fica salvo na NVS e sobrevive a reinícios. A fome, felicidade e
energia decaem lentamente, com taxas e personalidade em `src/Config.h`
e `src/Dna.h`. Durante o sono, a energia regenera e a fome cai mais devagar.
O relógio só avança enquanto a placa está ligada.

Sujeira e fome prolongada causam doença. O descuido acumula em estágios:
normal → descuidado → quase selvagem → selvagem. O pet selvagem fica arisco,
procura comida sozinho e pode morrer após cinco dias nessa condição.
Bem cuidado, vive indefinidamente. Esses cuidados já existiam no projeto.

## Hardware e brilho

[Documentação da placa](https://docs.waveshare.com/ESP32-S3-Matrix).

| Componente | Pinos |
|---|---|
| 64 LEDs WS2812B | GPIO14 |
| QMI8658, I²C | SDA GPIO11, SCL GPIO12 |
| BOOT | GPIO0 |

O firmware limita o brilho a **30/255**, a corrente da matriz a **400 mA**
e usa FastLED **3.6.0**, com driver RMT e sem dithering temporal.
A Waveshare informa que brilho excessivo aquece e pode danificar a placa.

O BOOT pressionado durante reset/energização entra no modo de gravação
do ESP32. Para interagir, pressione-o **depois** que o firmware iniciar.

Orientação: `DISPLAY_ROTATION` e `DISPLAY_MIRROR_X` em `src/Config.h`.
Se a inclinação estiver invertida, ajuste `TILT_SIGN`. Sensibilidade e
tempo dos gestos também ficam nesse arquivo. O IMU tenta os endereços
0x6B e 0x6A. A primeira amostra não é contada como movimento e o gesto
de virar usa histerese para evitar oscilar entre dormir/acordar.

## Compilar e gravar

Use VS Code com PlatformIO, ou:

```sh
python -m pip install platformio
python -m platformio run
python -m platformio run --target upload
python -m platformio device monitor
```

A geração da arte é automática antes da compilação. A configuração usa
flash de **4 MB** e USB CDC para o monitor serial a 115200 baud.

**Verificado nesta revisão:** compilação para `esp32-s3-matrix`, testes
C++ dos controles com hardware simulado e interações no preview.
Os gestos, a orientação e a aparência nos LEDs ainda precisam de teste
na placa física.

## Testes sem placa

```sh
python tools/gen_art.py
python tools/test_controls.py
```

O segundo comando exige `g++` ou `clang++` no PATH (ou `CXX` apontando
para o compilador). Alternativa: configure `ZIG_BINARY` com o caminho
do Zig. Os testes executam o C++ real de Game, Input, Imu, PetSim,
Canvas e arte; substituem apenas relógio, GPIO, sensor, NVS e saída LED.

Cobrem debounce, clique/segurar/reset, orientação inicial, histerese,
sono por gesto e manual, navegação, intervalo entre brincadeiras,
ovo parado e poses sem cortes ou sobreposição durante a refeição.
O GitHub Actions também compila o firmware e verifica a arte gerada.

## Editar a arte

Edite **`art/*.art`** e execute `python tools/gen_art.py`. Não edite
`src/art/ArtData.*` ou `preview/art.js` manualmente. Veja o formato e os
critérios para LEDs em [art/README.md](art/README.md).

```text
art/           sprites, paletas, efeitos, fonte
tools/         gerador da arte e testes locais
preview/       galeria e maquete de controles
src/Game.*     cenas, entrada e animações
src/PetSim.*   estado e regras
src/Input.*    botão BOOT com debounce
src/Imu.*      leitura e gestos do acelerômetro
src/Canvas.*   composição em 8×8
src/Display.*  saída FastLED, orientação e limites
src/Storage.*  persistência NVS
test/          testes C++ e mocks de hardware
```

## Licença

MIT — [LICENSE](LICENSE).

## Próximos passos

- Configuração pelo celular via rede criada pela placa: nome, escolha de
  pet e hora da internet para um ciclo dia/noite durante a vida offline.
- Evolução ovo → bebê → adulto com caminhos conforme os cuidados.
- Importação de PNGs do LibreSprite/Aseprite para `art/`.
