# Contribuindo com o PixelGotchi · Contributing

[Português](#português) · [English](#english)

## Português

Obrigado por querer ajudar! O PixelGotchi é um projeto pequeno e aberto:
qualquer contribuição conta, de um relato de teste a um bichinho novo.
Pode escrever em português ou em inglês.

### Formas de ajudar

| Contribuição | Como |
|---|---|
| **Montou na placa de verdade?** É a ajuda mais valiosa agora | Abra uma issue "Relato de teste" com fotos/vídeo dos LEDs, o que funcionou e o que não. Os roteiros estão em [docs/TESTES.md](docs/TESTES.md) |
| **Um bichinho novo** para vir de fábrica | Desenhe no [editor](https://pantojinho.github.io/PixelGotchi/editor.html), use **Exportar .art** e abra um PR colando o texto no fim de `art/pets.art` |
| Achou um erro | Issue com os passos para repetir, o que esperava e o que aconteceu |
| Documentação e tradução | PR direto: correções, exemplos, textos em inglês que faltam |
| Site, simulador ou editor | PR com captura de tela antes/depois |
| Firmware | PR com testes; para mudanças grandes, converse numa issue antes |

### O que pode mudar à vontade

- Arte: novas espécies (sempre **no fim** da lista `pet`), melhorias nos
  desenhos existentes, efeitos e ícones.
- Documentação, traduções, exemplos e os prompts para IA.
- Visual e usabilidade do site, do simulador e do editor.
- Testes novos e correções de bugs.

### O que pede uma conversa antes (abra uma issue)

- Regras e ritmo do jogo (`src/Config.h`, `src/PetSim.*`): mudam a
  experiência de quem já tem um bichinho.
- Formato salvo na placa (`PetState`, `STATE_VERSION`) e partições da flash:
  exigem pensar em migração para não apagar pets de ninguém.
- Formato do pacote do editor e o protocolo USB (`src/CustomPet.h`,
  `preview/petpack.js`): mudança incompatível precisa de nova versão de
  protocolo nos dois lados.
- Novas dependências, troca de ferramentas de build ou refatorações grandes.

### O que não mudar

- **Brilho e corrente acima dos limites** (`art/led-profile.json`,
  `MAX_MILLIAMPS`): a Waveshare avisa que brilho alto esquenta e pode
  danificar a placa.
- **Versão do FastLED (3.6.0)**: versões novas caíram num driver que
  "estoura" os LEDs para branco nesta placa.
- **Ordem das espécies existentes**: o estado salvo usa o índice.
- **Arquivos gerados à mão** (`src/art/ArtData.*`, `src/art/LedProfile.h`,
  `preview/art.js`): edite `art/*.art` e rode `python tools/gen_art.py`.
- Remover ou desligar testes para a CI passar.

### Passo a passo de um PR

1. Faça um fork e crie um branch a partir do `master`.
2. Rode o projeto local ([Desenvolvimento](docs/DESENVOLVIMENTO.md)):
   `python tools/preview.py` abre o site com recarga automática.
3. Mantenha **firmware (`src/`) e simulador (`preview/`) equivalentes**
   quando a mudança aparece na tela: o simulador é a vitrine do jogo.
4. Textos do site vão em **português e inglês** (`data-en="…"` no HTML e
   `L('português', 'English')` no JavaScript; veja `preview/i18n.js`).
   Documentos novos: se puder, a versão em `docs/en/`.
5. Rode os mesmos comandos da CI e confira que passam:

   ```sh
   python tools/gen_art.py
   git diff --exit-code -- src/art/ArtData.h src/art/ArtData.cpp src/art/LedProfile.h preview/art.js
   python tools/test_controls.py
   node test/test_preview.cjs
   node test/test_petpack.cjs
   node --experimental-vm-modules test/test_installer.cjs
   python -m unittest discover -s test -p test_web_installer.py
   python -m platformio run -e esp32-s3-matrix
   ```

6. Abra o PR explicando o quê e por quê, com capturas do simulador quando
   for visual. Diga o que você testou na placa e o que só foi testado em
   software: as duas coisas são bem-vindas, desde que estejam claras.

### Estilo

- Siga o jeito do código ao redor: nomes em inglês, comentários em
  português, explicando o porquê (não o óbvio).
- Pixel art: siga os [critérios para 64 LEDs](art/README.md#critérios-de-desenho).
  A silhueta vem antes das cores; preto é LED apagado, não contorno.
- Mensagens de commit curtas e no imperativo, em português ou inglês.

### Conduta

Seja gentil e paciente, principalmente com quem está montando o primeiro
projeto de eletrônica. Críticas ao código, nunca às pessoas.

### Licença

Ao contribuir, você concorda que a sua contribuição seja distribuída sob a
[licença MIT](LICENSE) do projeto.

---

## English

Thanks for wanting to help! PixelGotchi is a small, open project: every
contribution counts, from a test report to a new pet. English or Portuguese
are both fine.

### Ways to help

| Contribution | How |
|---|---|
| **Built one on a real board?** That is the most valuable help right now | Open a "Test report" issue with photos/video of the LEDs, what worked and what did not. The test scripts are in [docs/TESTES.md](docs/TESTES.md) (Portuguese) |
| **A new pet** to ship by default | Draw it in the [editor](https://pantojinho.github.io/PixelGotchi/editor.html?lang=en), use **Export .art** and open a PR pasting the text at the end of `art/pets.art` |
| Found a bug | Issue with steps to reproduce, what you expected and what happened |
| Docs and translation | Direct PR: fixes, examples, missing English texts |
| Website, simulator or editor | PR with before/after screenshots |
| Firmware | PR with tests; for big changes, talk in an issue first |

### What can change freely

- Art: new species (always **at the end** of the `pet` list), improvements to
  existing drawings, effects and icons.
- Documentation, translations, examples and the AI prompts.
- Look and usability of the website, simulator and editor.
- New tests and bug fixes.

### What needs a conversation first (open an issue)

- Game rules and pace (`src/Config.h`, `src/PetSim.*`): they change the
  experience of people who already have a pet.
- The format saved on the board (`PetState`, `STATE_VERSION`) and the flash
  partitions: they need a migration plan so nobody's pet gets wiped.
- The editor package format and the USB protocol (`src/CustomPet.h`,
  `preview/petpack.js`): an incompatible change needs a new protocol version
  on both sides.
- New dependencies, build tool changes or large refactors.

### What not to change

- **Brightness and current above the limits** (`art/led-profile.json`,
  `MAX_MILLIAMPS`): Waveshare warns that high brightness heats up and can
  damage the board.
- **The FastLED version (3.6.0)**: newer versions fell back to a driver that
  "blows" the LEDs to full white on this board.
- **The order of existing species**: the saved state uses the index.
- **Generated files by hand** (`src/art/ArtData.*`, `src/art/LedProfile.h`,
  `preview/art.js`): edit `art/*.art` and run `python tools/gen_art.py`.
- Removing or disabling tests to make CI pass.

### PR step by step

1. Fork and create a branch from `master`.
2. Run the project locally ([Development](docs/en/DEVELOPMENT.md)):
   `python tools/preview.py` opens the website with auto-reload.
3. Keep **firmware (`src/`) and simulator (`preview/`) equivalent** when the
   change is visible: the simulator is the game's showcase.
4. Website texts come in **Portuguese and English** (`data-en="…"` in HTML and
   `L('português', 'English')` in JavaScript; see `preview/i18n.js`). New
   documents: if you can, add the `docs/en/` version too.
5. Run the same commands as CI (listed above) and make sure they pass.
6. Open the PR explaining what and why, with simulator screenshots when it is
   visual. Say what you tested on a board and what was only tested in
   software: both are welcome, as long as it is clear.

### Style

- Follow the surrounding code: English identifiers, Portuguese comments that
  explain the why (not the obvious). Comments in English are fine too.
- Pixel art: follow the [64-LED guidelines](art/README.en.md#drawing-guidelines).
  Silhouette before colors; black is an unlit LED, not an outline.
- Short, imperative commit messages, in English or Portuguese.

### Conduct

Be kind and patient, especially with people building their first electronics
project. Criticize code, never people.

### License

By contributing, you agree that your contribution is distributed under the
project's [MIT license](LICENSE).
