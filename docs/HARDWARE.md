# Hardware, brilho e sensor

[README](../README.md) · [Montar o seu](GUIA-MONTAGEM.md) · [Como jogar](JOGO.md) · [Editor](EDITOR.md) · [Hardware](HARDWARE.md) · [Desenvolvimento](DESENVOLVIMENTO.md) · [Prompts para IA](PROMPTS-IA.md) · [English](en/HARDWARE.md)

[Documentação da placa](https://docs.waveshare.com/ESP32-S3-Matrix).

![Frente e verso da Waveshare ESP32-S3-Matrix, com matriz, USB-C, BOOT e RESET](images/hardware-components.png)

O alvo é a **Waveshare ESP32-S3-Matrix**, de **25 × 25 mm**, com 64 LEDs RGB,
USB-C, BOOT, RESET e QMI8658 integrados. Para jogar e instalar pelo USB,
**não é necessário soldar fios nem adicionar outro botão**. Se comprou no
AliExpress, confira o nome e as duas faces da placa acima: ter apenas um
ESP32 e uma matriz 8×8 não garante a mesma pinagem.

<details>
<summary>Dimensões e pinagem externa</summary>

![Dimensões da placa ESP32-S3-Matrix em milímetros](images/hardware-dimensions.png)

![Pinagem dos conectores externos da ESP32-S3-Matrix](images/hardware-pinout.png)

Os GPIOs da tabela abaixo são conexões **internas** usadas pelo firmware;
não representam fios que você precisa ligar nos conectores da imagem.
As três imagens do hardware foram fornecidas pelo autor do projeto.
Veja também a [referência oficial da Waveshare](https://docs.waveshare.com/ESP32-S3-Matrix).

</details>

| Componente | Pinos |
|---|---|
| 64 LEDs WS2812B, ordem de cor **RGB** (não o GRB comum) | GPIO14 |
| QMI8658, I²C | SDA GPIO11, SCL GPIO12 |
| BOOT | GPIO0 |

O firmware limita o brilho a **5/255** (antes 30, 18 e 13: com mais brilho as
cores vizinhas "sangram" e ficam difíceis de separar), a corrente da matriz
a **400 mA** e usa FastLED **3.6.0**, com driver RMT e sem dithering temporal.
A Waveshare informa que brilho excessivo aquece e pode danificar a placa.

As cores passam por uma **curva gamma suave de 1,6** antes do brilho global.
Ela reduz os meios-tons e ajuda a separar os tons claros e escuros. A capivara
usa corpo cobre, focinho âmbar claro e nariz marrom escuro, com maior distância
entre as cores. Preto continua apagado e as cores primárias continuam puras.

Depois da curva, o **brilho é ajustado por cor**: a soma R+G+B de cada pixel
é limitada (`glare_cap`), então branco e tons claros ofuscam menos que cores
puras; o azul é atenuado (`blue_gain`); e cores escuras nunca ficam abaixo de
`min_peak` degraus, pra não sumirem. Com o pet dormindo, cada LED aceso fica no
menor degrau visível.

O perfil fica em [`art/led-profile.json`](../art/led-profile.json) e gera a mesma
tabela para firmware e preview. Para reduzir mais, experimente `"brightness": 4`,
gere a arte e regrave a placa; não aumente o brilho para compensar contraste.
Esse ajuste não apaga o pet salvo. O resultado óptico ainda precisa de
conferência na sua unidade; a curva não é uma calibração medida do hardware.

O BOOT pressionado durante reset/energização entra no modo de gravação
do ESP32. Para interagir, pressione-o **depois** que o firmware iniciar.

Orientação da imagem: `DISPLAY_ROTATION` e `DISPLAY_MIRROR_X` em `src/Config.h`.

**Acelerômetro:** o QMI8658 fica no verso da placa, então "tela pra cima" é o
eixo z **negativo**. Os eixos e sinais ficam em `src/ImuCalib.h`, medidos na
placa real. Se a inclinação ou o "virar pra dormir" se comportarem errado
(outra placa, outro lote), recalibre em 4 posições e grave de novo:

```bash
python tools/calibrate_imu.py
```

Sensibilidade e tempo dos gestos ficam em `src/Config.h`. O IMU tenta os
endereços 0x6B e 0x6A. A primeira amostra não é contada como movimento e o
gesto de virar usa histerese para evitar oscilar entre dormir/acordar.

## Case 3D

Case de **28,7 × 28,7 × 8,6 mm** com acesso ao USB-C, aos botões BOOT e RESET
de trás (por pinos que nunca ficam apertando sozinhos) e argolinha de
chaveiro; versão com janela aberta ou com difusor e grade 8×8. Sem suporte.
Há também uma **versão com bateria** (28,7 × 47,7 × 18,8 mm, LiPo 160 mAh,
carregador TP4056 e chave liga/desliga, carregando pelo mesmo USB-C).
Veja [hardware/case](../hardware/case/README.md) (STLs, configuração para a
Ender-3 V3 KE, ligação elétrica e montagem).

![Case explodida](../hardware/case/images/exploded.png)
