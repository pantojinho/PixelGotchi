# Próxima sprint — criar um pet 8×8 e levar para a placa

[README](../README.md) · [Testes e critérios de aceite](TESTES.md#próxima-sprint-testes-planejados)

**Proposta de produto e arquitetura. Não implementada nesta revisão.**
O objetivo é permitir que qualquer pessoa desenhe o próprio bichinho,
escolha suas cores e animações, experimente o resultado e envie para a
Waveshare ESP32-S3-Matrix pelo USB. Também queremos guiar a instalação e
as atualizações do firmware, sem exigir que a pessoa programe.

## O que existe hoje

- Galeria e maquete de controles no navegador, com a arte compartilhada.
- Sprites e paletas em `art/*.art`, convertidos por `tools/gen_art.py` em C++.
- Compilação e upload por PlatformIO, documentados no README.
- USB CDC para logs; **não há protocolo de recebimento de pets personalizados**.
- Estado do jogo salvo na NVS. Novos formatos e migrações precisarão de projeto.

Alterar um desenho hoje exige regenerar e **recompilar o firmware**. O preview
atual não tem ferramentas de desenho e não é um simulador completo do jogo.

## Experiência pretendida

1. Abrir o editor e escolher um modelo inicial ou uma tela vazia de 8×8.
2. Pintar/apagar pixels, escolher cores e dar um nome ao pet.
3. Criar poses de descanso, piscar, comer, feliz e dormir; visualizar o loop.
4. Testar o visual e as interações de BOOT e movimento na maquete.
5. Salvar o projeto no computador para poder continuar depois.
6. Conectar a Matrix com um cabo USB-C com dados e escolher sua porta.
7. Se necessário, instalar/atualizar um firmware compatível de forma guiada.
8. Enviar o pet, aguardar a confirmação da placa e vê-lo nos LEDs.

O editor trabalha em 64 pixels reais, com fundo apagado e paleta editável.
Brilho visual do editor e limite elétrico da placa continuam sendo coisas
distintas: escolher uma cor não pode remover os limites do firmware.
Um modo de aproximação LED ajuda a escolher contraste, mas a conferência
final é feita na matriz física.

## Duas entregas para chegar lá

### Etapa A — editor e exportação, sem mudar o armazenamento da placa

Primeiro escopo recomendado: editor 8×8, paleta, frames/durações, salvar
projeto, importar/exportar e preview. Exportar o formato `.art` já aceito
pelo repositório, conforme [art/README.md](../art/README.md).

O envio nesta etapa usa o caminho existente: aplicar a arte a uma espécie
do projeto, gerar, compilar e gravar com PlatformIO. Um agente local pode
guiar o processo. O mapeamento das poses e os nomes exportados devem ser
compatíveis com o gerador e as referências de animação do firmware.

**Limite desta etapa:** ainda precisa de compilação local para cada arte;
não oferece envio direto de um projeto do navegador para a placa.

### Etapa B — firmware genérico, instalador web e pacotes via USB

Para o fluxo completo, o firmware precisa renderizar um pacote de arte
carregado em tempo de execução, sem recompilar a cada pet. O instalador web
entrega esse firmware; depois o editor envia os dados do pet por USB CDC.

Essas duas operações usam o mesmo cabo, mas têm funções diferentes:

| Operação | Destino | Como funciona |
|---|---|---|
| Instalar/atualizar o programa | Flash do ESP32-S3 | Protocolo de gravação do chip e binários de uma release |
| Enviar/atualizar um pet | Armazenamento de dados do programa | Protocolo novo no firmware, com validação, confirmação e ativação do pacote |

O bootloader de gravação não entende um arquivo de desenho por conta própria.
O projeto precisa implementar a recepção e a renderização desses dados.

## Como conectar e atualizar pelo navegador

Proposta inicial: **Chrome/Edge em computador**, com detecção de suporte a
Web Serial. A página publicada precisa usar HTTPS; a seleção da porta
parte de um clique do usuário. A aplicação apresenta andamento, confirma
o resultado e orienta BOOT/RESET quando necessário. A API não permite
prometer escolha silenciosa de qualquer dispositivo conectado.
Referência: [Web Serial no Chrome](https://developer.chrome.com/docs/capabilities/serial).

Para instalar releases, avaliar [ESP Web Tools](https://esphome.github.io/esp-web-tools/)
ou [esptool-js da Espressif](https://espressif.github.io/esptool-js/).
ESP Web Tools oferece instalação pelo navegador a partir de um manifesto
de firmware. A adoção depende de um ensaio na Matrix real; não há integração
dessas ferramentas no projeto hoje.

O fluxo proposto após escolher a porta:

- Verificar a família do chip e solicitar confirmação do modelo Matrix.
  Detectar ESP32-S3 não confirma sozinho a pinagem, o sensor ou a matriz.
- Consultar a versão e as capacidades do firmware, quando ele já for
  compatível com o protocolo do editor.
- Oferecer a release compatível, apresentar as mudanças e iniciar a gravação
  depois da ação do usuário; exibir progresso e instrução de recuperação.
- Reiniciar, reconectar se a porta mudar e verificar a versão em execução.
- Liberar o envio da arte apenas depois desse handshake.

“Atualizar automaticamente” significa automatizar essas etapas técnicas
**depois da escolha/autorização do usuário**, com retorno claro de sucesso
ou falha. Não significa gravar uma porta desconhecida em segundo plano.
Sem Web Serial, manter o caminho local por PlatformIO e o prompt para IA.
Wi-Fi não é necessário para essa proposta.

### Publicação das versões

A CI deverá gerar os artefatos de release e seu manifesto para **ESP32-S3,
4 MB**, incluindo os arquivos/offsets exigidos pela configuração real de
bootloader, partições e aplicação — ou uma imagem combinada validada.
Os endereços devem vir do build; não copiar offsets genéricos de outra placa.

Registrar versão, revisão, checksums e compatibilidade do protocolo/pacote.
Definir a política de atualização e migração antes de trocar o layout de
partições. A atualização normal deve preservar os dados compatíveis; uma
limpeza completa precisa ser uma opção explícita com aviso de perda do pet.

## Pacote de pet e protocolo propostos

O formato definitivo fica para a implementação. Campos mínimos sugeridos:

| Campo | Objetivo |
|---|---|
| Versão do formato e ID estável | Detectar incompatibilidades e não depender da posição na lista de espécies |
| Nome e autoria opcional | Identificar o pet no editor |
| Dimensões fixas 8×8 | Impedir arte fora da matriz |
| Paleta RGB e índice apagado | Escolher cores e representar LEDs desligados |
| Frames com 64 índices e durações | Descrever poses e animações com tamanho previsível |
| Mapeamento de estados | Descanso, piscar, comer, dormir, felicidade, tristeza e fome; definir alternativas para estados ausentes |
| Tamanho e checksum | Validar o conteúdo recebido antes da ativação |

Definir limites de paleta, frames, duração e tamanho total a partir do
orçamento de RAM/flash da Matrix. O pacote só contém **dados de arte**;
as regras e limites elétricos permanecem no programa.

Protocolo USB CDC sugerido, ainda sem comandos implementados:

```text
Editor → consulta de versão/capacidades
Placa  → versão, formato suportado, limites e pacote ativo
Editor → início do envio (ID, tamanho, checksum)
Editor → blocos numerados, com confirmação e tempo limite
Placa  → valida tamanho, índices, frames, versão e checksum
Editor → pedido de ativação
Placa  → confirma gravação/ativação e passa a mostrar o novo pet
```

Separar mensagens desse protocolo dos logs de diagnóstico para que logs
não corrompam transferências. Prever cancelamento, porta ocupada,
reconexão e desconexão no meio do envio.

Armazenamento sugerido: área para pacotes (por exemplo, LittleFS em uma
partição dimensionada no build) e metadados/estado na NVS. Gravar primeiro
em uma área temporária, validar, depois ativar de forma recuperável. Após
queda de energia, usar o último pacote confirmado. Esse comportamento
precisa ser comprovado em testes, não presumido pela escolha do filesystem.

O estado atual referencia espécies por índice. Acrescentar pets livres
exige definir IDs, coexistência com os seis pets originais, limite de
pacotes e migração do estado salvo. O firmware atual não ganha essa
compatibilidade apenas por receber um arquivo.

## Backlog e aceite

| Entrega | Condição para concluir |
|---|---|
| Editor 8×8 e paleta | Desenhar, desfazer/refazer, salvar e reabrir sem perda |
| Frames e preview | Animações e cores consistentes; controles acessíveis por BOOT/movimento |
| Exportação `.art` | Gerador e build aceitam a exportação; documentação mostra o caminho local |
| Ensaio do instalador web | Firmware correto instalado e recuperado em uma Matrix real |
| Formato/runtime de pacotes | Limites definidos, estados mapeados, renderização e armazenamento validados |
| Envio USB | Transferência confirmada; pacote inválido ou interrompido preserva o anterior |
| Atualização e migração | Versões compatíveis preservam dados; mudanças incompatíveis são comunicadas |
| Guia para iniciantes | Pessoa consegue criar/enviar com cabo USB e recuperar falhas seguindo o guia |

Executar os casos **FUT-01 a FUT-13** de [TESTES.md](TESTES.md#próxima-sprint-testes-planejados).
O aceite depende de teste em placa física, inclusive interrupção de upload
e energia. Se o prazo da sprint não comportar a etapa B, entregar a etapa A
com seu limite de compilação local documentado e manter o envio direto no backlog.

## Pendências: case 3D e primeira release

Case em [hardware/case](../hardware/case/README.md), gerada por
`tools/build_case.py` (OpenSCAD) com checagem automática de colisão contra um
modelo simplificado da placa. As medidas vieram do desenho cotado e de fotos;
o que falta depende da peça física:

| Pendência | Por quê / como fechar |
|---|---|
| Imprimir e testar o encaixe na placa real | Posição dos botões (±0,5 mm), altura do botão e espessura da placa são estimativas. Ajustar `btn_dx`, `btn_y`, `sw_h`, `pcb_t` no `.scad` se preciso |
| Confirmar que os pinos acionam BOOT e RESET sem ficar apertando | Ligar a placa dentro da case: não pode entrar em modo de gravação nem reiniciar sozinha |
| Conferir o rasgo do USB com os cabos que você usa | Capas de borracha grandes podem precisar de `usb_open_w`/`usb_open_h` maiores |
| Medir o passo dos LEDs antes de imprimir a versão difusor | Grade assume ~2,7 mm; medir centro do 1º ao 8º LED de uma linha e dividir por 7 (`led_pitch`) |
| Ajustar `plate_fit` à sua impressora | Tampa justa/solta varia entre impressoras |
| Variante em formato de ovo (estilo Tamagotchi) e com bateria | Ideia futura; a atual é compacta e só USB |
| **Primeira release no GitHub** (após aprovação) | Pacote pronto: `python tools/package_release.py v0.1.0` gera `dist/PixelGochi-v0.1.0.zip` (imagem única + binários com offsets do build, STLs, `SHA256SUMS.txt`, notas de [docs/releases/v0.1.0.md](releases/v0.1.0.md)). Falta: aprovação, criar a tag `v0.1.0` e publicar a release com o zip anexado (o `gh` não está instalado; dá pela página de Releases do GitHub) |
| Ensaiar os dois jeitos de gravar do pacote na placa real | "Do zero" com a imagem única (apaga o pet) e "atualizar" gravando só `firmware.bin` em 0x10000 (deve manter o pet) |
