// Capture de l'éditeur avec un niveau chargé.
import { chromium } from 'playwright';

const base = process.argv[2] ?? 'http://localhost:4173';
const out = process.argv[3] ?? 'editor.png';
const browser = await chromium.launch();
const page = await browser.newPage({ viewport: { width: 800, height: 680 } });
const errors = [];
page.on('console', (m) => m.type() === 'error' && errors.push(m.text()));
page.on('pageerror', (e) => errors.push('PAGEERROR: ' + e.message));

await page.goto(base + '/editor.html', { waitUntil: 'networkidle' });
await page.waitForTimeout(1500);
// Charge le niveau 1 du jeu pour montrer le rendu + la validation.
await page.evaluate(() => window.__editor.importUrl('Levels/level001.dp2'));
await page.waitForTimeout(800);
await page.screenshot({ path: out });
console.log('Éditeur capturé :', out);
console.log('Erreurs :', errors.length ? errors : 'aucune');
await browser.close();
