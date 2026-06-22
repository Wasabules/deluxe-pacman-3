// Capture la jauge de combo (HUD) et le récap de fin de niveau détaillé.
import { firefox } from 'playwright';

const browser = await firefox.launch();
const page = await browser.newPage({ viewport: { width: 1066, height: 600 } });
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
await page.keyboard.press('Enter');
await waitScreen('playing');

// Force une belle série pour la jauge ×N.
await page.evaluate(() => (window.__dp2.session.play.combo = 18));
await page.waitForTimeout(200);
await page.screenshot({ path: 'score_hud.png' });

// Termine le niveau → récap détaillé.
await page.evaluate(() => (window.__dp2.session.play.levelComplete = true));
await waitScreen('levelClear');
await page.waitForTimeout(300);
await page.screenshot({ path: 'score_clear.png' });

console.log('captures faites');
await browser.close();
