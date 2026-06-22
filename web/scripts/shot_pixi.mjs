// Capture du POC PixiJS, avec choix du navigateur (chromium|firefox).
import { chromium, firefox } from 'playwright';

const which = process.argv[2] ?? 'firefox';
const out = process.argv[3] ?? 'pixi.png';
const browser = await (which === 'chromium' ? chromium : firefox).launch();
const page = await browser.newPage({ viewport: { width: 800, height: 600 } });
const errors = [];
page.on('console', (m) => m.type() === 'error' && errors.push(m.text()));
page.on('pageerror', (e) => errors.push('PAGEERROR: ' + e.message));

await page.goto('http://localhost:4173/pixitest.html', { waitUntil: 'networkidle' });
await page.waitForTimeout(2500); // chargement assets + init WebGL + rendu
await page.screenshot({ path: out });
const info = await page.evaluate(() => {
  const p = window.__pixi;
  return p ? { renderer: p.app.renderer?.name ?? '?', lineSet: p.level?.lineSet } : null;
});
console.log(`[${which}] capturé :`, out, '| info:', JSON.stringify(info));
console.log('Erreurs :', errors.length ? errors : 'aucune');
await browser.close();
