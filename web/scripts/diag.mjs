// Diagnostic headless : vérifie le chargement audio, la PWA et l'absence d'erreurs.
import { chromium } from 'playwright';

const url = process.argv[2] ?? 'http://localhost:4173/';
const browser = await chromium.launch();
const page = await browser.newPage({ viewport: { width: 800, height: 600 } });

const errors = [];
page.on('console', (m) => {
  if (m.type() === 'error') errors.push(m.text());
});
page.on('pageerror', (e) => errors.push('PAGEERROR: ' + e.message));

await page.goto(url, { waitUntil: 'networkidle' });
await page.waitForTimeout(500);
// Une interaction pour débloquer l'AudioContext.
await page.keyboard.press('Enter');
await page.waitForTimeout(3000); // laisse le temps de décoder les ~30 sons

const info = await page.evaluate(() => {
  const dp2 = window.__dp2;
  return {
    audioLoaded: dp2?.audio?.loadedCount ?? -1,
    audioState: dp2?.audio?.state ?? 'none',
    music: dp2?.audio?.currentMusic ?? null,
    screen: dp2?.session?.screen ?? null,
    swController: !!navigator.serviceWorker?.controller,
    hasManifest: !!document.querySelector('link[rel="manifest"]'),
  };
});

console.log('--- Diagnostic ---');
console.log('Sons décodés        :', info.audioLoaded);
console.log('État AudioContext   :', info.audioState);
console.log('Musique en cours    :', info.music);
console.log('Écran courant       :', info.screen);
console.log('Service worker actif:', info.swController);
console.log('Manifest PWA présent:', info.hasManifest);
console.log('Erreurs console     :', errors.length ? errors : 'aucune');

await browser.close();
