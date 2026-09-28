# Testes do PixelGotchi

[README](../README.md) · [Plano da próxima sprint](PROXIMA-SPRINT.md)

## Instalador web — roteiro de aceitação na placa

O instalador pode ser aberto em [install.html](../preview/install.html).
A imagem combinada para ESP32-S3 é gerada do build por
`python tools/build_web_installer.py` e publicada no Pages pelo workflow
`.github/workflows/pages.yml`. O botão usa Web Serial; precisa de Chrome ou
Edge em HTTPS (localhost serve para teste local). A imagem de instalação do
zero cobre a NVS e reinicia o pet salvo. O teste local da página e a compilação
não substituem ensaio com placa real.

| Verificação | Resultado | Observação |
|---|---|---|
| Geração da imagem e manifesto | Passou localmente | `python tools/build_web_installer.py`; imagem ESP32-S3 combinada, offset 0, flash de 4 MB, versão e SHA-256 em `firmware-info.json` |
| Carregamento e estado da página | Passou localmente | Biblioteca 10.4.0 carregou em localhost; botão só habilitou após confirmação; console sem erros. Seleção da porta e gravação não foram exercitadas |
| Integridade e bloqueios | Passou em Node/Python | Código real bloqueia pacote ausente, versão divergente e SHA-256 inválido; bytes usados na gravação são os mesmos conferidos. Cobertos também HTTPS/Web Serial indisponíveis e runtime Python do CI |
| Gravação em Matrix física | Pendente | Usar placa de teste; confirmar que o aviso de apagamento aparece e esperar conclusão |
| Inicialização após gravação | Pendente | Reiniciar, conferir pets/LEDs, BOOT, IMU e logs USB |
| Falha e recuperação | Pendente | Desconectar/cancelar antes de gravar; testar BOOT + RESET e caminho manual |
| Atualização preservando o pet | Pendente | O instalador de imagem completa não promete preservar estado; validar fluxo parcial separado |

### Revisão das animações e do instalador — 28/09/2026

- Firmware compilado: RAM **20.720 bytes**, aplicação **367.017 bytes**.
- Pacote combinado local: **432.912 bytes**, ESP32-S3, offset 0, flash de 4 MB.
  Revisão local de desenvolvimento `e4b0f66-dev`; o CI gera a revisão do commit publicado.
- Passaram `tools/test_controls.py`, `test/test_preview.cjs`,
  `test/test_installer.cjs` e os dois testes Python de `test_web_installer.py`.
- Preview: alimentação elevou saciedade de 80 a 100; página USB carregou
  o pacote, e a confirmação habilitou/desabilitou o botão. Sem erros registrados
  nos consoles. A ferramenta de captura de tela estava indisponível; não foi
  registrada nova imagem nesta revisão.
- GitHub Pages habilitado com fonte **GitHub Actions**. O workflow gera a
  imagem e publica o site; falhas de configuração agora aparecem como falhas.
- Seleção real de porta, gravação, reset pós-upload e aparência nos LEDs
  permanecem pendentes de ensaio físico.

Este documento reúne as evidências do software atual, o roteiro para testar
a placa e os critérios do futuro editor de pets. **Passar no preview ou no
build não comprova gravação USB, funcionamento do sensor ou aparência física.**

## Resultado registrado — 28/09/2026

Referência do código: revisão
[`585571f`](https://github.com/pantojinho/PixelGotchi/commit/585571f551f1e96894965938618dd8db5d534ad6).
Os resultados abaixo referem-se a essa revisão. O ajuste posterior de brilho
e cores tem uma rodada específica registrada na próxima seção.

| Verificação | Resultado | Evidência / limite |
|---|---|---|
| Arte compartilhada regenerada | Passou | Gerador produz os arquivos de C++ e do preview sem diferenças |
| Controles em C++ com hardware simulado | Passou | `tools/test_controls.py`, local no Windows com Zig; também na CI com compilador nativo |
| Compilação ESP32-S3 | Passou | PlatformIO; build local: RAM 20.536 / 327.680 bytes, flash 330.357 / 1.310.720 bytes da partição de aplicação |
| Integração contínua | Passou | [Execução do GitHub Actions](https://github.com/pantojinho/PixelGotchi/actions/runs/36375305914): geração, testes C++ e build |
| Interações da maquete web | Passou | Alimentação, menu, inclinação, status, sono, limpeza, remédio, intervalo de brincadeira e reset verificados no navegador |
| Layout do preview | Passou na revisão visual | Capturas de [capivara](images/system-capybara.jpg), [gato](images/system-cat.jpg) e [galeria](images/system-gallery.jpg); conferência também em viewport móvel |
| Instalação/gravação USB | Pendente na placa | Não houve upload físico nesta verificação |
| Sensor, orientação, cores e aquecimento | Pendentes na placa | Dependem do modelo, montagem e LEDs reais |
| Persistência após desligar e atualização | Pendente na placa | Mocks não comprovam escrita real na NVS nem migração entre versões |

A capacidade de aplicação acima não é o tamanho total da flash: a Matrix
é configurada para **4 MB**. A maquete web não simula relógio do firmware,
DNA, incubação, descuido nem persistência.

## Rodada de brilho e cores — 28/09/2026

Motivo: relato do autor de brilho excessivo e pouca separação de cores na
placa. Ajuste: teto 5/255, gamma 1,6 compartilhado pelo firmware/preview e
nova paleta da capivara. A silhueta e o estado salvo não mudam.

| Verificação | Resultado / limite |
|---|---|
| Display C++ real com saída FastLED capturada | Passou: brilho 18, corrente 400 mA, dithering desligado, preto apagado e primárias sem mistura |
| Contraste digital da capivara | Passou nos 64 tons possíveis do DNA: luminância RGB ponderada do focinho ≥ 2× corpo, corpo ≥ nariz (no brilho 5 os dois podem cair no mesmo degrau); nariz mantém ao menos um nível de vermelho no sono |
| Controles e composição | Passaram com o Display real incluído no teste nativo |
| Compilação ESP32-S3 | Passou: RAM 20.536 bytes e flash de aplicação 330.637 bytes |
| Curva compartilhada | Gerador entrega a mesma LUT no header C++ e nos dados do preview |
| Aparência, conforto e contraste físico | Pendente de conferir na placa após regravar; os testes digitais não medem luz emitida |

A aproximação de luminância usada no teste é calculada dos valores enviados
aos LEDs; não é uma medição fotométrica nem uma relação de contraste certificada.

## Reproduzir os testes de software

Na raiz do repositório, com Python e PlatformIO disponíveis no mesmo ambiente:

```sh
python tools/gen_art.py
git diff --exit-code -- src/art/ArtData.h src/art/ArtData.cpp src/art/LedProfile.h preview/art.js
python tools/test_controls.py
python -m platformio run -e esp32-s3-matrix
```

Se seguiu o ambiente isolado do README, substitua `python` pelo caminho
do Python desse ambiente. O teste nativo exige `g++` ou `clang++` no PATH;
também aceita `CXX` ou `ZIG_BINARY` com o caminho do compilador/Zig.
O compilador baixado pelo PlatformIO para o ESP32 não substitui o compilador
nativo usado nesse teste. A CI prepara o ambiente Linux automaticamente.

### O que o teste C++ cobre

O código real de `Game`, `Input`, `Imu`, `PetSim`, `Canvas`, `Display` e arte é executado
com relógio, GPIO, sensor, NVS e saída LED substituídos por mocks.

| Caso | Comportamento esperado |
|---|---|
| Debounce e clique | Ruído do botão não produz entradas extras |
| Segurar e soltar | Longo apenas na soltura; reset de 8 s não confirma uma ação intermediária |
| Primeiro dado do IMU | Inicialização não inventa um gesto de movimento |
| Face para baixo/cima | Histerese e tempo de espera evitam alternância por ruído |
| Sono manual e por gesto | Desvirar acorda o sono por gesto; sono manual não é cancelado pelo gesto |
| Navegação por inclinação | Retorno ao centro libera a próxima troca |
| Brincadeiras | Intervalo impede gasto repetido de energia por sacudidas consecutivas |
| Ovo parado | Incubação pausa depois da tolerância sem zerar progresso |
| Composição visual | Poses verificadas não sofrem cortes; comida, coração e acontecimentos nunca cobrem pixels do pet |
| Refeição (6 espécies) | Silhueta intacta em toda a refeição; comida aparece, só diminui e some; volta ao descanso com quadro aceso |
| Sonhos e Conway | Regras, capítulos, três minutos de sono sem apagão, bolha de entrada, máscara do Conway acordado, prioridade de BOOT/movimento |

Esses testes são casos selecionados, não uma prova de todas as combinações
de regras, tempo, DNA e falhas de hardware.

## Sonhos e Conway

Implementação do [plano de animações e Conway](ANIMACOES-E-CONWAY.md)
(etapas 1 a 6). Comportamento no firmware:

- **Refeição:** o pet olha para o lado do focinho; a comida (1–2 px) aparece
  no primeiro pixel livre à frente da boca; ele se aproxima quando cabe,
  mastiga enquanto ela diminui e termina satisfeito com um coração.
- **Acordado:** pausas (duração pelo DNA) alternam com farejar, olhar em
  volta, observar algo passando, seguir uma borboleta, pulinhos e bocejo
  antes de cochilar. BOOT ou movimento interrompem na hora.
- **Conway ao redor:** após 2 min sem BOOT/gesto/movimento, com energia
  suficiente e sem necessidade urgente, 7 s a cada 45 s: glider (visitas
  pares) ou blinker + passarinho (ímpares), só em pixels livres com 1 px de
  respiro do pet. O pet para e olha na direção do que se move.
- **Sono:** pet visível por 8 s; então bolhinhas, bolha crescendo (2,4 s) e o
  mundo inteiro. Capítulos de 20–40 s alternam gliders, pulsação e campo
  turbulento, trocando por substituição de pixels. Vazio passa na hora,
  imóvel após 3 s; oscilador que sobrou pulsa no máximo 10 s. Movimento
  revela o pet dormindo por 8 s; BOOT acorda. Menu, gesto e cochilo
  automático usam a mesma sequência.

### Resultado registrado — 28/09/2026 (software)

| Verificação | Resultado / limite |
|---|---|
| Regras de Conway | Teste nativo: blinker alterna, bloco permanece, glider cruza a borda |
| Tipos de capítulo | Teste nativo em 585 sementes: gliders atravessam 32 gerações sem colidir, pulsação tem período 2, campo turbulento é reproduzível e não nasce vazio |
| Três minutos de sono | Teste do Game real: nenhum quadro apagado (inclusive na bolha), ao menos 4 trocas de capítulo, nenhum vazio > 0,6 s nem imobilidade > 3,6 s |
| Refeição | Teste nativo nas 6 espécies: pixels do pet idênticos ao pet sozinho durante 3 s, comida visível de 0,4 a 2,1 s e só diminuindo; capivara com comida em (7,4) |
| Conway acordado | Teste nativo (capivara e gato): células só em pixels livres sem vizinho do pet; população do autômato preservada; glider e blinker aparecem |
| Prioridade | BOOT durante cochilo ocioso inicia a refeição; movimento interrompe um acontecimento; todos os comportamentos desenham o pet |
| Temporização e controles | Visitas ociosas, sono contínuo, movimento revelando o pet sem acordá-lo, cochilo automático no mesmo sonho |
| Build ESP32-S3 | Passou: RAM 20.776 bytes; flash de aplicação 370.085 bytes |
| Regressão web | `node test/test_preview.cjs` executa os renderizadores reais nas 6 espécies: idle sem apagão, refeição com a nova sequência sem cobrir o pet, cuidados, 3 min de cochilo, movimento sem acordar e BOOT |
| Preview no Chromium | Playwright com relógio simulado, modo de cores de design, capivara e gato: início, refeição, 70 s de descanso, 45 s de visitas, 190 s de sono pelo gesto, movimento, retorno do sonho e BOOT. Nenhum quadro vazio, nenhum erro de console; capítulos mudaram entre os três tipos |
| Preview | Só a espera ociosa é encurtada (12 s, ciclos de 20 s); bolha, capítulos e passos usam os tempos do firmware |
| Matriz física e IMU | Pendente: conforto visual, leitura das cores do sonho no brilho 5, glider perceptível, retorno após mover a placa |

### Roteiro na placa

Sequência principal: alimentar → mastigar → satisfação → descanso → sonho →
movimento → pet dormindo. Anote se o observador entende o que o pet faz.

1. Grave o firmware. Alimente pelo BOOT e confirme, para capivara e gato,
   que o pet não some, que a comida é reconhecível à frente da boca e que ela
   diminui antes do coração.
2. Observe um minuto de descanso: reconheça pausas e ações diferentes. Clique
   BOOT ou mova a placa durante um acontecimento e confirme a resposta imediata.
3. Mantenha o pet acordado com energia suficiente e aguarde 2 min parado;
   confirme um glider ao redor do pet e, no ciclo seguinte, blinker e
   passarinho. A espécie deve continuar reconhecível nas cores reais.
4. Durma pelo menu. Após 8 s: bolhinhas, bolha e mundo inteiro, sem apagão.
   Observe 3 min: capítulos diferentes, sem longos períodos vazios ou parados.
   Mova a placa: o pet aparece dormindo; parado 8 s, o sonho volta. BOOT acorda.
5. Repita com o cochilo automático (energia < 25, 30 s sem entrada) e com o
   gesto de virar; desvirar deve acordar o sono do gesto. A recuperação até
   energia cheia continua acordando o pet.
6. Observe o brilho a uma distância confortável; o sonho usa o mesmo perfil
   reduzido de LEDs. Ajuste somente se a luz real pedir.

## Roteiro manual do preview

Execute `python tools/preview.py` e abra a maquete interativa. Registre a
revisão, navegador, tamanho da janela e resultado de cada caso.

| ID | Procedimento | Resultado esperado |
|---|---|---|
| WEB-01 | Selecione gato e capivara; alterne design/LED | Gato com orelhas e cauda reconhecíveis; capivara com focinho largo e pernas curtas; paleta permanece distinguível |
| WEB-02 | Escolha fome e clique no BOOT | Saciedade aumenta; o pet olha, a comida aparece à frente da boca, diminui enquanto ele mastiga e termina com coração, sem o pet sumir |
| WEB-03 | Segure BOOT por 0,6 s e solte | Indicador de confirmação aparece; menu abre no cuidado sugerido |
| WEB-04 | No menu, clique ou incline; confirme | Troca uma posição por entrada; executar não exige outro botão |
| WEB-05 | Escolha sujeira e doença, execute cuidados | Limpeza remove sujeira; remédio trata a condição simulada |
| WEB-06 | Chacoalhe duas vezes e repita após 5 s | A segunda entrada imediata é ignorada; a posterior permite brincar se houver energia |
| WEB-07 | Vire por 1,5 s e desvire; repita com sono pelo menu | Sono por gesto acorda ao desvirar; sono manual continua |
| WEB-08 | Abra status e espere/clique | Uma página por atributo (ícone + barra), idade e retorno ao pet |
| WEB-09 | Segure BOOT até 8 s | Barra de reset aparece após 3 s; reset ocorre sem executar outro cuidado |
| WEB-10 | Use Espaço, setas e controles de toque; abra galeria | Interações acessíveis, sem erro no console; poses legíveis em janela estreita |

Avaliação visual: conferir as poses na **grade 8×8**, não apenas ampliadas.
Manter contraste entre olhos, corpo e fundo e reconhecer a espécie também
em descanso, sono e comida. O modo LED é aproximação, não medição de luz.

## Roteiro da placa — execução pendente

Material: Waveshare ESP32-S3-Matrix, cabo USB-C com dados, computador e a
revisão anotada. Siga a [instalação manual](INSTALAR.md#instalação-manual-via-usb).
Mantenha os limites de brilho/corrente e a versão FastLED do projeto.

| ID | Procedimento | Critério de aprovação | Estado |
|---|---|---|---|
| HW-01 | Identificar placa/porta, compilar e gravar | Upload termina com sucesso na porta escolhida; RESET com BOOT solto inicia o jogo | Pendente |
| HW-02 | Abrir monitor 115200 e reiniciar | Logs de início, QMI8658 respondendo e fase do jogo; sem reinício contínuo | Pendente |
| HW-03 | Selecionar cada espécie e confirmar | Clique/inclinação selecionam; longo na soltura confirma uma vez | Pendente |
| HW-04 | Mover suavemente o ovo, parar e continuar | Incuba por movimento, pausa após 15 s parado e retoma do progresso anterior | Pendente |
| HW-05 | Conferir gato/capivara acordados, comendo e dormindo | Espécies reconhecíveis, sem corte e sem efeito escondendo o rosto durante toda a ação | Pendente |
| HW-06 | Alimentar, usar os oito itens do menu e status | Feedback legível; tudo acessível somente com BOOT | Pendente |
| HW-07 | Inclinar para ambos os lados, retornar ao centro | Direção correta; uma troca por gesto, sem repetição quando parado inclinado | Pendente |
| HW-08 | Chacoalhar, repetir e esperar 5 s | Brincadeira respeita intervalo e não interrompe cuidado/sono | Pendente |
| HW-09 | Virar por 1,5 s, desvirar e testar sono manual | Sono por gesto e manual se distinguem conforme a tabela de controles | Pendente |
| HW-10 | Após um cuidado concluído, reiniciar; depois desligar/religar | Estado já salvo é recuperado; relógio não conta tempo desligado. Repetir após um período de pelo menos 5 min para incluir salvamento periódico | Pendente |
| HW-11 | Atualizar entre revisões com mesmo formato/partições, sem apagar flash | Pet e configurações compatíveis são preservados; anotar revisões anterior e posterior | Pendente |
| HW-12 | Usar por 30 min em USB e observar cores/estabilidade/temperatura | Sem clarão branco inesperado, travamento ou aquecimento anormal; registrar temperatura se houver instrumento | Pendente |
| HW-13 | No fim do ensaio, segurar BOOT por 8 s | Barra a partir de 3 s, volta à seleção; reinício mantém o estado reiniciado | Pendente |

Faça HW-13 **depois** dos testes de persistência: ele apaga o progresso do jogo.
Em HW-10, o corte de energia deve ocorrer depois de um salvamento; desligar
antes dele pode perder as alterações ainda não gravadas.
Temperatura aceitável e consumo real precisam de medição e avaliação na
placa; o build não certifica esses valores.

### Registro de uma rodada

```text
Data / responsável:
Revisão do Git e versão anterior, se houver:
Modelo da placa / computador / sistema operacional:
Cabo / porta / fonte USB:
Casos executados e resultado (passou / falhou / não executado):
Logs e fotos/vídeos da matriz:
Falha: passos, resultado esperado, resultado observado:
Temperatura/consumo medidos, se disponíveis:
Correções de orientação ou sensibilidade utilizadas:
```

## Editor e envio de bichinhos pela USB

Resultado registrado em 28/09/2026, em software. Veja o [guia do editor](EDITOR.md).

| Verificação | Resultado / limite |
|---|---|
| Formato PGP1 nos dois lados | `test_petpack.cjs` gera o pacote dos seis modelos; o teste C++ carrega as fixtures de capivara e gato no firmware e confere frame a frame que desenham igual aos de fábrica |
| Protocolo USB | Teste C++: `PG?`, envio em blocos de 64 bytes, `PGEND`, adoção, troca ao vivo com o pet ativo e remoção; erros de CRC, tamanho, excesso, comando, nome e falha de gravação mantêm o pacote anterior |
| Validação | Motivo específico para assinatura, comida, cores, índice de cor, referência de frame e bytes sobrando |
| Persistência | Reinício simulado recarrega o pacote da NVS (mock); gravação real na flash pendente na placa |
| Exportação `.art` | O gerador real aceita o texto exportado numa cópia temporária do repositório |
| Editor no Chromium | Playwright com uma placa falsa em `navigator.serial` (mesmo protocolo): desenhar, conferência, conectar, enviar com adoção; bytes recebidos idênticos ao pacote; simulador abre com o bichinho; PT e EN; sem erros de console; celular sem rolagem lateral |
| Build ESP32-S3 | Passou: RAM 27.744 bytes; flash de aplicação 373.297 bytes |
| Placa física | Pendente: abrir a porta pelo Chrome/Edge (a placa pode reiniciar), enviar, adotar, reenviar com o pet ativo, puxar o cabo no meio do envio e desligar logo após gravar |

## Próxima sprint: testes planejados

**Os casos abaixo foram escritos antes do editor.** FUT-01 a FUT-04 e FUT-07 a
FUT-09 já têm cobertura em software (tabela acima); todos continuam pendentes
na placa física.
O instalador web do firmware existe; seus testes estão no início deste documento.
O [plano da próxima sprint](PROXIMA-SPRINT.md) separa um primeiro editor que
exporta `.art` da etapa posterior de enviar pacotes diretamente ao firmware.

| ID | Área | Critério para aceitar a entrega futura |
|---|---|---|
| FUT-01 | Editor | Desenhar/apagar em exatamente 8×8, escolher cores, desfazer/refazer e salvar/reabrir sem alteração |
| FUT-02 | Animação | Criar poses, durações e pré-visualizar loops; orientação consistente com a Matrix |
| FUT-03 | Exportação inicial | `.art` gerado é aceito pelo gerador e pelo build; saída C++ e preview coincidem |
| FUT-04 | Validação | Rejeitar dimensões, índices de paleta, durações, quantidade de frames e tamanho total fora dos limites definidos |
| FUT-05 | Firmware web | Identificar chip compatível, usar artefato de release/partições corretas e instalar com sucesso na placa de teste |
| FUT-06 | Conexão | Usuário escolhe a porta; cancelamento, porta ocupada, desconexão e navegador sem suporte recebem orientação útil |
| FUT-07 | Envio de pet | Firmware compatível recebe, valida e confirma o pacote; desenho e cores aparecem corretamente nos LEDs |
| FUT-08 | Integridade | Pacote incompleto, checksum errado ou versão incompatível não substituem o pet anterior |
| FUT-09 | Persistência | Reiniciar/desligar mantém o pacote confirmado e o estado do pet; pacote interrompido não se torna ativo |
| FUT-10 | Atualização | Atualização compatível preserva arte, configurações e estado; incompatibilidade é informada antes da gravação |
| FUT-11 | Recuperação | Falha no flash pode ser recuperada por BOOT/RESET e pelo caminho local documentado |
| FUT-12 | Limites elétricos | Paleta branca ou muitos pixels acesos continuam respeitando os limites de brilho/corrente no firmware |
| FUT-13 | Facilidade | Pessoa sem experiência instala/cria/envia seguindo o guia; registrar onde precisou de ajuda |

Não liberar o envio direto de pets só porque a instalação web funcionou:
são protocolos e critérios diferentes. FUT-07 a FUT-10 exigem testes reais
de transferência, gravação de dados e interrupção de energia.
