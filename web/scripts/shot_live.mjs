// Capture le jeu déployé sur GitHub Pages. Usage : node scripts/shot_live.mjs
import { firefox } from 'playwright';

const URL = process.argv[2] ?? 'https://wasabules.github.io/deluxe-pacman-3/';
const browser = await firefox.launch();
const page = await browser.newPage({ viewport: { width: 1066, height: 600 } });
const errors = [];
page.on('pageerror', (e) => errors.push(e.message));

await page.goto(URL, { waitUntil: 'domcontentloaded', timeout: 30000 });
await page.waitForTimeout(3500);
await page.screenshot({ path: 'live.png' });

const ok = await page.evaluate(() => !!window.__dp2);
console.log('jeu initialisé (__dp2) :', ok, '| erreurs :', errors.length ? errors : 'aucune');
await browser.close();
