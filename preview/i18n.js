// Português e inglês nas páginas do site, sem build nem bibliotecas.
// Ordem: ?lang=pt|en na URL, escolha salva, idioma do navegador.
// HTML: o texto em português fica no elemento e o inglês em data-en
// (ou data-en-title / data-en-placeholder / data-en-aria-label para atributos).
// JS: L('texto em português', 'English text').
(function () {
  'use strict';
  const KEY = 'pixelgotchi.lang';
  let saved = null;
  try { saved = localStorage.getItem(KEY); } catch (_) {}
  const q = new URLSearchParams(location.search).get('lang');
  const browser = (navigator.language || 'pt').toLowerCase().startsWith('pt') ? 'pt' : 'en';
  const lang = q === 'en' || q === 'pt' ? q : saved === 'en' || saved === 'pt' ? saved : browser;
  if (q === 'en' || q === 'pt') { try { localStorage.setItem(KEY, q); } catch (_) {} }
  window.LANG = lang;
  window.L = (pt, en) => (lang === 'en' ? en : pt);
  document.documentElement.lang = lang === 'en' ? 'en' : 'pt-BR';

  function apply(root = document) {
    if (lang !== 'en') return;
    root.querySelectorAll('[data-en]').forEach(el => { el.innerHTML = el.dataset.en; });
    for (const attr of ['title', 'placeholder', 'aria-label']) {
      const data = 'data-en-' + attr;
      root.querySelectorAll('[' + data + ']').forEach(el => el.setAttribute(attr, el.getAttribute(data)));
    }
    const t = document.querySelector('meta[name="title-en"]');
    if (t) document.title = t.content;
  }
  window.applyI18n = apply;

  // Botões PT | EN em qualquer elemento com a classe "lang-switch".
  function switcher() {
    document.querySelectorAll('.lang-switch').forEach(box => {
      box.innerHTML = '';
      [['pt', 'PT'], ['en', 'EN']].forEach(([code, label], i) => {
        if (i) box.append(' · ');
        const b = document.createElement('a');
        b.href = '?lang=' + code + location.hash;
        b.textContent = label;
        b.setAttribute('aria-label', code === 'pt' ? 'Português' : 'English');
        if (code === lang) b.setAttribute('aria-current', 'true');
        b.onclick = e => {
          e.preventDefault();
          try { localStorage.setItem(KEY, code); } catch (_) {}
          const url = new URL(location.href);
          url.searchParams.set('lang', code);
          location.href = url.toString();
        };
        box.append(b);
      });
    });
  }
  const ready = () => { apply(); switcher(); };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', ready);
  else ready();
})();
