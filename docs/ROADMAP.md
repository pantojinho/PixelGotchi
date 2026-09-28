# Roteiro do projeto

[README](../README.md) · [Testes](TESTES.md) · [Contribuir](../CONTRIBUTING.md) · [English](en/ROADMAP.md)

## No ar hoje (v0.1.0)

Lançado em 28/09/2026: [release v0.1.0](https://github.com/pantojinho/PixelGotchi/releases/tag/v0.1.0)
e [site](https://pantojinho.github.io/PixelGotchi/guia.html), atualizado a cada push no `master`.

- **Firmware** para a Waveshare ESP32-S3-Matrix: seis espécies, ovo que choca
  com movimento, nome pelo DNA, fome/alegria/energia, sujeira, doença,
  descuido e vida selvagem, menu de 8 ícones, status, cochilos, gestos
  (chacoalhar, inclinar, virar) e tudo salvo na NVS.
- **Animações contínuas**: refeição com a comida à frente da boca, pequenas
  intenções quando acordado e sonhos de Conway com capítulos.
- **Bichinho do editor**: recebido pela USB, validado e gravado na placa como
  7ª espécie, sem recompilar.
- **Site bilíngue (PT/EN)**: guia "Comece aqui", simulador, editor de
  bichinhos e instalador USB com o firmware mais recente.
- **Case 3D** (aberta, difusor e com bateria) e pacote de release com imagem
  única, binários e STLs.
- **CI** com testes do firmware (hardware simulado), do site, do editor e do
  instalador, e build do ESP32-S3.

## Falta conferir na placa física

Tudo acima foi verificado em software. O [roteiro na placa](TESTES.md#roteiro-na-placa)
ainda precisa ser executado: gravação pelo navegador, envio pelo editor (a
placa pode reiniciar ao abrir a porta), gestos, cores reais, persistência e
encaixe da case.

## Pendências conhecidas

| Pendência | Como fechar |
|---|---|
| Encaixe da case | Imprimir e medir; ajustar `btn_dx`, `btn_y`, `sw_h`, `pcb_t`, `plate_fit`, `usb_open_w/h` e, no difusor, `led_pitch` nos `.scad` |
| Versão com bateria | Montar com a troca do R3 do TP4056 por 10 kΩ, medir a bateria e a autonomia real (~1,5 h estimada) |
| Nível de bateria no jogo | Divisor 100k/100k num GPIO com ADC (IO1–IO7) e aviso de bateria fraca |
| Amarelos podem parecer esverdeados nos LEDs | Conferir o pintinho com o comando serial `cores RRGGBB …` e ajustar a paleta |
| Pintinho feliz (7 px) corta na borda ao pular no canto | Limitar a posição pela largura do frame atual ou afinar o sprite |
| Atualizar pelo navegador mantendo o pet | Hoje o instalador web grava a imagem completa (apaga tudo); oferecer a gravação só do programa |
| Relógio desligado | Com a placa desligada o tempo do bichinho para; hora pela internet ficaria para uma versão com Wi-Fi |

## Ideias para as próximas versões

- Vários bichinhos do editor na mesma placa e troca pelo menu.
- Orientação 360°: o "chão" gira com a placa; tontura ao virar de ponta-cabeça.
- DNA visível: manchas, listras e cor dos olhos; personalidades com nome.
- Letrinhas no ar: o bichinho "fala" ("OI", "?", "FOME", "♥").
- Reações extras ao sensor: tontura com sacudida forte, pulo com um toque.
- Minijogo: comida caindo do topo, inclinar para ele pegar.
- Evolução ovo → bebê → adulto conforme os cuidados.
- Configuração pelo celular (nome, hora, ciclo dia/noite).
- Importar PNG do LibreSprite/Aseprite para `art/`.
- Case em formato de ovo, estilo Tamagotchi.

Quer pegar alguma? Veja [como contribuir](../CONTRIBUTING.md) e abra uma issue
contando o plano antes de começar as maiores.
