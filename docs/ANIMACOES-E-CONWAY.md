# Plano de melhorias das animações e dos sonhos

[README](../README.md) · [Testes](TESTES.md) · [Editor de pets e USB](PROXIMA-SPRINT.md)

**Status: etapas 1 a 6 implementadas no firmware e na maquete; aceitação
na placa física pendente.** Diagnóstico original baseado na revisão `ad80145`.
A seção [Estado da implementação](#estado-da-implementação) resume o que
foi feito em cada etapa; as evidências ficam em
[Testes](TESTES.md#sonhos-e-conway). A ideia de comida procedural continua
para depois.

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

O código atual reinicia padrões turbulentos apenas quando ficam vazios.
Um padrão que se estabiliza pode permanecer imóvel durante o restante do
sono; essa situação precisa de tratamento.

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

## Estado da implementação

| Etapa | Firmware (`src/Game.cpp`, `src/Dream.cpp`) e maquete (`preview/`) | Na placa |
|---|---|---|
| 1 — quadro vazio | O preview só entra no sonho dentro da janela ativa; fora dela desenha o pet e seus efeitos. Teste de navegador com relógio simulado não encontrou quadro vazio na sequência completa | Pendente |
| 2 — alimentação | `planMeal()`: boca detectada pela diferença entre o idle e o frame de comer; comida (1–2 px nas duas cores mais usadas do sprite) no primeiro pixel livre à frente da boca; passo de aproximação só quando ainda cabe; mastiga, a comida diminui, coração no fim. Capivara mastiga no lugar, comida em (7,4). Carinho, brincadeira e remédio já mantinham o pet | Pendente |
| 3 — intenções | Novo `Beh::Watch` (algo passa no alto; pets de perfil viram a cabeça), borboleta no `Chase`, bocejo antes do `Nap`, farejar um ponto no chão à frente do focinho, pausas com duração pelo DNA. `reactToInput()` interrompe o acontecimento em BOOT/movimento. O gato de frente não tem frame de cabeça virada: observa parado | Pendente |
| 4 — Conway ao redor | Visitas alternam glider e blinker (+ passarinho). Máscara com 1 px de respiro do pet; posição inicial escolhida entre 16 candidatas pelo espaço livre; o autômato não é apagado. O pet para e olha para o passarinho ou para o centro do padrão | Pendente (cores e brilho reais) |
| 5 — sonho com capítulos | Bolhinhas da cabeça, bolha crescendo até cobrir a matriz (2,4 s). Capítulos `Gliders`, `Pulse`, `Soup` de 20–40 s, sementes por DNA + contador, troca por substituição de pixels; vazio passa de capítulo na hora, imóvel após 3 s, oscilador que sobrou pulsa no máximo 10 s | Pendente |
| 6 — controles | Menu, gesto e cochilo automático entram pelo mesmo `updateDreamView()`. Movimento revela o pet dormindo, o sonho volta pela bolha; BOOT acorda; desvirar acorda o sono do gesto | Pendente |

## Orientação de execução para o Cláudio

Implementar uma etapa por vez, começando pelo defeito do preview. Em cada
etapa, conferir a sequência visual completa no navegador e a composição
equivalente no firmware. Registrar os resultados em `docs/TESTES.md`,
distinguindo testes de software de observações na placa física.

A sequência principal de aceitação é: alimentar → mastigar → satisfação →
descanso → sonho → movimento → pet dormindo. O observador deve entender o
que o pet está fazendo, sem apagões inesperados nas transições.
