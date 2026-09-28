# Plano de melhorias das animações e dos sonhos

[README](../README.md) · [Testes](TESTES.md) · [Editor de pets e USB](PROXIMA-SPRINT.md)

**Status: implementado e revisado no software; validação na matriz física pendente.**
Diagnóstico original baseado na revisão `ad80145`; a implementação começou
em `e4b0f66`. O roteiro abaixo preserva as ideias e critérios de aceite.

## O que foi implementado

| Etapa | Resultado atual |
|---|---|
| 1 — quadro vazio | O desenho normal acontece antes e entre as janelas procedurais; regressão verifica todos os seis pets e o fim da refeição |
| 2 — cuidados | Pet presente do começo ao fim de comida, brincadeira, carinho e remédio; migalhas e coração pequeno usam pixels livres. Limpeza e sujeira também preservam a pose |
| 3 — intenções | Farejada ganha um detalhe visual; olhar alterna direção; a maquete demonstra pausas, farejada da capivara e atenção do gato. Pesos de personalidade existentes continuam no firmware |
| 4 — ambiente | Conway roda na grade completa, mas é desenhado só fora da silhueta enquanto acordado; a máscara não modifica a simulação |
| 5 — capítulos | Transição de 1,2 s; capítulos de até 30 s, com renovação antecipada quando vazios ou estáticos por 20 s; sementes e paletas variam de forma reproduzível |
| 6 — cochilo | Todos os sonos usam os capítulos; movimento mostra o pet dormindo por 8 s, depois retoma; BOOT acorda |

Testes usam os renderizadores reais: C++ com hardware substituído e JavaScript
com DOM/relógio simulados. Três minutos de sonho foram percorridos sem quadro
vazio. A comida procedural continua como experimento futuro. Migalhas indicam
consumo, mas não há perseguição física do alimento nem ganho por célula.

## Problema observado e diagnóstico

O pet desaparece durante a alimentação e pode ficar ausente quando a ação
termina. Foram identificadas duas causas no código:

- Em `src/Game.cpp`, `drawAction()` mostra somente a comida nos primeiros
  600 ms da refeição e somente o coração nos últimos 500 ms. A cena
  `comendo` de `preview/index.html` segue a mesma composição.
- Em `preview/controls.js`, a condição de ociosidade captura o pet saudável
  e sem ação mesmo antes do prazo do sonho ou fora da janela de exibição.
  Esses caminhos não desenham o pet e não alcançam o desenho normal nos
  `else if` seguintes, devolvendo um quadro vazio. Isso também acontece
  depois de terminar uma refeição.

O primeiro passo é corrigir o quadro vazio. Depois, tornar os cuidados
ações visíveis do pet e dar mais variedade aos acontecimentos e sonhos.

## Etapa 1 — corrigir o quadro vazio no preview

Entrar no caminho de renderização do sonho apenas quando sua janela estiver
ativa. Nos outros momentos da vida, desenhar o pet e seus efeitos normais.
Um pet saudável deve continuar visível antes do primeiro sonho e entre duas
visitas ao mundo procedural.

**Aceite:** observar o início da maquete, terminar uma refeição e esperar
um ciclo completo de sonho ocioso. Todos os intervalos devem ter uma cena
visível. Um quadro vazio não deve aparecer como resultado da troca de estado.

## Etapa 2 — alimentação com continuidade

Manter a silhueta do pet durante toda a refeição:

1. O pet olha para o lado do focinho.
2. Um pedaço de comida de um ou dois pixels aparece numa posição livre.
3. O focinho se aproxima e a comida diminui enquanto o pet mastiga.
4. O pet fica satisfeito e um pequeno coração aparece acima dele.

Posicionar os efeitos conforme o espaço livre de cada espécie. Na capivara,
preservar o focinho comprido e usar uma pequena mudança da mandíbula para
sugerir mastigação. O efeito deve manter os olhos e o nariz reconhecíveis e
respeitar os limites de 8×8. Usar os frames existentes quando forem adequados.

Aplicar a mesma sequência em `src/Game.cpp` e `preview/index.html`.
Rever também carinho, brincadeira e remédio, que atualmente têm momentos
com um ícone sozinho substituindo o pet.

**Aceite:** durante toda a alimentação, reconhecer a espécie e acompanhar
a comida sendo consumida. A conclusão deve retornar ao descanso sem apagão.
Cada refeição continua com um ganho fixo de saciedade; os efeitos visuais
não definem a quantidade de alimento recebida.

## Etapa 3 — pequenas intenções quando acordado

Alternar pausas com acontecimentos curtos: farejar um ponto, observar algo
passando, acompanhar uma borboleta ou se acomodar antes de cochilar.
Variar o intervalo dessas ações e usar o DNA para escolher suas frequências.

- Capivara: curiosidade calma, movimentos lentos e farejadas.
- Gato: olhar atento e acompanhamento dos objetos com a cabeça.

BOOT ou movimento devem receber resposta imediata. Um cuidado solicitado
tem prioridade sobre um acontecimento ocioso. A maquete deve demonstrar
essa ordem de prioridade.

**Aceite:** observar um minuto de descanso e reconhecer pausas e ações
diferentes; alimentar ou abrir o menu deve responder durante um acontecimento.

## Etapa 4 — Conway como ambiente ao redor do pet

Simular a grade completa com as regras de Conway e exibir suas células
somente nos pixels livres ao redor do pet. A máscara é uma decisão de
renderização; não apagar as células escondidas do estado do autômato.
Desenhar o pet por cima para proteger olhos, focinho e silhueta.

Usar padrões com intenções visuais claras: o glider fornece deslocamento e
o blinker uma pequena pulsação. Um passarinho ou uma borboleta pode ser um
efeito desenhado que acompanha o acontecimento; o pet olha para sua direção.
Manter poucos elementos ao mesmo tempo para a leitura funcionar em 64 pixels.

Referências: [glider](https://playgameoflife.com/lexicon/glider) e
[blinker](https://playgameoflife.com/lexicon/blinker).

**Aceite:** o acontecimento é perceptível e a espécie permanece reconhecível,
inclusive nas cores e no brilho usados pela matriz física.

## Etapa 5 — sonho longo com capítulos

Entrar no sonho por uma sequência legível: o pet fecha os olhos, uma bolha
surge acima da cabeça e se espalha até virar o mundo inteiro. Fazer a
transição por substituição de pixels, preservando conteúdo visível em cada
passo e evitando uma passagem pela tela inteira apagada.

Durante o sono, variar os capítulos aproximadamente a cada 20–40 s:
gliders atravessando, pequenas pulsações e grupos nascendo e se desfazendo.
Detectar um padrão vazio ou parado e preparar o capítulo seguinte. Um
oscilador pode continuar por algum tempo antes da próxima mudança.

Na implementação anterior, um padrão estabilizado podia permanecer imóvel
durante o restante do sono. O contador de gerações iguais agora antecipa a
troca de capítulo após 20 s; todos os padrões também renovam em até 30 s.

Variar posição, orientação e semente com o DNA e um contador de capítulos
para ter variedade reproduzível. Cada capítulo usa uma paleta pequena,
cores bem separadas e o perfil reduzido de brilho já existente. As gerações
de Conway continuam usando suas regras; a troca de semente acontece na
passagem entre capítulos.

**Aceite:** observar pelo menos três minutos de sono e identificar mudança
de capítulos, evolução dos padrões e ausência de longos períodos vazios ou
involuntariamente imóveis.

## Etapa 6 — controles e cochilo automático

O sono pelo menu, pelo gesto e o cochilo automático por energia baixa devem
usar a mesma sequência de sonhos. Durante o sonho, movimento revela o pet
ainda dormindo; alguns segundos parado retomam o sonho. BOOT acorda o pet.
Preservar também o gesto existente de desvirar para acordar o sono iniciado
por virar a placa e a recuperação até energia cheia.

Acordado, os acontecimentos procedurais aparecem ocasionalmente e cedem
espaço aos cuidados. Menus e status continuam acessíveis pelo único botão.

**Aceite:** testar o sono manual e o cochilo automático, mover a placa,
confirmar que o pet continua dormindo, esperar o sonho retornar e usar BOOT
para acordá-lo. Conferir separadamente o sono iniciado pelo gesto.

## Ideia posterior — comida procedural

A comida pode deixar um pequeno rastro de células que o pet consome
visualmente. O ganho fixo por refeição mantém o equilíbrio do jogo mesmo
quando a evolução das células é imprevisível. Experimentar esse efeito
depois de validar a alimentação contínua e os capítulos dos sonhos.

## Orientação para próximas revisões

Implementar uma etapa por vez, começando pelo defeito do preview. Em cada
etapa, conferir a sequência visual completa no navegador e a composição
equivalente no firmware. Registrar os resultados em `docs/TESTES.md`,
distinguindo testes de software de observações na placa física.

A sequência principal de aceitação é: alimentar → mastigar → satisfação →
descanso → sonho → movimento → pet dormindo. O observador deve entender o
que o pet está fazendo, sem apagões inesperados nas transições.
