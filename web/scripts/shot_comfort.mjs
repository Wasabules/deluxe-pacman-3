// Capture les écrans de confort : READY?, pause, récap de fin de niveau.
import { firefox } from 'playwright';

const browser = await firefox.launch();
const page = await browser.newPage({ viewport: { width: 1066, height: 600 } });
const errors = [];
page.on('console', (m) => m.type() === 'error' && errors.push(m.text()));
await page.goto('http://localhost:4173/', { waitUntil: 'networkidle' });
await page.waitForFunction(() => window.__dp2 && window.__dp2.session, { timeout: 15000 });
await page.waitForTimeout(300);

const waitScreen = (s) => page.waitForFunction((x) => window.__dp2.session.screen === x, s, { timeout: 8000 });

await page.evaluate(() => {
  const d = window.__dp2;
  d.session.toMenu();
  d.session.startGame(1);
});
await waitScreen('levelIntro');
await page.waitForTimeout(300);
await page.screenshot({ path: 'c_ready.png' }); // READY?

await page.keyboard.press('Enter'); // skip → playing
await waitScreen('playing');
await page.waitForTimeout(400);
await page.keyboard.press('Escape'); // pause
await page.waitForTimeout(200);
await page.screenshot({ path: 'c_pause.png' });

await page.keyboard.press('Escape'); // reprendre
await page.waitForTimeout(120);
await page.evaluate(() => (window.__dp2.session.play.levelComplete = true)); // fin de niveau
await waitScreen('levelClear');
await page.waitForTimeout(300);
await page.screenshot({ path: 'c_clear.png' });

console.log('erreurs :', errors.length ? errors : 'aucune');
await browser.close();
