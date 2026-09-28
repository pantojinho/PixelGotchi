# Testes do PixelGotchi

[README](../README.md) · [Plano da próxima sprint](PROXIMA-SPRINT.md)

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
| Composição visual | Poses verificadas não sofrem cortes; efeitos da refeição alternam com o pet |

Esses testes são casos selecionados, não uma prova de todas as combinações
de regras, tempo, DNA e falhas de hardware.

## Roteiro manual do preview

Execute `python tools/preview.py` e abra a maquete interativa. Registre a
revisão, navegador, tamanho da janela e resultado de cada caso.

| ID | Procedimento | Resultado esperado |
|---|---|---|
| WEB-01 | Selecione gato e capivara; alterne design/LED | Gato com orelhas e cauda reconhecíveis; capivara com focinho largo e pernas curtas; paleta permanece distinguível |
| WEB-02 | Escolha fome e clique no BOOT | Saciedade aumenta; comida, mastigação e coração aparecem em sequência |
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
revisão anotada. Siga a [instalação manual](../README.md#instalação-manual-via-usb).
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

## Próxima sprint: testes planejados

**Não executados: editor, pacotes de pets e instalador web ainda não existem.**
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
