# Editor de bichinhos

[README](../README.md) · [Montar o seu](GUIA-MONTAGEM.md) · [Como jogar](JOGO.md) · [Editor](EDITOR.md) · [Hardware](HARDWARE.md) · [Desenvolvimento](DESENVOLVIMENTO.md) · [Prompts para IA](PROMPTS-IA.md) · [English](en/EDITOR.md)

Crie um bichinho seu no navegador e mande para a placa pelo cabo USB, sem
instalar nada e sem compilar. Abra o
[editor no site](https://pantojinho.github.io/PixelGotchi/editor.html) ou,
na cópia local, `http://localhost:8765/editor.html`.

## Passo a passo

1. **Começar de** um dos seis bichinhos (ou em branco) e **Usar este modelo**.
   Partir de um modelo é o jeito mais rápido: todas as poses já existem e
   você só muda o que quiser.
2. **Nome** (até 12 letras, sem acento), **comida favorita** e se ele é
   desenhado **de perfil** (olhando para a direita; o jogo espelha quando anda
   para a esquerda).
3. **Desenhe** na grade 8×8: clique ou arraste para pintar, botão direito
   apaga, **Balde** preenche uma área. **Espelhar** e as setas ajudam a alinhar.
   Preto é LED apagado: a silhueta é feita de cor, não de contorno.
4. **Animações**: escolha Descanso, Piscar, Andar, Comer, Dormir, Feliz,
   Triste, Com fome ou Cansado. Cada uma é uma sequência de **passos**; cada
   passo mostra um **frame**. Um mesmo frame pode aparecer em vários passos e
   animações (o editor avisa onde ele é usado; editar muda todos esses
   lugares). **+ Frame novo** cria uma cópia para você alterar; **+ Repetir
   este** repete o mesmo frame. Ajuste o tempo de cada passo em ms.
5. **Conferência**: em vermelho, o que a placa não aceitaria; em laranja, o que
   funciona mas pode ficar estranho. Quando estiver tudo certo aparece o
   tamanho do pacote.
6. **Testar no simulador** abre o jogo com o seu bichinho selecionado, nos
   mesmos controles da placa. Ele também aparece na galeria de animações.
7. **Enviar para a placa**: Chrome ou Edge no computador, placa num cabo USB-C
   de dados. **Conectar placa**, escolha a porta, **Enviar bichinho**.

O projeto fica salvo no navegador. Use **Baixar projeto** para guardar um
arquivo `.pixelgotchi.json` e **Abrir projeto** para continuar em outro
computador. Desfazer/refazer: botões ou `Ctrl+Z` / `Ctrl+Y`.

## O que acontece na placa

- Sem marcar nada, o bichinho fica **salvo como 7ª espécie**. Ele aparece na
  seleção quando você recomeça o jogo (segure BOOT por 8 s).
- Marcando **Trocar o meu bichinho atual por um ovo deste**, o pet atual é
  substituído na hora por um ovo do novo bichinho. O pet anterior é perdido.
- Se o pet atual **já é** o do editor, o desenho novo aparece na hora: dá para
  ajustar e reenviar quantas vezes quiser sem perder o bichinho.
- Cabe **um** bichinho do editor por vez; enviar outro substitui o anterior.
- O **instalador** do firmware apaga a placa inteira, inclusive o bichinho do
  editor. Guarde o arquivo do projeto para reenviar depois.

A placa precisa do firmware atual. Se o editor disser que ela não respondeu,
grave o firmware pelo [instalador](https://pantojinho.github.io/PixelGotchi/install.html)
e conecte de novo. Um envio interrompido ou corrompido não substitui o
bichinho que já estava salvo.

## Dicas de desenho para 64 LEDs

- Deixe ao menos **uma coluna livre** (7 de largura): com 8 ele não anda.
- Todas as poses dividem as mesmas colunas; a linha pontilhada na grade mostra
  a largura que o jogo vai usar. O bichinho fica apoiado na última linha.
- **Comer** precisa ser diferente de **Descanso** na boca: o jogo compara os
  dois para achar a boca e pôr a comida bem na frente dela.
- **Dormir** baixo (até 4 ou 5 linhas) deixa espaço para os Zzz e a bolha do sonho.
- Poucas cores bem diferentes em claridade funcionam melhor que tons próximos,
  que se misturam no brilho dos LEDs. Confira na prévia com aparência de LED.
- Veja também os [critérios de arte](../art/README.md#critérios-de-desenho).

## Limites

| Item | Limite |
|---|---|
| Cores | 1 a 15 (mais o LED apagado) |
| Frames diferentes | até 40 |
| Passos por animação | 1 a 12, cada um de 40 a 5000 ms |
| Nome | até 12 caracteres ASCII |
| Pacote | até 3072 bytes |

## Para desenvolvedores

O editor monta um pacote binário **PGP1** (`preview/petpack.js`) que o
firmware valida e grava na NVS (`src/CustomPet.*`). O formato e o protocolo
estão documentados no topo de [`src/CustomPet.h`](../src/CustomPet.h).
Resumo do protocolo, em linhas de texto pela mesma USB dos logs a 115200:

```text
PG?            -> PG HELLO <protocolo> <máx bytes> <tem pet 0/1> <ativo 0/1> <nome|->
PGPUT <bytes>  -> PG READY
PGD <hex>      -> PG ACK <recebidos>        (até 64 bytes por linha)
PGEND          -> PG SAVED <nome> | PG ERR <motivo>
PGADOPT        -> PG ADOPTED
PGDEL          -> PG DELETED
```

O pacote é validado inteiro (tamanho, índices de cor, referências, tempos e
CRC-32) antes de ser gravado; a NVS grava o valor novo antes de soltar o
antigo. `test/test_petpack.cjs` gera fixtures que o teste C++ carrega no
firmware, garantindo que navegador e placa entendem os mesmos bytes.
