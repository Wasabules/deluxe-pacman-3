// Internationalisation (i18n). Anglais par défaut, français en second.
// Couche de présentation uniquement : le `core` reste sans texte d'interface.
// Le dictionnaire est un objet typé (pas de clés « magiques ») : chaque langue
// implémente la même interface `Dict`, ce qui garantit qu'aucune chaîne ne manque.

export type Lang = 'en' | 'fr';
export const LANGS: Lang[] = ['en', 'fr'];

export interface Dict {
  langName: string; // nom natif (affiché dans le réglage)

  // --- Menu ---
  arcadeEdition: string;
  pressAnyKey: string;
  thanksNeil: string; // crédit cliquable de l'écran d'accueil
  continueLabel: string;
  play: string;
  toolsMenu: string;
  toolsTitle: string;
  toolsHint: string;
  leaderboard: string;
  selectHint: string; // menu principal
  modeTitle: string;
  modeClassic: string;
  modeTimeAttack: string;
  modeSurvival: string;
  modeDaily: string;
  modeReverse: string;
  reverseLives: string; // libellé HUD : vies du Pacman à attraper
  reversePills: string; // libellé HUD : pastilles restantes (jauge de danger)
  timeLabel: string; // HUD : chrono Time Attack
  numberOfPlayers: string;
  players: (n: number) => string;
  playHint: string; // écran de choix des joueurs
  settings: string;
  difficulty: string;
  sounds: string;
  music: string;
  shadows: string;
  language: string;
  back: string;
  settingsHint: string;
  diffName: (d: number) => string; // Capitalisé (Easy / Facile)
  on: string;
  off: string;
  ok: string;

  // --- HUD ---
  score: string;
  lives: string;
  energy: string;
  tool: string;
  combo: string;
  level: (n: number) => string;
  bonus: (n: number) => string;
  player: (n: number) => string;

  // --- Écrans ---
  paused: string;
  resumeHint: string;
  quitHint: string;
  ready: string;
  go: string;
  levelCleared: string;
  bonusCleared: string;
  clearBaseLabel: string;
  clearPerfectLabel: string;
  clearSpeedLabel: string;
  clearLivesLabel: string;
  clearTotalLabel: string;
  gameOver: string;
  newScore: string;
  playerLabel: (n: number) => string;
  points: (n: number) => string;
  enterName: string;
  validate: string;
  hallOfFame: string;
  difficultyLine: (name: string) => string;
  lvlShort: (n: number) => string;
  pressKey: string;

  // --- Chargements + aide ---
  loadingResources: string;
  loadingLevel: string;
  helpLine: (fps: string) => string;
  renderError1: string;
  renderError2: string;
}

const en: Dict = {
  langName: 'English',
  arcadeEdition: 'ARCADE EDITION',
  pressAnyKey: 'PRESS ANY KEY',
  thanksNeil: 'Thanks to Neil Roy',
  continueLabel: 'CONTINUE',
  play: 'PLAY',
  toolsMenu: 'TOOLS',
  toolsTitle: 'TOOLS & ITEMS',
  toolsHint: '← → page · ESC back',
  leaderboard: 'LEADERBOARD',
  selectHint: 'ENTER = SELECT',
  modeTitle: 'GAME MODE',
  modeClassic: 'CLASSIC',
  modeTimeAttack: 'TIME ATTACK',
  modeSurvival: 'SURVIVAL',
  modeDaily: 'DAILY CHALLENGE',
  modeReverse: 'REVERSE',
  reverseLives: 'PAC-MAN',
  reversePills: 'PELLETS',
  timeLabel: 'TIME',
  numberOfPlayers: 'NUMBER OF PLAYERS',
  players: (n) => `${n} PLAYER${n > 1 ? 'S' : ''}`,
  playHint: 'ENTER = PLAY     ESC = BACK',
  settings: 'SETTINGS',
  difficulty: 'DIFFICULTY',
  sounds: 'SOUNDS',
  music: 'MUSIC',
  shadows: 'SHADOWS',
  language: 'LANGUAGE',
  back: 'BACK',
  settingsHint: 'UP/DOWN = NAVIGATE    LEFT/RIGHT = CHANGE',
  diffName: (d) => ['Easy', 'Normal', 'Hard'][d] ?? '?',
  on: 'ON',
  off: 'OFF',
  ok: 'OK',

  score: 'SCORE',
  lives: 'LIVES',
  energy: 'ENERGY',
  tool: 'TOOL',
  combo: 'COMBO',
  level: (n) => `LEVEL ${n}`,
  bonus: (n) => `BONUS ${n}`,
  player: (n) => `PLAYER ${n}`,

  paused: 'PAUSE',
  resumeHint: 'ESC / ENTER : resume',
  quitHint: 'Q : quit to menu',
  ready: 'READY?',
  go: 'GO!',
  levelCleared: 'LEVEL CLEARED',
  bonusCleared: 'BONUS CLEARED!',
  clearBaseLabel: 'Level',
  clearPerfectLabel: 'No deaths',
  clearSpeedLabel: 'Speed',
  clearLivesLabel: 'Lives',
  clearTotalLabel: 'TOTAL',
  gameOver: 'GAME OVER',
  newScore: 'NEW SCORE!',
  playerLabel: (n) => `Player ${n}`,
  points: (n) => `${n} points`,
  enterName: 'Enter your name:',
  validate: 'ENTER to confirm',
  hallOfFame: 'HALL OF FAME',
  difficultyLine: (name) => `Difficulty: ${name}`,
  lvlShort: (n) => `lv.${n}`,
  pressKey: 'Press any key',

  loadingResources: 'Loading resources…',
  loadingLevel: 'Loading level…',
  helpLine: (fps) => `${fps} fps · arrows/WASD · CTRL dash · SPACE shoot · ESC pause · F fullscreen`,
  renderError1: 'Graphics could not start (WebGL / WebGPU).',
  renderError2: 'Enable hardware acceleration in your browser, update your GPU drivers, then reload.',
};

const fr: Dict = {
  langName: 'Français',
  arcadeEdition: 'ARCADE EDITION',
  pressAnyKey: 'APPUYEZ SUR UNE TOUCHE',
  thanksNeil: 'Merci à Neil Roy',
  continueLabel: 'CONTINUER',
  play: 'JOUER',
  toolsMenu: 'OUTILS',
  toolsTitle: 'OUTILS & OBJETS',
  toolsHint: '← → page · ÉCHAP retour',
  leaderboard: 'CLASSEMENT',
  selectHint: 'ENTRÉE = CHOISIR',
  modeTitle: 'MODE DE JEU',
  modeClassic: 'CLASSIQUE',
  modeTimeAttack: 'CONTRE-LA-MONTRE',
  modeSurvival: 'SURVIE',
  modeDaily: 'DÉFI DU JOUR',
  modeReverse: 'REVERSE',
  reverseLives: 'PAC-MAN',
  reversePills: 'PASTILLES',
  timeLabel: 'TEMPS',
  numberOfPlayers: 'NOMBRE DE JOUEURS',
  players: (n) => `${n} JOUEUR${n > 1 ? 'S' : ''}`,
  playHint: 'ENTRÉE = JOUER     ÉCHAP = RETOUR',
  settings: 'PARAMÈTRES',
  difficulty: 'DIFFICULTÉ',
  sounds: 'SONS',
  music: 'MUSIQUE',
  shadows: 'OMBRES',
  language: 'LANGUE',
  back: 'RETOUR',
  settingsHint: 'HAUT/BAS = NAVIGUER    GAUCHE/DROITE = MODIFIER',
  diffName: (d) => ['Facile', 'Moyen', 'Difficile'][d] ?? '?',
  on: 'ON',
  off: 'OFF',
  ok: 'OK',

  score: 'SCORE',
  lives: 'VIES',
  energy: 'ÉNERGIE',
  tool: 'OUTIL',
  combo: 'COMBO',
  level: (n) => `NIVEAU ${n}`,
  bonus: (n) => `BONUS ${n}`,
  player: (n) => `JOUEUR ${n}`,

  paused: 'PAUSE',
  resumeHint: 'ÉCHAP / ENTRÉE : reprendre',
  quitHint: 'Q : quitter vers le menu',
  ready: 'PRÊT ?',
  go: 'GO !',
  levelCleared: 'NIVEAU TERMINÉ',
  bonusCleared: 'BONUS RÉUSSI !',
  clearBaseLabel: 'Niveau',
  clearPerfectLabel: 'Sans mourir',
  clearSpeedLabel: 'Rapidité',
  clearLivesLabel: 'Vies',
  clearTotalLabel: 'TOTAL',
  gameOver: 'GAME OVER',
  newScore: 'NOUVEAU SCORE !',
  playerLabel: (n) => `Joueur ${n}`,
  points: (n) => `${n} points`,
  enterName: 'Entrez votre nom :',
  validate: 'ENTRÉE pour valider',
  hallOfFame: 'HALL OF FAME',
  difficultyLine: (name) => `Difficulté : ${name}`,
  lvlShort: (n) => `niv.${n}`,
  pressKey: 'Appuyez sur une touche',

  loadingResources: 'Chargement des ressources…',
  loadingLevel: 'Chargement du niveau…',
  helpLine: (fps) => `${fps} fps · flèches/WASD · CTRL rapide · ESPACE tir · ÉCHAP pause · F plein écran`,
  renderError1: "Le moteur graphique n'a pas démarré (WebGL / WebGPU).",
  renderError2: "Activez l'accélération matérielle du navigateur, mettez à jour vos pilotes GPU, puis rechargez.",
};

const DICTS: Record<Lang, Dict> = { en, fr };

const STORAGE_KEY = 'dp2.lang';

function loadLang(): Lang {
  try {
    const v = localStorage.getItem(STORAGE_KEY);
    if (v === 'en' || v === 'fr') return v;
  } catch {
    /* localStorage indisponible : on garde le défaut */
  }
  return 'en'; // anglais par défaut (« english first »)
}

let current: Lang = loadLang();

/** Dictionnaire de la langue courante. Appeler à chaque rendu pour rester réactif. */
export function t(): Dict {
  return DICTS[current];
}

export function getLang(): Lang {
  return current;
}

export function setLang(lang: Lang): void {
  current = lang;
  try {
    localStorage.setItem(STORAGE_KEY, lang);
  } catch {
    /* persistance impossible : sans effet */
  }
}

/** Fait défiler la langue (réglage du menu). */
export function cycleLang(dir: number): void {
  const i = LANGS.indexOf(current);
  const next = LANGS[(i + dir + LANGS.length) % LANGS.length]!;
  setLang(next);
}
