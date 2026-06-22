// Validation des effets « juice ». Démarre une partie, fait bouger Pacman (qui
// gobe des pills → particules), puis déclenche l'effet de démo (éclat + shake +
// flash + combo). Usage : node scripts/shot_fx.mjs
import { firefox } from 'playwright';

const browser = await firefox.launch();
const page = await browser.newPage({ viewport: { width: 1066, height: 600 } });
const errors = [];
page.on('console', (m) => m.type() === 'error' && errors.push(m.text()));
page.on('pageerror', (e) => errors.push('PAGEERROR: ' + e.message));

await page.goto('http://localhost:4173/', { waitUntil: 'networkidle' });
await page.waitForTimeout(2000);

// Démarre une partie solo et passe en jeu.
await page.evaluate(() => {
  const d = window.__dp2;
  d.session.toMenu();
  d.session.startGame(1);
});
await page.waitForTimeout(800);
await page.keyboard.press('Enter'); // skip l'intro
await page.waitForTimeout(300);

// Fait avancer Pacman pour qu'il gobe des pills (particules jaunes).
await page.keyboard.press('ArrowRight');
await page.waitForTimeout(250);
await page.screenshot({ path: 'fx_pills.png' });

// Déclenche l'effet de démonstration (éclat + tremblement + flash + combo).
await page.evaluate(() => window.__dp2.fx());
await page.waitForTimeout(120); // particules en vol
await page.screenshot({ path: 'fx_burst.png' });

console.log('erreurs :', errors.length ? errors : 'aucune');
await browser.close();
