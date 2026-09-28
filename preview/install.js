// Textos em português e inglês (i18n.js); sem ele, português.
const L = globalThis.L || ((pt) => pt);
// Verifica o pacote antes de disponibilizar o fluxo USB do ESP Web Tools.
const support = document.querySelector('#support');
const button = document.querySelector('#install');
const consent = document.querySelector('#consent');
const widget = document.querySelector('esp-web-install-button');
const retry = document.querySelector('#retry');
let ready = false;

function updateButton() { button.disabled = !ready || !consent.checked; }
consent.addEventListener('change', updateButton);
widget.addEventListener('click', event => {
  if (!ready || !consent.checked) { event.preventDefault(); event.stopImmediatePropagation(); }
}, true);

async function getJson(path) {
  const response = await fetch(path, { cache: 'no-store' });
  if (!response.ok) throw new Error(L('O pacote de instalação não está disponível.', 'The installation package is not available.'));
  return response.json();
}

async function loadInstaller() {
  ready = false; updateButton(); retry.hidden = true;
  if (!window.isSecureContext) {
    support.textContent = L('Abra esta página em HTTPS ou localhost para liberar o USB.', 'Open this page over HTTPS or localhost to enable USB.'); return;
  }
  if (!('serial' in navigator)) {
    support.textContent = L('Este navegador não oferece USB serial. Use Chrome ou Edge em um computador.', 'This browser has no USB serial support. Use Chrome or Edge on a computer.'); return;
  }
  support.textContent = L('Conferindo o firmware e carregando a conexão USB…', 'Checking the firmware and loading the USB connection…');
  try {
    const [manifest, info] = await Promise.all([getJson('manifest.json'), getJson('firmware-info.json')]);
    const build = manifest.builds?.find(item => item.chipFamily === 'ESP32-S3');
    const part = build?.parts?.[0];
    if (build?.parts?.length !== 1 || part?.offset !== 0 || part.path !== info.path ||
        manifest.version !== info.version || info.chipFamily !== 'ESP32-S3' ||
        info.flashSize !== 4194304 || !Number.isInteger(info.size) || info.size < 65536 ||
        info.size > info.flashSize || !/^firmware\/PixelGotchi-esp32s3-[A-Za-z0-9._-]+\.bin$/.test(info.path) ||
        !/^[a-f0-9]{64}$/.test(info.sha256)) {
      throw new Error(L('O pacote de instalação está inconsistente.', 'The installation package is inconsistent.'));
    }
    const binary = await fetch(new URL(part.path, new URL('manifest.json', location.href)), { cache: 'no-store' });
    if (!binary.ok) throw new Error(L('O arquivo do firmware não está disponível.', 'The firmware file is not available.'));
    const bytes = await binary.arrayBuffer();
    const digest = await crypto.subtle.digest('SHA-256', bytes);
    const hash = [...new Uint8Array(digest)].map(value => value.toString(16).padStart(2, '0')).join('');
    if (bytes.byteLength !== info.size || new Uint8Array(bytes)[0] !== 0xE9 || hash !== info.sha256) {
      throw new Error(L('O arquivo do firmware está incompleto ou não corresponde à versão publicada.', 'The firmware file is incomplete or does not match the published version.'));
    }
    await import('https://unpkg.com/esp-web-tools@10.4.0/dist/web/install-button.js?module');
    await customElements.whenDefined('esp-web-install-button');
    // A biblioteca baixa novamente o binário ao gravar. URLs blob mantêm os
    // mesmos bytes conferidos acima, mesmo se houver outra publicação do site.
    const imageUrl = URL.createObjectURL(new Blob([bytes], {type:'application/octet-stream'}));
    const locked = {...manifest, builds:[{...build, parts:[{path:imageUrl, offset:0}]}]};
    locked.new_install_prompt_erase = false;
    locked.new_install_improv_wait_time = 0;
    widget.manifest = URL.createObjectURL(new Blob([JSON.stringify(locked)], {type:'application/json'}));
    document.querySelector('#version').textContent = `${L('Versão', 'Version')} ${info.version} · Waveshare ESP32-S3-Matrix · flash 4 MB`;
    ready = true; updateButton();
    support.textContent = L('Firmware conferido. Confirme acima, escolha a porta USB e selecione Install na janela. Acompanhe a gravação e a conclusão nessa janela.', 'Firmware checked. Confirm above, choose the USB port and select Install in the dialog. Follow the progress until it finishes in that dialog.');
  } catch (error) {
    support.textContent = `${error.message} ${L('Confira sua conexão ou use a instalação manual.', 'Check your connection or use the manual installation.')}`;
    retry.hidden = false;
  }
}
retry.addEventListener('click', loadInstaller);
loadInstaller();
