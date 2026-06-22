// Capture avec Firefox (reproduction de bugs spécifiques à Firefox).
import { firefox } from 'playwright';

const url = process.argv[2] ?? 'http://localhost:4173/';
const out = process.argv[3] ?? 'firefox.png';
const browser = await firefox.launch();
const page = await browser.newPage({ viewport: { width: 800, height: 600 } });
const errors = [];
page.on('console', (m) => m.type() === 'error' && errors.push(m.text()));
page.on('pageerror', (e) => errors.push('PAGEERROR: ' + e.message));

await page.goto(url, { waitUntil: 'networkidle' });
await page.waitForTimeout(1500);
await page.keyboard.press('Enter'); // intro → menu
await page.waitForTimeout(300);
if (process.env.STOP !== 'menu') {
  await page.keyboard.press('Enter'); // menu → démarrage
  await page.waitForTimeout(2600); // levelIntro → jeu
}
await page.screenshot({ path: out });
console.log('Firefox capturé :', out);
console.log('Erreurs :', errors.length ? errors : 'aucune');
await browser.close();
