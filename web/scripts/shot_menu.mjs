// Capture du menu animé. Usage : node scripts/shot_menu.mjs <firefox|chromium> <out> <keys csv> <waitMs>
import { chromium, firefox } from 'playwright';

const which = process.argv[2] ?? 'firefox';
const out = process.argv[3] ?? 'menu.png';
const keys = (process.argv[4] ?? '').split(',').filter(Boolean);
const waitMs = Number(process.argv[5] ?? 2000);

const browser = await (which === 'chromium' ? chromium : firefox).launch();
const page = await browser.newPage({ viewport: { width: 800, height: 600 } });
const errors = [];
page.on('console', (m) => (m.type() === 'error' || m.type() === 'warning') && errors.push(`[${m.type()}] ${m.text()}`));
page.on('pageerror', (e) => errors.push('PAGEERROR: ' + e.message));

await page.goto('http://localhost:4173/', { waitUntil: 'networkidle' });
await page.waitForTimeout(1800); // chargement assets + init WebGL + police
for (const k of keys) {
  await page.keyboard.press(k);
  await page.waitForTimeout(200);
}
await page.waitForTimeout(waitMs); // laisse l'animation se développer
await page.screenshot({ path: out });
console.log(`[${which}] ${out} | erreurs:`, errors.length ? errors : 'aucune');
await browser.close();
