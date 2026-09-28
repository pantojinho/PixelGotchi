# Arte para 64 LEDs

`pets.art`, `props.art` e `font.art` são a fonte da arte do firmware e
do preview. `python tools/gen_art.py` valida referências, dimensões,
caracteres e paletas, depois gera C++ e JavaScript.

## Formato

```text
palette exemplo
  B #8DAAE2
  P #FF7FA6
end

sprite exemplo_idle exemplo
B...B
BPBPB
B.B.B
BBPBB
end

anim exemplo_idle 450 exemplo_idle
```

`.` é transparente: LED apagado. As linhas de cada sprite têm a mesma
largura; largura e altura máximas são 8. Animações listam o tempo de
cada frame em milissegundos e os nomes dos sprites em sequência.
`sprite@paleta` reutiliza um desenho com outra paleta. `palette variante
: base` herda os índices para troca de paleta em tempo de execução.

Um pet precisa das animações `idle`, `blink`, `walk`, `eat`, `sleep`,
`happy`, `sad`, `hungry` e `egg`, prefixadas pelo seu ID. A declaração
`pet` define nome, comida, paleta selvagem e se é de perfil (`side=1`).
Mantenha a ordem de espécies existentes: o estado salvo usa esse índice.

## Critérios de desenho

- A silhueta precisa funcionar antes das cores. Preto não serve como
  contorno: desaparece no fundo. Olhos apagados usam esse contraste.
- Gato: orelhas pontudas separadas, focinho claro, peito, patas e cauda
  com pelo menos um pixel de separação do corpo.
- Capivara: perfil, orelha curta arredondada, nariz na extremidade de um
  focinho largo, costas arredondadas e pernas curtas. Sem cauda longa.
- Prefira massas de cor grandes. Tons próximos se misturam com o halo
  dos LEDs. Confira no modo LED a 30/255 e também com a paleta de design.
- Preserve orelhas e focinho nas expressões. Tristeza não deve transformar
  a espécie em outra silhueta.
- As poses do gato e da capivara usam largura 7: só sobra um pixel para
  caminhar, mas as partes essenciais ficam legíveis. Ao espelhar a
  capivara, o perfil inteiro muda de direção.
- Todas as poses são ancoradas pelo centro inferior. Sono usa altura 4,
  abrindo espaço para Zzz. Verifique os dois extremos do movimento.
- Efeitos grandes e comida alternam no tempo com o pet. Nunca apague
  colunas da composição para simular mordidas: isso apagaria o pet junto.
- As oito opções de menu reservam a última linha aos pontos de navegação.

O preview de LED é uma aproximação, sem medir difusão, gama ou calibração
de cada unidade. A validação final acontece na matriz física.
