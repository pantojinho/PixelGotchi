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
  dos LEDs. Confira no modo LED a 18/255 e também com a paleta de design.
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

## Perfil de brilho e cor

`led-profile.json` define o brilho global (**18/255**) e a curva gamma
(**1,6**). O gerador produz `src/art/LedProfile.h` e `ART.ledProfile` no
preview, incluindo a mesma tabela de 256 valores. Não edite as saídas à mão.

A curva é aplicada uma vez, na saída do Display, depois da composição/DNA
e antes do brilho do FastLED. O modo de design mostra o RGB original;
o modo LED usa a tabela e a quantização de brilho do firmware. O monitor
ainda faz uma aproximação da percepção, sem reproduzir fisicamente a placa.

Escolha diferenças de luminosidade e de matiz entre corpo, rosto e detalhes.
Na capivara, o corpo cobre fica mais escuro que o focinho âmbar, e o nariz
tem um terceiro nível. Os testes verificam a separação depois de gamma,
brilho e DNA, além de o nariz continuar aceso no sono. Isso verifica os
valores digitais; a leitura visual final depende dos LEDs e da luz ambiente.

Para um ambiente escuro, pode reduzir `brightness` para 12. Rode o gerador
e recompile/regrave para aplicar. O perfil aceita 1–30 de brilho e 1–2,2
de gamma; o padrão de 1,6 é um ajuste inicial para preservar detalhes nos
64 LEDs, sujeito à avaliação na placa.
