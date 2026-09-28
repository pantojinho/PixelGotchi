# Prompts para usar com IA

[README](../README.md) · [Montar o seu](GUIA-MONTAGEM.md) · [Como jogar](JOGO.md) · [Hardware](HARDWARE.md) · [Desenvolvimento](DESENVOLVIMENTO.md) · [Prompts para IA](PROMPTS-IA.md)

Copie o bloco inteiro e cole na IA. Há dois tipos de assistente:

- **Chat comum** (no navegador ou no celular): conversa e gera texto, mas não
  mexe no seu computador nem na placa. Serve para os prompts 2 e 4.
- **Agente local** (um agente de programação rodando no seu computador, com
  terminal): consegue baixar o projeto, rodar comandos e gravar a placa pelo
  USB. Necessário para os prompts 1, 3 e 5.

Para só gravar o jogo, você não precisa de IA: o
[instalador pelo navegador](https://pantojinho.github.io/PixelGotchi/install.html)
faz isso em poucos cliques.

## 1. Instalar na placa com um agente local

```text
Instale o PixelGotchi da https://github.com/pantojinho/PixelGotchi
na minha Waveshare ESP32-S3-Matrix conectada por USB. Autorizo baixar
as ferramentas, compilar e gravar o firmware nessa placa.

1. Leia o README, docs/INSTALAR.md e o platformio.ini da revisão atual.
   Confira o modelo ESP32-S3-Matrix, com matriz RGB 8x8, BOOT e QMI8658.
   Preserve arquivos locais existentes; clone em uma pasta própria se necessário.
2. Identifique meu sistema operacional, Git, Python e PlatformIO.
   Use um ambiente Python fora do repositório e PlatformIO 6.2.0,
   como no guia. Instale apenas as dependências necessárias.
3. Liste as portas USB/seriais e identifique a minha placa. Se houver
   ambiguidade, peça que eu desconecte/reconecte a placa para confirmar.
   Não escolha uma porta apenas por ser a primeira da lista.
4. Compile o ambiente esp32-s3-matrix. A geração de arte é automática.
   Mantenha a flash de 4 MB, FastLED 3.6.0, USB CDC e limites de brilho
   e corrente do projeto. Não altere a arte ou as regras do jogo.
5. Faça upload usando explicitamente a porta identificada. Se precisar
   de modo de download, oriente BOOT segurado + toque RESET + soltar
   BOOT, liste a porta de novo e prossiga. Após gravar, RESET com BOOT
   solto inicia o jogo. Não execute erase_flash.
6. Abra o monitor em 115200 e confira os logs de início e do QMI8658.
   Informe o resultado real, a revisão instalada e a porta usada.
   Se o USB não estiver acessível, explique o bloqueio e forneça os
   comandos exatos para eu concluir; não declare sucesso sem upload.
7. Explique a seleção do pet, incubação com movimento, clique para
   alimentar, segurar e soltar para o menu e gestos de brincar/dormir.
   Diferencie o que verificou no software do que eu devo conferir nos LEDs.
```

## 2. Desenhar um bichinho novo (chat comum)

Troque o que está entre `< >`. A IA devolve o texto no formato `.art` do
projeto; depois use o prompt 3 ou siga
[Editar a arte](DESENVOLVIMENTO.md#editar-a-arte) para testar e gravar.

```text
Quero criar um bichinho novo para o PixelGotchi, um bichinho virtual numa
matriz de LEDs RGB 8x8 (https://github.com/pantojinho/PixelGotchi).
O bichinho: <ex.: um pinguim de frente, preto e branco, bico laranja>.
ID curto em minúsculas: <ex.: pingu>. Nome: <ex.: Pinguim>.

Gere um bloco de texto no formato art/*.art do projeto, seguindo à risca:

Formato
- "palette <nome>" seguido de linhas "  <LETRA> #RRGGBB" e "end".
- "sprite <nome> <paleta>" seguido das linhas do desenho e "end".
  "." é LED apagado. Todas as linhas de um sprite têm a mesma largura.
  Largura e altura máximas: 8. Use só letras definidas na paleta.
- "anim <nome> <ms_por_frame> <sprite> <sprite> ..." em uma linha.
- "sprite@paleta" reutiliza um desenho com outra paleta.
- "palette <id>_wild : <id>" redefine as mesmas letras com cores
  mais escuras/terrosas (versão selvagem, quando o bichinho é descuidado).

O que entregar (troque <id> pelo ID)
- palette <id> com 2 a 4 cores bem saturadas e diferentes em claridade.
- palette <id>_wild : <id>.
- palette egg_<id> com as letras E (casca), S (pintas), H (brilho,
  #FFFFFF) e C (rachadura), nessa ordem, nas cores do bichinho.
- Sprites e as animações obrigatórias:
  <id>_idle, <id>_blink, <id>_walk, <id>_eat, <id>_sleep, <id>_happy,
  <id>_sad, <id>_hungry, <id>_tired e
  "anim <id>_egg 700 egg0@egg_<id>" (o desenho do ovo já existe).
- Por último, a linha:
  pet <id> "<Nome>" food=<food_melon|food_fish|food_fly|food_worm|food_carrot|food_shrimp>
  wild=<id>_wild   (acrescente side=1 se o bichinho for desenhado de perfil
  olhando para a direita)

Regras de desenho para 64 LEDs
- Fundo apagado; não use preto como contorno. A silhueta precisa ser
  reconhecível só pela forma. Olhos costumam ser um pixel apagado.
- Poses de pé com no máximo 7 de largura e 7 de altura; todas se alinham
  pelo centro da base. Sono com altura 4 (sobra espaço para o Zzz).
- Triste e cansado nunca mais largos que o idle.
- Expressões: feliz = olhos fechadinhos; triste = lágrima azul (#4FA8FF)
  sob o olho; fome = boca aberta; cansado = cabeça baixa, olhos semicerrados
  e bocejo; comer = a boca muda entre dois frames (o jogo usa essa diferença
  para achar a boca e pôr a comida à frente dela).
- Preserve orelhas, focinho e rosto em todas as poses.

Devolva só o bloco .art, pronto para colar no fim de art/pets.art,
com a linha pet por último. Depois, explique em 3 linhas como o desenho
foi pensado.
```

## 3. Criar, testar e gravar um bichinho (agente local)

```text
No repositório PixelGotchi (https://github.com/pantojinho/PixelGotchi),
adicione um bichinho novo e deixe-o pronto para eu testar e gravar.
<Cole aqui a descrição do bichinho ou o bloco .art que você já tem.>

1. Leia art/README.md, docs/DESENVOLVIMENTO.md e o fim de art/pets.art
   (ovos, paletas selvagens e linhas "pet").
2. Acrescente paleta, paleta selvagem "<id>_wild : <id>", paleta
   egg_<id>, sprites e as animações idle, blink, walk, eat, sleep, happy,
   sad, hungry, tired e egg. Adicione a linha "pet" DEPOIS das existentes:
   o estado salvo usa a posição na lista, então não reordene espécies.
3. Rode python tools/gen_art.py até passar sem erros.
4. Rode python tools/test_controls.py e node test/test_preview.cjs.
   Se um teste de composição falhar, ajuste o desenho (largura, altura,
   diferença entre idle e eat), não o teste.
5. Rode python tools/preview.py e me diga o que conferir no simulador
   (seleção, refeição, sono, triste, com fome) com o novo bichinho.
6. Só grave na placa se eu pedir, seguindo docs/INSTALAR.md com a porta
   confirmada por mim. Não altere brilho, corrente nem regras do jogo.
7. Mostre o diff final e liste o que foi verificado em software e o que
   eu ainda preciso olhar nos LEDs.
```

## 4. Tirar dúvidas de compra, impressão e montagem (chat comum)

```text
Estou montando um PixelGotchi (https://github.com/pantojinho/PixelGotchi):
Waveshare ESP32-S3-Matrix, case impressa em 3D e, opcionalmente, bateria.
Use como referência docs/GUIA-MONTAGEM.md e hardware/case/README.md
desse repositório. Minha situação: <ex.: tenho uma Ender-3, nunca
imprimi peças pequenas / comprei uma placa e quero conferir se é a certa
(descreva ou envie fotos) / quero fazer a versão com bateria>.

Me guie passo a passo. Não invente medidas: quando algo depender da minha
placa ou impressora, diga como medir. Na versão com bateria, destaque os
riscos elétricos (troca do resistor do TP4056, diodo, nunca ligar a
bateria no 3V3) antes de qualquer solda.
```

## 5. Mexer no código com um agente local

```text
Quero modificar o PixelGotchi (https://github.com/pantojinho/PixelGotchi):
<descreva a mudança>.

Antes de editar, leia README.md, docs/DESENVOLVIMENTO.md, docs/JOGO.md,
src/Config.h e os testes em test/. Regras do projeto:
- Arte só em art/*.art, gerada por tools/gen_art.py; nunca edite
  src/art/ArtData.* nem preview/art.js à mão.
- Firmware (src/) e maquete (preview/) precisam continuar equivalentes:
  mude os dois lados quando a mudança for visível.
- Não aumente brilho (art/led-profile.json), corrente (MAX_MILLIAMPS) nem
  troque a versão do FastLED.
- Antes de concluir, rode os mesmos comandos da CI (docs/DESENVOLVIMENTO.md,
  "Testes sem placa") e mostre a saída. Diga claramente o que não pôde ser
  verificado sem a placa física.
```
