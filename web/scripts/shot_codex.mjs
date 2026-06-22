// Capture le codex des outils (2 pages).
import { firefox } from 'playwright';

const browser = await firefox.launch();
const page = await browser.newPage({ viewport: { width: 1066, height: 600 } });
await page.goto('http://localhost:4173/', { waitUntil: 'networkidle' });
await page.waitForFunction(() => window.__dp2 && window.__dp2.session, { timeout: 15000 });
await page.waitForTimeout(300);

await page.evaluate(() => window.__dp2.session.showTools());
await page.waitForFunction(() => window.__dp2.session.screen === 'tools', { timeout: 5000 });
await page.waitForTimeout(400);
await page.screenshot({ path: 'codex_p1.png' });

await page.keyboard.press('ArrowRight'); // page 2
await page.waitForTimeout(300);
await page.screenshot({ path: 'codex_p2.png' });

await page.keyboard.press('ArrowRight'); // page 3 (nouveaux outils)
await page.waitForTimeout(300);
await page.screenshot({ path: 'codex_p3.png' });

console.log('pages capturées');
await browser.close();
