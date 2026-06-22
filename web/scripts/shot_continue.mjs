// Vérifie que « CONTINUER » apparaît dans le menu après une partie sauvegardée.
import { firefox } from 'playwright';

const browser = await firefox.launch();
const page = await browser.newPage({ viewport: { width: 1066, height: 600 } });
await page.goto('http://localhost:4173/', { waitUntil: 'networkidle' });
await page.waitForFunction(() => window.__dp2 && window.__dp2.session, { timeout: 15000 });
await page.waitForTimeout(300);

// Lance une partie (sauvegarde au début du niveau) puis revient au menu.
await page.evaluate(() => {
  const d = window.__dp2;
  d.session.toMenu();
  d.session.startGame(1);
});
await page.waitForFunction(() => window.__dp2.session.screen === 'levelIntro', { timeout: 8000 });
await page.evaluate(() => window.__dp2.session.toMenu());
await page.waitForFunction(() => window.__dp2.session.screen === 'menu', { timeout: 5000 });
await page.waitForTimeout(700);
await page.screenshot({ path: 'continue.png' });

const saved = await page.evaluate(() => window.__dp2.session.hasSavedGame());
console.log('hasSavedGame :', saved);
await browser.close();
