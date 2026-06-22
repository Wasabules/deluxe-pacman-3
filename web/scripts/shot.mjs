// Capture d'écran headless du jeu pour vérifier le rendu.
// Usage : node scripts/shot.mjs [url] [fichier_sortie]
import { chromium } from 'playwright';

const url = process.argv[2] ?? 'http://localhost:4173/';
const out = process.argv[3] ?? 'shot.png';

const browser = await chromium.launch();
const page = await browser.newPage({ viewport: { width: 800, height: 600 }, deviceScaleFactor: 1 });

const errors = [];
page.on('console', (m) => {
  if (m.type() === 'error') errors.push(m.text());
});
page.on('pageerror', (e) => errors.push('PAGEERROR: ' + e.message));

// Le serveur peut mettre un instant à démarrer : on réessaie la navigation.
let ok = false;
for (let i = 0; i < 20 && !ok; i++) {
  try {
    await page.goto(url, { waitUntil: 'networkidle', timeout: 3000 });
    ok = true;
  } catch {
    await page.waitForTimeout(500);
  }
}
if (!ok) {
  console.log('Impossible de joindre', url);
  await browser.close();
  process.exit(1);
}

await page.waitForTimeout(Number(process.env.INIT_MS ?? 1500)); // assets + premier rendu

// Séquence de touches optionnelle : KEYS='Enter,ArrowUp,Enter' (avec délai entre).
if (process.env.KEYS) {
  for (const key of process.env.KEYS.split(',')) {
    await page.keyboard.press(key.trim());
    await page.waitForTimeout(Number(process.env.KEY_GAP ?? 120));
  }
}

// Attente avant action (pour atteindre l'état de jeu).
const waitMs = Number(process.env.WAIT_MS ?? 0);
if (waitMs) await page.waitForTimeout(waitMs);

// Action optionnelle : EVAL exécute du JS dans la page (ex. déclencher une transition).
if (process.env.EVAL) {
  await page.evaluate(process.env.EVAL);
}

// Observation après l'action.
const wait2Ms = Number(process.env.WAIT2_MS ?? 0);
if (wait2Ms) await page.waitForTimeout(wait2Ms);

// Seconde séquence de touches optionnelle (ex. saisie d'un nom puis ENTRÉE).
if (process.env.KEYS2) {
  for (const key of process.env.KEYS2.split(',')) {
    await page.keyboard.press(key.trim());
    await page.waitForTimeout(Number(process.env.KEY_GAP ?? 120));
  }
}
const wait3Ms = Number(process.env.WAIT3_MS ?? 0);
if (wait3Ms) await page.waitForTimeout(wait3Ms);

// Pilotage optionnel : HOLD=ArrowRight HOLD_MS=1000 maintient une touche avant la capture.
const hold = process.env.HOLD;
const holdMs = Number(process.env.HOLD_MS ?? 1000);
if (hold) {
  for (const key of hold.split(',')) {
    await page.keyboard.down(key);
    await page.waitForTimeout(holdMs);
    await page.keyboard.up(key);
  }
}

await page.screenshot({ path: out });
console.log('Screenshot écrit :', out);
if (errors.length) {
  console.log('--- Erreurs page ---');
  for (const e of errors) console.log(e);
} else {
  console.log('Aucune erreur console.');
}
await browser.close();
