// Validation du passage de joueur en multijoueur. Démarre une partie à 2 joueurs
// via le hook de debug, fait « mourir » le joueur 1, et capture l'écran du joueur 2.
// Usage : node scripts/shot_multi.mjs <out>
import { firefox } from 'playwright';

const out = process.argv[2] ?? 'multi.png';
const browser = await firefox.launch();
const page = await browser.newPage({ viewport: { width: 1066, height: 600 } });
const errors = [];
page.on('console', (m) => (m.type() === 'error') && errors.push(`[${m.type()}] ${m.text()}`));
page.on('pageerror', (e) => errors.push('PAGEERROR: ' + e.message));

await page.goto('http://localhost:4173/', { waitUntil: 'networkidle' });
await page.waitForTimeout(2000);

// Démarre une partie à 2 joueurs.
await page.evaluate(() => {
  const d = window.__dp2;
  d.session.toMenu();
  d.session.startGame(2);
});
await page.waitForTimeout(900); // chargement niveau J1 + intro

await page.keyboard.press('Enter'); // skip l'intro J1 → playing
await page.waitForTimeout(500);

const beforeDeath = await page.evaluate(() => ({
  current: window.__dp2.session.current,
  screen: window.__dp2.session.screen,
}));

// Le joueur 1 meurt (garde des vies) → la main doit passer au joueur 2.
await page.evaluate(() => {
  const p = window.__dp2.session.play;
  if (p) p.awaitingRespawn = true;
});

const dump = () =>
  page.evaluate(() => {
    const s = window.__dp2.session;
    return {
      current: s.current,
      screen: s.screen,
      hasPlay: !!s.play,
      players: s.players.map((p) => ({ loaded: p.loaded, hasPlay: !!p.play, elim: p.eliminated, clevel: p.clevel })),
    };
  });

const samples = [];
for (let i = 0; i < 8; i++) {
  await page.waitForTimeout(150);
  samples.push(await dump());
}

await page.screenshot({ path: out });
console.log('avant mort J1 :', beforeDeath);
samples.forEach((s, i) => console.log(`+${(i + 1) * 150}ms :`, JSON.stringify(s)));
console.log('erreurs :', errors.length ? errors : 'aucune');
await browser.close();
