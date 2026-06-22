// Vérifie qu'après une partie (niveau modifié) puis relance, le terrain repart
// neuf — malgré le cache de niveaux. Usage : node scripts/shot_replay.mjs
import { firefox } from 'playwright';

const browser = await firefox.launch();
const page = await browser.newPage({ viewport: { width: 1066, height: 600 } });
await page.goto('http://localhost:4173/', { waitUntil: 'networkidle' });
await page.waitForTimeout(2000);

const r = await page.evaluate(async () => {
  const wait = (ms) => new Promise((res) => setTimeout(res, ms));
  const d = window.__dp2;
  const waitPlay = async () => {
    for (let i = 0; i < 60; i++) {
      if (d.session.play) return true;
      await wait(50);
    }
    return false;
  };

  d.session.toMenu();
  d.session.startGame(1);
  if (!(await waitPlay())) return { error: 'p1 jamais chargé', screen: d.session.screen };
  const p1 = d.session.play;
  const total = p1.level.map.flat().filter((c) => c.isPill).length;
  // Simule la consommation de toutes les pills (comme le gameplay mute le map).
  for (const row of p1.level.map) for (const c of row) if (c.isPill) { c.tile = 0; c.isPill = false; }
  const afterEat = p1.level.map.flat().filter((c) => c.isPill).length;

  d.session.startGame(1); // relance
  await wait(60); // laisse repartir un cycle de chargement
  if (!(await waitPlay())) return { error: 'p2 jamais chargé', screen: d.session.screen };
  const p2 = d.session.play;
  const replayPills = p2.level.map.flat().filter((c) => c.isPill).length;
  return { total, afterEat, replayPills, distinctInstances: p1.level !== p2.level };
});

console.log('résultat :', JSON.stringify(r));
console.log(r.replayPills === r.total && r.replayPills > 0 ? '✅ terrain neuf à la relance' : '❌ terrain réutilisé');
await browser.close();
