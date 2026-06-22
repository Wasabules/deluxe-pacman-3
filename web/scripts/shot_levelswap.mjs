// Vérifie qu'au passage de niveau, le sprite Pacman est figé sur le spawn du
// NOUVEAU niveau pendant le READY? (et non resté à la position de l'ancien).
// Usage : node scripts/shot_levelswap.mjs
import { chromium } from 'playwright';

const browser = await chromium.launch();
const page = await browser.newPage({ viewport: { width: 1066, height: 600 } });
await page.goto('http://localhost:4173/', { waitUntil: 'domcontentloaded' });
await page.waitForFunction(() => !!window.__dp2, { timeout: 25000 });

// Démarre une partie et entre en jeu (niveau 1).
await page.evaluate(() => window.__dp2.session.startGame(1, 'classic'));
await page.waitForFunction(() => window.__dp2.session.screen === 'levelIntro', { timeout: 8000 });
await page.evaluate(() => window.__dp2.input.pushKey('enter'));
await page.waitForFunction(() => window.__dp2.session.screen === 'playing', { timeout: 8000 });

// Déplace Pacman loin de son spawn pour piéger le tampon d'interpolation.
await page.evaluate(() => {
  const p = window.__dp2.session.play;
  p.pacman.x = 700;
  p.pacman.y = 470;
});
await page.waitForTimeout(300);

// Force la fin de niveau (toutes les pills mangées) → flash → clear → niveau 2.
await page.evaluate(() => (window.__dp2.session.play.pillsLeft = 0));
await page.waitForFunction(() => window.__dp2.session.screen === 'levelIntro', { timeout: 12000 });
await page.waitForTimeout(400); // laisse rendre quelques frames de READY?

const r = await page.evaluate(() => {
  const p = window.__dp2.session.play;
  const sprite = window.__dp2.pixiPacman();
  const clevel = p.level; // nouveau niveau
  return {
    screen: window.__dp2.session.screen,
    spawn: { x: Math.round(p.pacman.x), y: Math.round(p.pacman.y) },
    sprite: { x: Math.round(sprite.x), y: Math.round(sprite.y) },
    hasLevel: !!clevel,
  };
});
const dx = Math.abs(r.sprite.x - r.spawn.x);
const dy = Math.abs(r.sprite.y - r.spawn.y);
const ok = dx <= 2 && dy <= 2;
console.log(JSON.stringify(r), `| écart=(${dx},${dy})`, ok ? '✅ sprite sur le spawn' : '❌ sprite décalé');
await page.screenshot({ path: 'levelswap.png' });
await browser.close();
process.exit(ok ? 0 : 1);
