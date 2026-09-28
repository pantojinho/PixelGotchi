# Testes

[README](../README.md) · [Desenvolvimento](DESENVOLVIMENTO.md) · [Roteiro do projeto](ROADMAP.md) · [English](en/TESTING.md)

Há dois tipos de verificação: os **testes automáticos**, que rodam a cada push
sem placa nenhuma, e o **roteiro na placa física**, que só uma pessoa com a
Matrix na mão consegue fazer. Passar nos automáticos não comprova gravação
USB, sensor, cores reais nem encaixe da case.

## Testes automáticos

Rodam no GitHub Actions a cada push e pull request
([verify.yml](../.github/workflows/verify.yml)). Para rodar no seu computador,
veja [Testes sem placa](DESENVOLVIMENTO.md#testes-sem-placa).

| Suíte | O que garante |
|---|---|
| Arte gerada | `tools/gen_art.py` valida `art/*.art` e as saídas versionadas estão em dia |
| `tools/test_controls.py` (C++ real, hardware simulado) | BOOT (debounce, segurar, reset), IMU (gestos, histerese), menu, status, ovo, sono e cochilo, poses sem corte, refeição nas seis espécies sem cobrir o pet, sonhos (capítulos, bolha, 3 min sem apagão), Conway ao redor do pet, contraste dos LEDs e o bichinho do editor (protocolo USB, validação, NVS, adoção, troca ao vivo) |
| `test/test_preview.cjs` | Simulador: seis espécies sem apagão, refeição, cuidados, 3 min de sonho, movimento e BOOT |
| `test/test_petpack.cjs` | Pacote do editor nos seis modelos, validação, simulador e exportação `.art`; gera as fixtures que o teste C++ carrega no firmware |
| `test/test_installer.cjs` e `test_web_installer.py` | Instalador web: consentimento, SHA-256, pacote ausente ou inconsistente |
| Build ESP32-S3 | O firmware compila com PlatformIO |

## Estado na placa física (v0.1.0)

Tudo acima passa. **Na placa física, o roteiro abaixo ainda não foi executado
por completo.** Se você montar um, seus resultados ajudam muito: use o modelo
de issue **Relato de teste** e diga a revisão testada.

## Roteiro na placa

Material: Waveshare ESP32-S3-Matrix, cabo USB-C de dados, computador com
Chrome ou Edge. Anote a revisão (versão da release ou commit).

| ID | O que fazer | Esperado |
|---|---|---|
| HW-01 | Gravar pelo [instalador web](https://pantojinho.github.io/PixelGotchi/install.html) e tocar RESET | Instala até o fim e a seleção de bichinhos aparece |
| HW-02 | Monitor serial a 115200 e RESET | Logs de início, `QMI8658 ok` e a fase do jogo; sem reinício em loop |
| HW-03 | Trocar de espécie (clique e inclinação) e confirmar (segurar e soltar) | Uma troca por entrada; confirma uma vez só, ao soltar |
| HW-04 | Mexer o ovo com calma, parar 15 s, continuar | Incuba com movimento, pausa parado e retoma de onde estava |
| HW-05 | Alimentar gato e capivara | Pet sempre visível; comida à frente da boca, diminui, coração no fim |
| HW-06 | Usar os oito itens do menu e o status | Tudo alcançável só com BOOT; ícones e textos legíveis |
| HW-07 | Chacoalhar duas vezes seguidas e depois de 5 s | A segunda seguida é ignorada; a posterior brinca |
| HW-08 | Virar a tela para baixo 1,5 s e desvirar; depois dormir pelo menu | Sono por gesto acorda ao desvirar; o do menu continua |
| HW-09 | Deixar 2 min parado com energia boa; depois dormir e esperar 3 min | Conway ao redor do pet; no sono, bolha e capítulos do sonho sem apagão |
| HW-10 | Com energia abaixo de 60, deixar 3 min sem mexer | Cochila sozinho; mexer mostra o pet dormindo; BOOT acorda |
| HW-11 | No [editor](EDITOR.md), conectar a placa e enviar um bichinho; depois enviar com "Trocar" | A placa responde; o bichinho aparece como 7ª espécie; com "Trocar", vira um ovo dele |
| HW-12 | Puxar o cabo no meio de um envio do editor e reconectar | O bichinho anterior continua valendo |
| HW-13 | Reiniciar e desligar/religar depois de 5 min de jogo | Pet e bichinho do editor voltam salvos |
| HW-14 | Gravar só o `firmware.bin` da release em 0x10000 | Atualiza mantendo o pet |
| HW-15 | Usar 30 min no USB | Cores confortáveis, sem clarão branco, travamento ou aquecimento anormal |
| HW-16 | Case: imprimir, montar e ligar | Encaixe firme; os pinos não deixam BOOT/RESET apertados; USB acessível |
| HW-17 | No fim, segurar BOOT 8 s | Barra vermelha a partir de 3 s e volta à seleção |

Faça o HW-17 por último: ele apaga o progresso do jogo.

### Modelo de registro

```text
Data / quem testou:
Revisão (release ou commit):
Placa / computador / sistema / navegador:
Cabo / porta / fonte USB:
Casos executados e resultado (passou / falhou / não executado):
Fotos, vídeos e logs:
Falhas: passos, esperado, observado:
Ajustes de orientação ou sensibilidade usados:
```
