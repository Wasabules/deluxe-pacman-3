import { chromium } from 'playwright';
const b = await chromium.launch();
const page = await b.newPage({ viewport: { width: 1066, height: 600 } });
const errs = [];
page.on('pageerror', (e) => errs.push(e.message));
await page.goto('http://localhost:4173/', { waitUntil: 'domcontentloaded' });
await page.waitForFunction(() => !!window.__dp2, { timeout: 25000 });
await page.waitForTimeout(500);

// 1) Menu des modes : intro → menu → Jouer → modes.
await page.evaluate(() => window.__dp2.input.pushKey('enter'));
await page.waitForTimeout(250);
await page.evaluate(() => window.__dp2.input.pushKey('enter'));
await page.waitForTimeout(450);
await page.screenshot({ path: 'reverse_menu.png' });

// 2) Lance le mode Reverse et entre en jeu.
await page.evaluate(() => window.__dp2.session.startGame(1, 'reverse'));
await page.waitForFunction(() => window.__dp2.session.screen === 'levelIntro', { timeout: 8000 });
await page.evaluate(() => window.__dp2.input.pushKey('enter'));
await page.waitForFunction(() => window.__dp2.session.screen === 'playing', { timeout: 8000 });
await page.waitForTimeout(700);
await page.screenshot({ path: 'reverse_game.png' });

const probe = await page.evaluate(() => {
  const s = window.__dp2.session, p = s.play;
  return { reverse: p.reverse, pacmanLives: p.pacmanLives, screen: s.screen, pacX: Math.round(p.pacman.x), redX: Math.round(p.ghosts[0].x) };
});
console.log('reverse:', JSON.stringify(probe), errs.length ? `ERREURS ${errs}` : 'ok');
await b.close();
