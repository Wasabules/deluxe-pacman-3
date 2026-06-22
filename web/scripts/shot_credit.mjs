// Vérifie le crédit cliquable « Merci à Neil Roy » sur l'écran d'accueil.
// Usage : node scripts/shot_credit.mjs
import { firefox } from 'playwright';

const browser = await firefox.launch();
const page = await browser.newPage({ viewport: { width: 1066, height: 600 } });
const errors = [];
page.on('console', (m) => m.type() === 'error' && errors.push(m.text()));
page.on('pageerror', (e) => errors.push('PAGEERROR: ' + e.message));

await page.goto('http://localhost:4173/', { waitUntil: 'networkidle' });
await page.waitForTimeout(2500); // écran d'accueil affiché (crédit visible)
await page.screenshot({ path: 'credit.png' });

// Intercepte window.open et clique sur le crédit (centré en bas, y ≈ HEIGHT-26).
await page.evaluate(() => {
  window.__opened = null;
  window.open = (u) => {
    window.__opened = u;
    return null;
  };
});
await page.mouse.click(533, 574);
await page.waitForTimeout(200);

const opened = await page.evaluate(() => window.__opened);
console.log('lien ouvert au clic :', opened ?? '(aucun)');
console.log('erreurs :', errors.length ? errors : 'aucune');
await browser.close();
