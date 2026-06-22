// Machine d'états de la partie : intro, menu, enchaînement des niveaux,
// multi-joueurs hot-seat, fin de niveau, game over et hall of fame.
// Le `core` (PlayState) gère un niveau ; la Session gère joueurs et transitions.

import type { Level } from '../core/types';
import { createPlayState, stepGame, respawnPlayer, carryOf, type PlayState } from '../core/game';
import type { Input } from '../platform/input';
import { qualifies, insertScore, getTable, type ScoreEntry } from '../platform/hiscore';
import { codexPageCount } from '../render/codex';
import { TICK_HZ, MAX_LIVES } from '../core/constants';

export type Screen =
  | 'intro'
  | 'menu'
  | 'loading'
  | 'levelIntro'
  | 'playing'
  | 'paused'
  | 'levelFlash'
  | 'levelClear'
  | 'gameOver'
  | 'hiscoreEntry'
  | 'hiscoreTable'
  | 'tools';

const INTRO_FRAMES = 120; // ~2 s d'affichage du message de niveau
const FLASH_FRAMES = 108; // ~1,8 s (3 clignotements de 0,3 s)
const CLEAR_FRAMES = 150; // ~2,5 s de récap de fin de niveau
const GAMEOVER_FRAMES = 150;
const MAX_NAME = 12;
const TIMEATTACK_FRAMES = 3 * 60 * 60; // 3 minutes (mode Time Attack)
const SURVIVAL_SPEED_STEP = 0.1; // accélération des fantômes par vague (Survie)

/** Modes de jeu. */
export type GameMode = 'classic' | 'timeattack' | 'survival' | 'daily' | 'reverse';

/** Graine du jour (défi quotidien) : identique pour tous le même jour. */
function dailySeed(): number {
  const d = new Date();
  return d.getFullYear() * 10000 + (d.getMonth() + 1) * 100 + d.getDate();
}

/** PRNG déterministe (LCG) pour le défi du jour. */
function seededRng(seed: number): () => number {
  let s = (seed >>> 0) || 1;
  return () => {
    s = (Math.imul(s, 1664525) + 1013904223) >>> 0;
    return s / 0x100000000;
  };
}

/** Catégorie de classement : les modes spéciaux ont leur propre table. */
function scoreCategory(mode: GameMode, difficulty: number): string {
  return mode === 'classic' ? `classic:${difficulty}` : mode;
}

interface PlayerSlot {
  clevel: number; // niveau normal courant du joueur
  bonusLevel: number; // 0 = normal, >0 = niveau bonus en cours
  wrap: boolean; // a déjà bouclé les 50 niveaux
  play: PlayState; // état du niveau en cours (null tant que !loaded)
  loaded: boolean; // le PlayState a-t-il déjà été créé (1er tour fait) ?
  eliminated: boolean;
  savedCarry?: ReturnType<typeof carryOf>; // score/vies à restaurer (reprise de partie)
}

const SAVE_KEY = 'dp2.save';

interface SavedGame {
  difficulty: number;
  current: number;
  players: { clevel: number; bonusLevel: number; wrap: boolean; eliminated: boolean; carry: ReturnType<typeof carryOf> | null }[];
}

export interface SessionOptions {
  loadLevel: (clevel: number, bonus: number) => Promise<Level>;
  maxLevels: number;
  maxBonus: number;
  difficulty?: number;
  rng?: () => number;
}

export class Session {
  screen: Screen = 'intro';
  players: PlayerSlot[] = [];
  current = 0;
  menuPlayers = 1; // sélection du nombre de joueurs (1..4)
  difficulty: number;
  mode: GameMode = 'classic';
  timeLeft = 0; // frames restantes (mode Time Attack)
  flashOn = true;
  clearGain = 0; // total des points gagnés au dernier niveau (écran de récap)
  clearBonus = false; // le dernier niveau terminé était-il un bonus réussi ?
  clearBase = 0; // bonus de base du niveau
  clearPerfect = 0; // bonus « sans mourir »
  clearSpeed = 0; // bonus de rapidité
  clearLives = 0; // bonus de vies restantes
  private pendingCarry?: ReturnType<typeof carryOf>;

  // état hall of fame
  hiscoreQueue: number[] = []; // indices de joueurs à saisir
  nameInput = '';
  hiscoreTableData: ScoreEntry[] = [];
  toolPage = 0; // page du codex des outils

  private timer = 0;
  private loadGen = 0;
  private readonly opts: SessionOptions;
  private readonly defaultRng: () => number;
  private rng: () => number; // remplacé par un PRNG seedé pour le défi du jour

  constructor(opts: SessionOptions) {
    this.opts = opts;
    this.difficulty = opts.difficulty ?? 2;
    this.defaultRng = opts.rng ?? Math.random;
    this.rng = this.defaultRng;
  }

  /** PlayState courant (pour le rendu du jeu), ou null hors jeu. */
  get play(): PlayState | null {
    return this.players[this.current]?.play ?? null;
  }

  /** Avance la session d'une frame. */
  tick(input: Input): void {
    switch (this.screen) {
      case 'intro':
      case 'menu':
        break; // écrans gérés par le menu animé de l'app (voir MenuController)
      case 'loading':
        break; // attente du chargement asynchrone
      case 'levelIntro':
        if (++this.timer >= INTRO_FRAMES || input.takeKey()) this.screen = 'playing';
        break;
      case 'playing':
        this.tickPlaying(input);
        break;
      case 'paused':
        this.tickPaused(input);
        break;
      case 'levelFlash':
        this.tickFlash();
        break;
      case 'levelClear':
        if (++this.timer >= CLEAR_FRAMES || input.takeKey()) this.loadCurrentLevel(this.pendingCarry);
        break;
      case 'gameOver':
        if (++this.timer >= GAMEOVER_FRAMES) this.beginHiscores();
        break;
      case 'hiscoreEntry':
        this.tickHiscoreEntry(input);
        break;
      case 'hiscoreTable':
        if (input.takeKey()) this.toMenu();
        break;
      case 'tools':
        this.tickTools(input);
        break;
    }
  }

  /** Codex des outils : pages (←/→) et retour au menu (ÉCHAP/ENTRÉE). */
  private tickTools(input: Input): void {
    const pages = codexPageCount();
    let k: string | null;
    while ((k = input.takeKey())) {
      if (k === 'arrowright' || k === 'd') this.toolPage = (this.toolPage + 1) % pages;
      else if (k === 'arrowleft' || k === 'a') this.toolPage = (this.toolPage + pages - 1) % pages;
      else if (k === 'escape' || k === 'enter' || k === ' ') {
        this.toMenu();
        return;
      }
    }
  }

  // --- Intro & menu (transitions exposées à l'app) ---

  /** Passe à l'écran menu (appelé par l'app depuis l'accueil, ou après le hall of fame). */
  toMenu(): void {
    this.screen = 'menu';
    this.menuPlayers = 1;
  }

  /** Affiche le classement (hall of fame) du mode courant. Une touche revient au menu. */
  showLeaderboard(): void {
    this.hiscoreTableData = getTable(this.scoreCat);
    this.screen = 'hiscoreTable';
  }

  /** Affiche le codex des outils depuis le menu. */
  showTools(): void {
    this.toolPage = 0;
    this.screen = 'tools';
  }

  // --- Démarrage d'une partie ---

  /** Démarre une partie. Les modes spéciaux (time attack / survie / défi) sont solo. */
  startGame(numPlayers: number, mode: GameMode = 'classic'): void {
    this.mode = mode;
    this.rng = mode === 'daily' ? seededRng(dailySeed()) : this.defaultRng;
    this.timeLeft = mode === 'timeattack' ? TIMEATTACK_FRAMES : 0;
    const n = mode === 'classic' ? numPlayers : 1;
    this.players = [];
    for (let i = 0; i < n; i++) {
      this.players.push({
        clevel: 1,
        bonusLevel: 0,
        wrap: false,
        play: null as unknown as PlayState, // chargé au 1er tour du joueur
        loaded: false,
        eliminated: false,
      });
    }
    // Survie : on démarre avec une seule vie (lives = 0 ⇒ game over à la prochaine mort).
    if (mode === 'survival') {
      this.players[0]!.savedCarry = { score: 0, lives: 0, pvalue: 1000, extra: 0, extraTime: false };
    }
    this.current = 0;
    this.loadCurrentLevel();
  }

  // --- Chargement de niveau (asynchrone) ---

  private loadCurrentLevel(carryOver?: ReturnType<typeof carryOf>): void {
    const slot = this.players[this.current]!;
    // Reprise de partie : au 1er chargement d'un joueur, applique son carry sauvegardé.
    const carry = carryOver ?? slot.savedCarry;
    slot.savedCarry = undefined;
    // Wrap : au-delà du dernier niveau, on reboucle.
    if (slot.bonusLevel === 0 && slot.clevel > this.opts.maxLevels) {
      slot.clevel = 1;
      slot.wrap = true;
    }
    this.screen = 'loading';
    const gen = ++this.loadGen;
    void this.opts
      .loadLevel(slot.clevel, slot.bonusLevel)
      .then((level) => {
        if (gen !== this.loadGen) return; // chargement périmé
        // Survie : les fantômes accélèrent à chaque vague (niveau).
        const ghostSpeed = this.mode === 'survival' ? 1 + (slot.clevel - 1) * SURVIVAL_SPEED_STEP : 1;
        slot.play = createPlayState(level, {
          rng: this.rng,
          difficulty: this.difficulty,
          bonusLevel: slot.bonusLevel,
          carry,
          ghostSpeed,
          reverse: this.mode === 'reverse',
        });
        slot.loaded = true;
        this.beginLevelIntro();
        this.prefetchNext(slot);
      })
      .catch(() => {
        // Niveau introuvable : on reboucle au niveau 1.
        slot.clevel = 1;
        slot.bonusLevel = 0;
        slot.wrap = true;
        if (gen === this.loadGen) this.loadCurrentLevel(carryOver);
      });
  }

  /** Pré-charge en arrière-plan le prochain niveau normal probable (clevel+1)
   *  pour que la transition soit instantanée. Fire-and-forget : ne sert qu'à
   *  remplir le cache de `loadLevel` (sans effet si celui-ci ne cache pas). */
  private prefetchNext(slot: PlayerSlot): void {
    if (slot.bonusLevel !== 0) return; // en bonus, on revient au clevel courant (déjà chargé)
    const next = slot.clevel + 1;
    if (next > this.opts.maxLevels) return; // wrap : prochain niveau inconnu
    void this.opts.loadLevel(next, 0).catch(() => {});
  }

  private beginLevelIntro(): void {
    // Le libellé affiché est construit par la couche présentation (drawLevelIntro)
    // à partir de current/players, pour suivre la langue courante (i18n).
    this.saveProgress(); // point de sauvegarde stable (début de chaque niveau)
    this.timer = 0;
    this.screen = 'levelIntro';
  }

  // --- Sauvegarde / reprise de partie (localStorage) ---

  private saveProgress(): void {
    if (this.mode !== 'classic') return; // seul le mode classique est repris via « Continuer »
    try {
      const data: SavedGame = {
        difficulty: this.difficulty,
        current: this.current,
        players: this.players.map((p) => ({
          clevel: p.clevel,
          bonusLevel: p.bonusLevel,
          wrap: p.wrap,
          eliminated: p.eliminated,
          carry: p.loaded && p.play ? carryOf(p.play) : null,
        })),
      };
      localStorage.setItem(SAVE_KEY, JSON.stringify(data));
    } catch {
      /* localStorage indisponible : pas de sauvegarde */
    }
  }

  private clearProgress(): void {
    try {
      localStorage.removeItem(SAVE_KEY);
    } catch {
      /* sans effet */
    }
  }

  /** Une partie sauvegardée est-elle disponible (pour le « Continuer » du menu) ? */
  hasSavedGame(): boolean {
    try {
      return localStorage.getItem(SAVE_KEY) !== null;
    } catch {
      return false;
    }
  }

  /** Reprend la partie sauvegardée (ou démarre une partie solo si aucune). */
  continueGame(): void {
    let data: SavedGame | null = null;
    try {
      data = JSON.parse(localStorage.getItem(SAVE_KEY) ?? 'null') as SavedGame | null;
    } catch {
      data = null;
    }
    if (!data || !data.players?.length) {
      this.startGame(1);
      return;
    }
    this.difficulty = data.difficulty;
    this.players = data.players.map((p) => ({
      clevel: p.clevel,
      bonusLevel: p.bonusLevel,
      wrap: p.wrap,
      eliminated: p.eliminated,
      play: null as unknown as PlayState,
      loaded: false,
      savedCarry: p.carry ?? undefined,
    }));
    this.current = Math.min(data.current, this.players.length - 1);
    this.loadCurrentLevel();
  }

  // --- Jeu ---

  private tickPlaying(input: Input): void {
    const slot = this.players[this.current]!;
    const play = slot.play;

    play.desiredDir = input.desiredDir;
    play.wantFast = input.fast;
    play.wantFire = input.firing;

    let k: string | null;
    while ((k = input.takeKey())) {
      if (k === 'escape') {
        this.screen = 'paused'; // ÉCHAP met en pause (ne quitte plus la partie)
        return;
      }
    }

    stepGame(play);

    // Changement de niveau demandé par un outil (BONUS/JUMP).
    if (play.requestJump !== 0) {
      this.handleJump(play.requestJump);
      play.requestJump = 0;
      return;
    }
    if (play.levelComplete) {
      this.timer = 0;
      this.flashOn = true;
      this.screen = 'levelFlash';
      return;
    }
    if (play.gameOver) {
      this.eliminateCurrent();
      return;
    }
    // Mode Reverse : le Pacman a vidé le labyrinthe → défaite (fin de partie).
    if (play.reverseLost) {
      this.eliminateCurrent();
      return;
    }
    if (play.awaitingRespawn) {
      this.afterDeath();
      return;
    }
    // Time Attack : le chrono s'écoule ; à zéro, fin de la partie.
    if (this.mode === 'timeattack' && --this.timeLeft <= 0) {
      this.timeLeft = 0;
      this.eliminateCurrent();
    }
  }

  /** Écran de pause : reprendre (ÉCHAP/ENTRÉE/P) ou quitter vers le menu (Q). */
  private tickPaused(input: Input): void {
    let k: string | null;
    while ((k = input.takeKey())) {
      if (k === 'escape' || k === 'enter' || k === ' ' || k === 'p') {
        this.screen = 'playing';
        return;
      }
      if (k === 'q') {
        this.toMenu();
        return;
      }
    }
  }

  private handleJump(req: number): void {
    const slot = this.players[this.current]!;
    const carry = carryOf(slot.play);
    if (req === -1) {
      // BONUS : part sur un niveau bonus, le clevel normal est conservé.
      slot.bonusLevel = 1 + Math.floor(this.rng() * this.opts.maxBonus);
    } else if (req === -2) {
      // JUMP : saut vers un niveau normal aléatoire différent.
      let n = slot.clevel;
      while (n === slot.clevel) n = 1 + Math.floor(this.rng() * this.opts.maxLevels);
      slot.clevel = n;
      slot.bonusLevel = 0;
    }
    this.loadCurrentLevel(carry);
  }

  private tickFlash(): void {
    this.timer++;
    this.flashOn = Math.floor(this.timer / 18) % 2 === 0; // ~0,3 s par phase
    if (this.timer >= FLASH_FRAMES) this.applyLevelEnd();
  }

  private applyLevelEnd(): void {
    const slot = this.players[this.current]!;
    const play = slot.play;
    const carry = carryOf(play);

    let base = 0;
    let perfect = 0;
    let speed = 0;
    let livesBonus = 0;
    let bonusCleared = false;
    if (slot.bonusLevel > 0) {
      // Fin d'un niveau bonus : récompense si entièrement nettoyé.
      if (play.pillsLeft === 0) {
        base = 50000;
        bonusCleared = true;
        if (carry.lives < MAX_LIVES) carry.lives++;
      }
      slot.bonusLevel = 0; // retour au niveau normal courant
    } else {
      // Niveau normal terminé : base + bonus de perfection / rapidité / vies.
      base = 10000;
      carry.pvalue = Math.min(20000, carry.pvalue + 1000);
      if (!play.diedThisLevel) perfect = 5000;
      speed = Math.max(0, 6000 - Math.floor(play.levelTime / 60) * 80); // -80 pts/s
      livesBonus = Math.max(0, carry.lives) * 1000;
      slot.clevel++;
    }
    const total = base + perfect + speed + livesBonus;
    carry.score += total;

    // Écran de récap (détail des bonus) avant de charger le niveau suivant.
    this.clearBase = base;
    this.clearPerfect = perfect;
    this.clearSpeed = speed;
    this.clearLives = livesBonus;
    this.clearGain = total;
    this.clearBonus = bonusCleared;
    this.pendingCarry = carry;
    this.timer = 0;
    this.screen = 'levelClear';
  }

  // --- Mort & rotation des joueurs ---

  private afterDeath(): void {
    const slot = this.players[this.current]!;
    respawnPlayer(slot.play); // prêt pour son prochain tour
    const next = this.nextLiving(this.current);
    if (next === this.current) {
      this.screen = 'playing'; // seul joueur en lice : on reprend tout de suite
    } else {
      this.switchToPlayer(next);
    }
  }

  private eliminateCurrent(): void {
    this.players[this.current]!.eliminated = true;
    const next = this.nextLiving(this.current);
    if (next < 0) {
      this.endGame();
    } else {
      this.switchToPlayer(next);
    }
  }

  /** Donne la main au joueur `next`. À son tout premier tour son niveau n'est pas
   *  encore chargé (en multi seul le joueur 0 l'est au démarrage) → on le charge ;
   *  sinon on reprend son tour (son PlayState a été préparé après sa mort). */
  private switchToPlayer(next: number): void {
    this.current = next;
    if (this.players[next]!.loaded) {
      this.beginLevelIntro();
    } else {
      this.loadCurrentLevel();
    }
  }

  /** Prochain joueur non éliminé après `from` (circulaire). -1 si aucun. */
  private nextLiving(from: number): number {
    const n = this.players.length;
    for (let i = 1; i <= n; i++) {
      const idx = (from + i) % n;
      if (!this.players[idx]!.eliminated) return idx;
    }
    return -1;
  }

  // --- Fin de partie & hall of fame ---

  private endGame(): void {
    this.clearProgress(); // partie finie : plus de reprise possible
    this.timer = 0;
    this.screen = 'gameOver';
  }

  private get scoreCat(): string {
    return scoreCategory(this.mode, this.difficulty);
  }

  private beginHiscores(): void {
    this.hiscoreQueue = this.players
      .map((p, i) => ({ p, i }))
      .filter(({ p }) => qualifies(p.play.score, this.scoreCat))
      .map(({ i }) => i);
    this.nextHiscore();
  }

  private nextHiscore(): void {
    if (this.hiscoreQueue.length === 0) {
      this.hiscoreTableData = getTable(this.scoreCat);
      this.screen = 'hiscoreTable';
      return;
    }
    this.nameInput = '';
    this.screen = 'hiscoreEntry';
  }

  private tickHiscoreEntry(input: Input): void {
    let k: string | null;
    while ((k = input.takeKey())) {
      if (k === 'enter') {
        const idx = this.hiscoreQueue.shift()!;
        const slot = this.players[idx]!;
        const name = this.nameInput.trim() || 'PLAYER';
        this.hiscoreTableData = insertScore(this.scoreCat, {
          name: name.toUpperCase(),
          score: slot.play.score,
          level: slot.clevel,
        });
        this.nextHiscore();
        return;
      } else if (k === 'backspace') {
        this.nameInput = this.nameInput.slice(0, -1);
      } else if (k.length === 1 && /[a-z0-9 ]/.test(k) && this.nameInput.length < MAX_NAME) {
        this.nameInput += k.toUpperCase();
      }
    }
  }

  /** Indice du joueur en cours de saisie (hall of fame). */
  get hiscorePlayer(): number {
    return this.hiscoreQueue[0] ?? 0;
  }

  /** Score du joueur en cours de saisie. */
  get hiscoreScore(): number {
    return this.players[this.hiscorePlayer]?.play.score ?? 0;
  }
}

export { TICK_HZ };
