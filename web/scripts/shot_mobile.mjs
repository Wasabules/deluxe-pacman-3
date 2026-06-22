// Valide les contrôles tactiles + l'orientation. Émule un téléphone (hasTouch).
// Usage : node scripts/shot_mobile.mjs
import { chromium } from 'playwright';

const URL = 'http://localhost:4173/';
const browser = await chromium.launch();

async function run(name, w, h) {
  const ctx = await browser.newContext({
    viewport: { width: w, height: h },
    hasTouch: true,
    isMobile: true,
    deviceScaleFactor: 2,
  });
  const page = await ctx.newPage();
  const errors = [];
  page.on('pageerror', (e) => errors.push(e.message));
  await page.goto(URL, { waitUntil: 'domcontentloaded' });
  await page.waitForFunction(() => !!window.__dp2, { timeout: 25000 });
  await page.waitForTimeout(800);

  // Démarre une partie classique et force l'entrée en jeu.
  await page.evaluate(() => window.__dp2.session.startGame(1, 'classic'));
  await page.waitForTimeout(1500);
  await page.evaluate(() => window.__dp2.input.pushKey('enter')); // saute le READY?
  await page.waitForTimeout(1200);

  await page.screenshot({ path: `mob_${name}.png` });

  // Contrôles présents + mapping directionnel + boutons.
  const probe = await page.evaluate(() => {
    const inp = window.__dp2.input;
    const dpad = document.querySelector('#touch .dpad');
    const fire = document.querySelector('#touch .fire');
    const r = dpad.getBoundingClientRect();
    const cx = r.left + r.width / 2;
    const cy = r.top + r.height / 2;
    const send = (el, type, x, y) =>
      el.dispatchEvent(new PointerEvent(type, { pointerId: 1, clientX: x, clientY: y, bubbles: true }));
    const dirs = {};
    send(dpad, 'pointerdown', cx + 50, cy); dirs.right = inp.desiredDir;
    send(dpad, 'pointermove', cx, cy - 50); dirs.up = inp.desiredDir;
    send(dpad, 'pointermove', cx - 50, cy); dirs.left = inp.desiredDir;
    send(dpad, 'pointermove', cx, cy + 50); dirs.down = inp.desiredDir;
    send(dpad, 'pointerup', cx, cy + 50);
    send(fire, 'pointerdown', 0, 0); const fireOn = inp.firing;
    send(fire, 'pointerup', 0, 0); const fireOff = inp.firing;
    return {
      touchClass: document.body.classList.contains('touch'),
      hasDpad: !!dpad,
      hasFire: !!fire,
      dirs, // attendu : right=3, up=0, left=2, down=1
      fireOn, // attendu true
      fireOff, // attendu false
      screen: window.__dp2.session.screen,
    };
  });
  console.log(`[${name} ${w}x${h}]`, JSON.stringify(probe), errors.length ? `ERREURS: ${errors}` : 'ok');
  await ctx.close();
}

await run('portrait', 412, 915);
await run('landscape', 915, 412);
await browser.close();
