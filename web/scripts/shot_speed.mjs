// Capture la traînée néon du mode rapide. Force l'énergie au max et maintient
// CTRL (fast) + Flèche droite. Usage : node scripts/shot_speed.mjs
import { firefox } from 'playwright';

const browser = await firefox.launch();
const page = await browser.newPage({ viewport: { width: 1066, height: 600 } });
const errors = [];
page.on('console', (m) => m.type() === 'error' && errors.push(m.text()));
page.on('pageerror', (e) => errors.push('PAGEERROR: ' + e.message));

await page.goto('http://localhost:4173/', { waitUntil: 'networkidle' });
await page.waitForTimeout(2000);

await page.evaluate(() => {
  const d = window.__dp2;
  d.session.toMenu();
  d.session.startGame(1);
});
await page.waitForTimeout(800);
await page.keyboard.press('Enter'); // skip l'intro
await page.waitForTimeout(200);

// Mode rapide soutenu : flèche bas + CTRL maintenus, énergie réinjectée. On
// capture tôt (en plein mouvement) pour voir la traînée s'étirer sous Pacman.
await page.keyboard.down('ArrowDown');
await page.keyboard.down('Control');
for (let i = 0; i < 4; i++) {
  await page.evaluate(() => {
    const p = window.__dp2.session.play;
    if (p) p.energy = 500;
  });
  await page.waitForTimeout(55);
}
await page.screenshot({ path: 'speed.png' });
await page.keyboard.up('Control');
await page.keyboard.up('ArrowDown');

console.log('erreurs :', errors.length ? errors : 'aucune');
await browser.close();
