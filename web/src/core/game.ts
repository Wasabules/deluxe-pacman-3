// Simulation de jeu (logique pure, sans dépendance navigateur).
// Transposition fidèle du déplacement de Pacman piloté par pacman.timer (dp2_main.c).

import { MAPX, MAPY, MOVE_PIXELS, MAX_ENERGY, TILE_SIZE, PLAYFIELD_W, PLAYFIELD_H, TELEPORT_UNSET } from './constants';
import type { Level, Tile, MapCoord } from './types';
import type { Dir, Ghost, Pacman } from './entities';
import { createGhost, stepGhost, collides, resetGhostPosition, SCARED_FRAMES, type GhostContext } from './ghosts';
import { dir, getPath, type GhostDir } from './pathfinding';
import {
  Tool,
  MAX_TOOLS,
  MAX_PICKUPS,
  SPAWN_WAIT,
  SPAWN_SCREEN,
  TOOL_USE_FRAMES,
  TIME_USE_FRAMES,
  EXTRA_ALL,
  extraBit,
  isExtraLetter,
  PERSISTENT_TOOLS,
} from './tools';

const HALF = TILE_SIZE / 2; // 16
const MOUTH_DELAY = 2; // pas de mouvement entre deux frames de bouche (~30/s à 60 Hz)
const START_LIVES = 3; // vies de départ (player.lives = 3 dans le C)

// --- Mode Reverse (le joueur incarne le fantôme rouge, le Pacman est une IA) ---
const REVERSE_PAC_LIVES = 3; // captures nécessaires pour gagner le niveau
const REVERSE_PAC_SPEED = 0.95; // le Pacman IA, un peu plus lent → capturable
const REVERSE_DANGER = 5; // distance (cases) en deçà de laquelle le Pacman fuit
const REVERSE_HUNT = 6; // distance en deçà de laquelle le Pacman chasse un fantôme bleu
const REVERSE_CAPTURE_POINTS = 5000; // points par capture du Pacman
// « eyes » fantôme (0=bas 1=gauche 2=droite 3=haut) → direction Pacman.
const EYES_TO_DIR: Dir[] = [1, 2, 3, 0];
const DEFAULT_DIFFICULTY = 2; // 0 facile · 1 moyen · 2 difficile (IA complète)
const DEATH_PAUSE = 90; // frames de pause après une mort
const BULLET_SPEED = 4 * MOVE_PIXELS; // 16 px/frame
const BULLET_DURATION = Math.floor(500 / BULLET_SPEED); // ~31 frames

/** Projectile du pistolet (GUN). */
export interface Bullet {
  active: boolean;
  x: number;
  y: number;
  dx: number;
  dy: number;
  d: number; // frames écoulées
  rot: number;
  flip: boolean;
}

/** Bombe à retardement (outil BOMB). */
export interface Bomb {
  active: boolean;
  x: number; // px (centre)
  y: number;
  timer: number; // frames avant explosion
}

/** Point flottant « +N » à afficher (position en px, repère jeu). */
export interface ScorePop {
  x: number;
  y: number;
  value: number;
  big: boolean; // gros gain (fantôme/objet) → plus visible
}

/** Ce qui est posé sur la case d'apparition. */
export type SpawnKind = 'none' | 'pickup' | 'tool';

export interface PlayState {
  level: Level;
  pacman: Pacman;
  ghosts: Ghost[];
  desiredDir: Dir | -1; // dernière direction demandée (persistante, comme `direction` en C)
  wantFast: boolean; // CTRL maintenu
  score: number;
  energy: number;
  pillsLeft: number;
  lives: number;
  ghostScore: number; // points du prochain fantôme mangé (double à chaque)
  combo: number; // série de gains consécutifs (multiplicateur de combo)
  comboTimer: number; // frames depuis le dernier gain (décroissance de la série)
  levelTime: number; // frames écoulées dans le niveau (bonus de rapidité)
  diedThisLevel: boolean; // le joueur est-il mort durant ce niveau (bonus perfection) ?
  scorePops: ScorePop[]; // points flottants à afficher ce frame
  difficulty: number;
  bonusLevel: number; // 0 = niveau normal, >0 = numéro du niveau bonus
  gameOver: boolean; // ce joueur n'a plus de vies
  levelComplete: boolean; // toutes les pills mangées (ou mort sur niveau bonus)
  awaitingRespawn: boolean; // pause de mort écoulée : l'app doit respawn/changer de joueur
  deathPause: number; // frames restantes de pause après une mort
  moveAcc: number; // accumulateur de pas (gère le ×1.5 du mode rapide)
  ghostMoveAcc: number; // accumulateur de pas des fantômes (gère le ×0.5 de GLUE)
  ghostSpeed: number; // facteur de vitesse des fantômes (mode Survie)
  rng: () => number;
  // --- outils & pickups ---
  spawnKind: SpawnKind; // ce qui est posé sur la case d'apparition
  spawnCurrent: number; // index du pickup (0..34) ou de l'outil (1..25)
  spawnTimer: number; // horloge d'apparition/affichage (frames)
  toolInUse: boolean[]; // index par Tool ; effets persistants actifs
  toolTimer: number; // frames restantes d'effet persistant
  pvalue: number; // valeur d'un pickup alimentaire
  extra: number; // bits des lettres EXTRA collectées
  extraTime: boolean; // le joueur a obtenu le tool TIME
  bullet: Bullet;
  bomb: Bomb; // bombe à retardement posée (outil BOMB)
  wantFire: boolean; // espace maintenu
  requestJump: number; // 0 = aucun, >0 = niveau, -1 = niveau bonus aléatoire
  // événements transitoires du dernier step (pour le son / les fx)
  atePill: boolean;
  atePowerpill: boolean;
  ateGhost: boolean;
  atePickup: boolean;
  ateTool: Tool | 0;
  gotExtraLife: boolean;
  died: boolean;
  teleported: boolean;
  fired: boolean;
  bombExploded: boolean; // la bombe a explosé ce frame (fx)
  warped: boolean; // téléportation par l'outil WARP ce frame (fx)
  spawnAppeared: SpawnKind; // 'pickup'/'tool' le frame où l'objet apparaît
  justTeleported: boolean; // garde anti-rebond du téléport
  // --- mode Reverse (le joueur incarne le fantôme rouge) ---
  reverse: boolean; // mode Reverse actif
  pacmanDir: Dir | -1; // direction calculée par l'IA du Pacman
  pacmanLives: number; // captures restantes du Pacman avant victoire du joueur
  reverseLost: boolean; // le Pacman a vidé le labyrinthe → le joueur perd
  pacmanFrozen: number; // frames de gel du Pacman (outil Gel — Reverse)
  capturedPacman: boolean; // fx : le Pacman vient d'être capturé ce frame
  playerEaten: boolean; // fx : le fantôme-joueur vient d'être mangé ce frame
}

function makePacman(level: Level): Pacman {
  const p = level.player;
  const px = p.x * TILE_SIZE + HALF;
  const py = p.y * TILE_SIZE + HALF;
  return {
    map: { x: p.x, y: p.y },
    x: px,
    y: py,
    ox: px,
    oy: py,
    dx: 1,
    dy: 0,
    rot: 0,
    flip: false,
    curImg: 1,
    inc: 1,
    animCntr: 0,
    fast: false,
    dead: false,
  };
}

function makeBullet(): Bullet {
  return { active: false, x: 0, y: 0, dx: 0, dy: 0, d: 0, rot: 0, flip: false };
}

function makeBomb(): Bomb {
  return { active: false, x: 0, y: 0, timer: 0 };
}

const BOMB_FUSE = 2 * 60; // 2 s avant explosion
const BOMB_RADIUS = 2.4 * TILE_SIZE; // rayon de l'explosion (px)

/** Données qui survivent au changement de niveau pour un même joueur. */
export interface PlayerCarry {
  score: number;
  lives: number;
  pvalue: number;
  extra: number;
  extraTime: boolean;
}

export interface PlayStateOptions {
  rng?: () => number;
  carry?: PlayerCarry;
  difficulty?: number;
  bonusLevel?: number;
  ghostSpeed?: number; // facteur de vitesse des fantômes (mode Survie : >1)
  reverse?: boolean; // mode Reverse (joueur = fantôme rouge)
}

export function createPlayState(level: Level, opts: PlayStateOptions = {}): PlayState {
  const rng = opts.rng ?? Math.random;
  const carry = opts.carry;
  // Copie défensive : le gameplay consomme les pills (mute `map`). Le Level
  // source peut être partagé (cache/pré-chargement), il ne doit pas être muté —
  // sinon en rejouant on récupérerait le terrain dans son état précédent.
  const own: Level = { ...level, map: level.map.map((row) => row.map((cell) => ({ ...cell }))) };
  return {
    level: own,
    pacman: makePacman(own),
    ghosts: own.ghosts.map((g, i) => createGhost(i, g)),
    desiredDir: -1,
    wantFast: false,
    score: carry?.score ?? 0,
    energy: 0,
    pillsLeft: level.pills,
    lives: carry?.lives ?? START_LIVES,
    ghostScore: 500,
    combo: 0,
    comboTimer: 0,
    levelTime: 0,
    diedThisLevel: false,
    scorePops: [],
    difficulty: opts.difficulty ?? DEFAULT_DIFFICULTY,
    bonusLevel: opts.bonusLevel ?? 0,
    gameOver: false,
    levelComplete: false,
    awaitingRespawn: false,
    deathPause: 0,
    moveAcc: 0,
    ghostMoveAcc: 0,
    ghostSpeed: opts.ghostSpeed ?? 1,
    rng,
    spawnKind: 'none',
    spawnCurrent: 0,
    spawnTimer: 0,
    toolInUse: new Array<boolean>(MAX_TOOLS + 1).fill(false),
    toolTimer: 0,
    pvalue: carry?.pvalue ?? 1000,
    extra: carry?.extra ?? 0,
    extraTime: carry?.extraTime ?? false,
    bullet: makeBullet(),
    bomb: makeBomb(),
    wantFire: false,
    requestJump: 0,
    atePill: false,
    atePowerpill: false,
    ateGhost: false,
    atePickup: false,
    ateTool: 0,
    gotExtraLife: false,
    died: false,
    teleported: false,
    fired: false,
    bombExploded: false,
    warped: false,
    spawnAppeared: 'none',
    justTeleported: false,
    reverse: opts.reverse ?? false,
    pacmanDir: -1,
    pacmanLives: REVERSE_PAC_LIVES,
    reverseLost: false,
    pacmanFrozen: 0,
    capturedPacman: false,
    playerEaten: false,
  };
}

/** Extrait les données à reporter au niveau suivant pour ce joueur. */
export function carryOf(s: PlayState): PlayerCarry {
  return { score: s.score, lives: s.lives, pvalue: s.pvalue, extra: s.extra, extraTime: s.extraTime };
}

/** Remet Pacman et les fantômes à leurs positions de spawn (après une mort). */
export function respawnPlayer(s: PlayState): void {
  s.pacman = makePacman(s.level);
  for (const g of s.ghosts) resetGhostPosition(g);
  s.ghostScore = 500;
  s.desiredDir = -1;
  s.moveAcc = 0;
  s.pacman.dead = false;
  s.awaitingRespawn = false;
  s.deathPause = 0;
  s.toolInUse.fill(false);
  s.toolTimer = 0;
  s.bullet.active = false;
}

/** Une case est-elle traversable ? (vide ou pill, mais pas un mur). */
function isFree(level: Level, x: number, y: number): boolean {
  if (x < 0 || x >= MAPX || y < 0 || y >= MAPY) return false;
  const c = level.map[y][x]!;
  return c.isPill || !c.tile;
}

/** Tente d'appliquer la direction demandée (demi-tour OU aligné au centre, et case
 *  libre). `phase` (outil PHASE) permet de tourner même dans un mur. */
function applyTurn(pac: Pacman, level: Level, dir: Dir, phase: boolean): void {
  const canGo = (x: number, y: number): boolean => phase || isFree(level, x, y);
  switch (dir) {
    case 0: // UP
      if (pac.dy === 1 || (pac.x % 32 === 16 && pac.map.x > 0 && pac.map.x < 22)) {
        if (canGo(pac.map.x, pac.map.y - 1)) {
          pac.dy = -1;
          pac.dx = 0;
          pac.rot = -Math.PI / 2;
          pac.flip = false;
        }
      }
      break;
    case 1: // DOWN
      if (pac.dy === -1 || (pac.x % 32 === 16 && pac.map.x > 0 && pac.map.x < 22)) {
        if (canGo(pac.map.x, pac.map.y + 1)) {
          pac.dy = 1;
          pac.dx = 0;
          pac.rot = Math.PI / 2;
          pac.flip = false;
        }
      }
      break;
    case 2: // LEFT
      if (pac.dx === 1 || (pac.y % 32 === 16 && pac.map.y > 0 && pac.map.y < 16)) {
        if (canGo(pac.map.x - 1, pac.map.y)) {
          pac.dx = -1;
          pac.dy = 0;
          pac.rot = 0;
          pac.flip = true;
        }
      }
      break;
    case 3: // RIGHT
      if (pac.dx === -1 || (pac.y % 32 === 16 && pac.map.y > 0 && pac.map.y < 16)) {
        if (canGo(pac.map.x + 1, pac.map.y)) {
          pac.dx = 1;
          pac.dy = 0;
          pac.rot = 0;
          pac.flip = false;
        }
      }
      break;
  }
}

/** Un pas de déplacement de Pacman (équivaut à un tick de pacman.timer). */
function stepOnce(s: PlayState): void {
  const pac = s.pacman;
  const level = s.level;

  const phase = s.toolInUse[Tool.PHASE] ?? false;
  // En mode Reverse, Pacman est piloté par l'IA (pacmanDir) ; sinon par le joueur.
  const wantDir = s.reverse ? s.pacmanDir : s.desiredDir;
  if (wantDir !== -1) applyTurn(pac, level, wantDir, phase);

  // Blocage par un mur droit devant, une fois centré sur la cellule (sauf PHASE).
  let movex = true;
  let movey = true;
  const tx = pac.map.x + pac.dx;
  const ty = pac.map.y + pac.dy;
  if (!phase && tx >= 0 && tx <= 22 && ty >= 0 && ty <= 16) {
    const cell = level.map[ty][tx]!;
    if (cell.tile && !cell.isPill) {
      if (pac.dx && pac.x % 32 === 16) movex = false;
      if (pac.dy && pac.y % 32 === 16) movey = false;
    }
  }

  pac.ox = pac.x;
  pac.oy = pac.y;
  if (movex) pac.x += pac.dx * MOVE_PIXELS;
  if (movey) pac.y += pac.dy * MOVE_PIXELS;
  pac.map.x = Math.trunc(pac.x / 32);
  pac.map.y = Math.trunc(pac.y / 32);
  // Wrap des tunnels (mêmes bornes que le C).
  if (pac.x > 760) pac.x = -24;
  else if (pac.x < -24) pac.x = 760;
  if (pac.y > 568) pac.y = -24;
  else if (pac.y < -24) pac.y = 568;

  // Téléports : passer d'un téléport à l'autre (garde anti-rebond justTeleported).
  let onTeleport = false;
  for (let i = 0; i < 2; i++) {
    const t = level.teleport[i]!;
    if (t.x < TELEPORT_UNSET && t.x === pac.map.x && t.y === pac.map.y) {
      onTeleport = true;
      if (!s.justTeleported) {
        const o = level.teleport[1 - i]!;
        pac.map.x = o.x;
        pac.map.y = o.y;
        pac.x = pac.ox = o.x * TILE_SIZE + HALF;
        pac.y = pac.oy = o.y * TILE_SIZE + HALF;
        s.teleported = true;
      }
      break; // une seule téléportation par frame (sinon rebond immédiat sur l'autre)
    }
  }
  s.justTeleported = onTeleport;

  const moved = pac.ox !== pac.x || pac.oy !== pac.y;

  // Consommation d'énergie en mode rapide.
  if (pac.fast && s.energy > 0 && moved) {
    s.energy = Math.max(0, s.energy - MOVE_PIXELS);
  }

  // Manger la pill de la cellule courante.
  const cx = pac.map.x;
  const cy = pac.map.y;
  if (cx >= 0 && cx < MAPX && cy >= 0 && cy < MAPY) {
    const cell = level.map[cy][cx]!;
    if (cell.tile && cell.isPill) eatPill(s, cell, cx * TILE_SIZE + HALF, cy * TILE_SIZE + HALF);
  }

  // Aimant : aspire aussi les pills dans un rayon de 2 cases.
  if (s.toolInUse[Tool.MAGNET]) {
    for (let my = cy - 2; my <= cy + 2; my++) {
      for (let mx = cx - 2; mx <= cx + 2; mx++) {
        if (mx < 0 || mx >= MAPX || my < 0 || my >= MAPY) continue;
        const c = level.map[my][mx]!;
        if (c.tile && c.isPill) eatPill(s, c, mx * TILE_SIZE + HALF, my * TILE_SIZE + HALF);
      }
    }
  }

  // Sur la case d'apparition : ramasser le pickup ou l'outil présent.
  if (s.spawnKind !== 'none' && cx === level.pickup.x && cy === level.pickup.y) {
    collectSpawn(s);
  }

  // Animation de la bouche (uniquement en mouvement).
  if (moved) {
    pac.animCntr++;
    if (pac.animCntr >= MOUTH_DELAY) {
      pac.animCntr = 0;
      pac.curImg += pac.inc;
      if (pac.curImg >= 3) pac.inc = -1;
      else if (pac.curImg <= 0) pac.inc = 1;
    }
  }
}

// --- Points & multiplicateurs (add_points) ---

const COMBO_DECAY = 100; // frames sans gain avant de perdre la série
const COMBO_STEP = 8; // nombre de gains par palier de multiplicateur
const COMBO_CAP = 4; // série max : multiplicateur ×(1 + CAP) = ×5

/** Multiplicateur de série courant (×1 à ×5). */
export function comboMult(s: PlayState): number {
  return 1 + Math.min(COMBO_CAP, Math.floor(s.combo / COMBO_STEP));
}

/** Multiplicateur + progression (0..1) vers le palier suivant, pour le HUD. */
export function comboInfo(s: PlayState): { mult: number; progress: number } {
  const mult = comboMult(s);
  const progress = mult >= 1 + COMBO_CAP ? 1 : (s.combo % COMBO_STEP) / COMBO_STEP;
  return { mult, progress };
}

/** Enregistre un gain : prolonge la série. */
function bumpCombo(s: PlayState): void {
  s.combo++;
  s.comboTimer = 0;
}

/** Applique les multiplicateurs ×2/×5/×7 ET la série (combo). */
function applyMultipliers(s: PlayState, points: number): number {
  let m = 1;
  if (s.toolInUse[Tool.TIMES2]) m = 2;
  else if (s.toolInUse[Tool.TIMES5]) m = 5;
  else if (s.toolInUse[Tool.TIMES7]) m = 7;
  return Math.round(points * m * comboMult(s));
}

/** Consomme une pill (normale ou power) : points, série, énergie, compteur, événements. */
function eatPill(s: PlayState, cell: Tile, px: number, py: number): void {
  cell.tile = 0;
  cell.isPill = false;
  if (cell.isPowerpill) {
    cell.isPowerpill = false;
    const pts = applyMultipliers(s, 1000);
    s.score += pts;
    s.energy += 50;
    s.atePowerpill = true;
    s.scorePops.push({ x: px, y: py, value: pts, big: true });
  } else {
    s.score += pillPoints(s); // pop omis (gain fréquent) ; alimente quand même la série
    s.energy += 10;
    s.atePill = true;
  }
  bumpCombo(s);
  if (s.energy > MAX_ENERGY) s.energy = MAX_ENERGY;
  if (s.pillsLeft > 0) s.pillsLeft--;
  // Toutes les pills mangées : niveau réussi… sauf en Reverse, où c'est le Pacman
  // (IA) qui les mange et nous fait perdre.
  if (s.pillsLeft === 0) {
    if (s.reverse) s.reverseLost = true;
    else s.levelComplete = true;
  }
}

/** Points d'une pill normale, selon present/diamants puis multiplicateurs. */
export function pillPoints(s: PlayState): number {
  let pts = 100;
  if (s.toolInUse[Tool.PRESENT]) pts = Math.floor(s.rng() * 4000) + 1000;
  else if (s.toolInUse[Tool.BLUE_DIAMOND]) pts = 1000;
  else if (s.toolInUse[Tool.PINK_DIAMOND]) pts = 2000;
  return applyMultipliers(s, pts);
}

// --- Apparition et collecte des pickups/outils ---

/** Tire un nouveau pickup/outil sur la case d'apparition (pickup_check). */
function spawnNew(s: PlayState): void {
  // 50 % outil, 50 % pickup alimentaire.
  if (s.rng() < 0.5) {
    const t = (Math.floor(s.rng() * MAX_TOOLS) + 1) as Tool;
    // Lettre EXTRA déjà possédée, ou TIME déjà obtenu → bascule en pickup.
    const skip =
      (isExtraLetter(t) && (s.extra & extraBit(t)) !== 0) || (t === Tool.TIME && s.extraTime);
    if (skip) {
      s.spawnKind = 'pickup';
      s.spawnCurrent = Math.floor(s.rng() * MAX_PICKUPS);
    } else {
      s.spawnKind = 'tool';
      s.spawnCurrent = t;
    }
  } else {
    s.spawnKind = 'pickup';
    s.spawnCurrent = Math.floor(s.rng() * MAX_PICKUPS);
  }
  s.spawnTimer = 0;
  s.spawnAppeared = s.spawnKind;
}

/** Fait évoluer l'apparition : attente → apparition → affichage → disparition. */
function updateSpawn(s: PlayState): void {
  s.spawnTimer++;
  if (s.spawnKind === 'none') {
    if (s.spawnTimer >= SPAWN_WAIT) spawnNew(s);
  } else if (s.spawnTimer >= SPAWN_SCREEN) {
    s.spawnKind = 'none'; // expiré ; spawnTimer continue jusqu'au prochain SPAWN_WAIT
  }
}

/** Ramasse ce qui est sur la case d'apparition. */
function collectSpawn(s: PlayState): void {
  if (s.spawnKind === 'pickup') {
    const pts = applyMultipliers(s, s.pvalue);
    s.score += pts;
    s.energy = Math.min(MAX_ENERGY, s.energy + 100);
    s.atePickup = true;
    s.scorePops.push({ x: s.pacman.x, y: s.pacman.y, value: pts, big: true });
    bumpCombo(s);
  } else if (s.spawnKind === 'tool') {
    applyTool(s, s.spawnCurrent as Tool);
  }
  s.spawnKind = 'none';
  s.spawnTimer = 0; // 16 s avant la prochaine apparition
}

/** Active un outil (switch(tool.current) du C). */
export function applyTool(s: PlayState, toolId: Tool): void {
  // RANDOM : remplace par un autre outil tiré au sort.
  let t = toolId;
  while (t === Tool.RANDOM) t = (Math.floor(s.rng() * MAX_TOOLS) + 1) as Tool;
  s.ateTool = t;

  // Outils persistants : (ré)arme le minuteur d'effet.
  if (PERSISTENT_TOOLS.has(t)) {
    // Un seul groupe de multiplicateur/diamant à la fois : on remet à zéro.
    s.toolInUse.fill(false);
    s.toolInUse[t] = true;
    s.toolTimer = TOOL_USE_FRAMES;
  }

  switch (t) {
    case Tool.SHIELDS:
    case Tool.PRESENT:
    case Tool.BLUE_DIAMOND:
    case Tool.PINK_DIAMOND:
    case Tool.TIMES2:
    case Tool.TIMES5:
    case Tool.TIMES7:
    case Tool.GUN:
      break; // effet géré par toolInUse
    case Tool.SPEED:
      break; // toolInUse[SPEED] accélère Pacman
    case Tool.GLUE:
      break; // toolInUse[GLUE] ralentit les fantômes
    case Tool.FREEZE: {
      let frozen = 0;
      for (const g of s.ghosts) {
        if (!g.dead) {
          g.frozen = true;
          g.eyes = 4;
          frozen++;
        }
      }
      if (frozen === 0) s.toolInUse[Tool.FREEZE] = false;
      break;
    }
    case Tool.AUTISM:
      s.score += applyMultipliers(s, 30000);
      break;
    case Tool.PRECIOUS:
      s.score += applyMultipliers(s, 50000);
      break;
    case Tool.GOLDLEAF:
      s.score += applyMultipliers(s, 40000);
      break;
    case Tool.DYNAMITE:
      for (const g of s.ghosts) {
        if (!g.dead) {
          g.dead = true;
          g.scared = false;
          g.frozen = false;
          s.score += 5000;
        }
      }
      break;
    case Tool.EXTRA_E:
    case Tool.EXTRA_X:
    case Tool.EXTRA_T:
    case Tool.EXTRA_R:
    case Tool.EXTRA_A:
      s.extra |= extraBit(t);
      if ((s.extra & EXTRA_ALL) === EXTRA_ALL) {
        s.lives++;
        s.extra = 0;
        s.gotExtraLife = true;
      }
      break;
    case Tool.TIME:
      s.extraTime = true;
      s.toolTimer = Math.max(s.toolTimer, TIME_USE_FRAMES);
      break;
    case Tool.BONUS:
      s.score += s.level.pills * 100;
      s.requestJump = -1; // niveau bonus
      break;
    case Tool.JUMP:
      s.score += s.level.pills * 100;
      s.requestJump = -2; // niveau normal aléatoire (résolu par l'app)
      break;
    case Tool.SKULL:
      killPacman(s);
      break;
    case Tool.MAGNET:
    case Tool.PANIC:
    case Tool.PHASE:
      break; // effet géré par frame via toolInUse
    case Tool.BOMB:
      s.bomb = { active: true, x: s.pacman.x, y: s.pacman.y, timer: BOMB_FUSE };
      break;
    case Tool.LIGHTNING: {
      // Foudroie le fantôme vivant le plus proche.
      let best = -1;
      let bestD = Infinity;
      s.ghosts.forEach((g, i) => {
        if (g.dead) return;
        const d = (g.x - s.pacman.x) ** 2 + (g.y - s.pacman.y) ** 2;
        if (d < bestD) {
          bestD = d;
          best = i;
        }
      });
      const g = s.ghosts[best];
      if (g) {
        g.dead = true;
        g.scared = false;
        g.frozen = false;
        s.score += applyMultipliers(s, 5000);
      }
      break;
    }
    case Tool.WARP: {
      // Téléporte Pacman sur une cellule libre (couloir) tirée au hasard.
      for (let tries = 0; tries < 80; tries++) {
        const rx = 1 + Math.floor(s.rng() * (MAPX - 2));
        const ry = 1 + Math.floor(s.rng() * (MAPY - 2));
        const cell = s.level.map[ry]![rx]!;
        if (!cell.tile || cell.isPill) {
          s.pacman.map.x = rx;
          s.pacman.map.y = ry;
          s.pacman.x = s.pacman.ox = rx * TILE_SIZE + HALF;
          s.pacman.y = s.pacman.oy = ry * TILE_SIZE + HALF;
          s.warped = true;
          break;
        }
      }
      break;
    }
  }
}

/** Met à jour les effets temporisés (expiration des outils) et le pistolet. */
function updateTools(s: PlayState): void {
  if (s.toolTimer > 0) {
    s.toolTimer--;
    if (s.toolTimer === 0) {
      s.toolInUse.fill(false);
      // Dégèle les fantômes restants.
      for (const g of s.ghosts) g.frozen = false;
    }
  }
}

// --- Pistolet (GUN) ---

function fireBullet(s: PlayState): void {
  if (!s.toolInUse[Tool.GUN] || s.bullet.active) return;
  const pac = s.pacman;
  s.bullet = {
    active: true,
    x: pac.x,
    y: pac.y,
    dx: pac.dx * BULLET_SPEED,
    dy: pac.dy * BULLET_SPEED,
    d: 0,
    rot: pac.rot,
    flip: pac.flip,
  };
  s.fired = true;
}

function moveBullet(s: PlayState): void {
  const b = s.bullet;
  if (!b.active) return;
  b.x += b.dx;
  b.y += b.dy;
  b.d++;
  if (b.d >= BULLET_DURATION) {
    b.active = false;
    return;
  }
  // Wrap dans la zone de jeu.
  if (b.x >= PLAYFIELD_W) b.x = 0;
  else if (b.x < 0) b.x = PLAYFIELD_W - 1;
  if (b.y >= PLAYFIELD_H) b.y = 0;
  else if (b.y < 0) b.y = PLAYFIELD_H - 1;

  // Collision avec un fantôme : il meurt et rapporte des points.
  for (const g of s.ghosts) {
    if (g.dead) continue;
    const xd = g.x + 25 - b.x;
    const yd = g.y + 25 - b.y;
    const dist = 25 + 8; // demi-sprite + demi-balle (~15px)
    if (xd < dist && xd > -dist && yd < dist && yd > -dist) {
      g.dead = true;
      g.scared = false;
      g.frozen = false;
      s.score += applyMultipliers(s, Math.floor(s.rng() * 4000) + 1000);
      b.active = false;
      break;
    }
  }
}

function killPacman(s: PlayState): void {
  s.combo = 0; // la mort casse la série
  s.comboTimer = 0;
  s.diedThisLevel = true;
  // Sur un niveau bonus, mourir ne coûte pas de vie : on sort simplement du niveau.
  if (s.bonusLevel > 0) {
    s.pacman.dead = true;
    s.levelComplete = true;
    return;
  }
  s.lives--;
  s.died = true;
  s.pacman.dead = true;
  if (s.lives < 0) s.gameOver = true;
  else s.deathPause = DEATH_PAUSE;
}

/** Fait avancer la bombe posée ; à l'explosion, détruit les fantômes (et Pacman) du rayon. */
function updateBomb(s: PlayState): void {
  if (!s.bomb.active) return;
  s.bomb.timer--;
  if (s.bomb.timer > 0) return;
  s.bomb.active = false;
  s.bombExploded = true;
  for (const g of s.ghosts) {
    if (g.dead) continue;
    if (Math.hypot(g.x + 16 - s.bomb.x, g.y + 16 - s.bomb.y) <= BOMB_RADIUS) {
      g.dead = true;
      g.scared = false;
      g.frozen = false;
      s.score += applyMultipliers(s, 5000);
    }
  }
  // La bombe est dangereuse pour tous : Pacman doit s'en éloigner.
  if (!s.pacman.dead && Math.hypot(s.pacman.x - s.bomb.x, s.pacman.y - s.bomb.y) <= BOMB_RADIUS) {
    killPacman(s);
  }
}

/** Déclenche la frayeur de tous les fantômes vivants (powerpill mangée). */
function frightenGhosts(s: PlayState): void {
  s.ghostScore = 500;
  for (const g of s.ghosts) {
    if (!g.dead) {
      g.scared = true;
      g.stimer = 0;
      g.stime = SCARED_FRAMES;
      g.inc = 1;
      g.curImg = 0;
    }
  }
}

/** Gère la collision Pacman ↔ fantôme : manger le fantôme bleu, ou mourir. */
function handleCollision(s: PlayState, g: Ghost): void {
  if (g.scared) {
    g.dead = true;
    g.scared = false;
    g.frozen = false;
    const pts = Math.round(s.ghostScore * comboMult(s));
    s.score += pts;
    s.ghostScore *= 2;
    s.ateGhost = true;
    s.scorePops.push({ x: g.x + 16, y: g.y + 16, value: pts, big: true });
    bumpCombo(s);
    // Les yeux repartent vers le spawn.
    g.eyes = 0;
  } else if (s.toolInUse[Tool.SHIELDS] || g.frozen) {
    // Le bouclier protège, un fantôme gelé est inoffensif.
  } else {
    killPacman(s);
  }
}

/** Avance la simulation d'une frame (60 Hz). Le mode rapide effectue 1,5 pas/frame. */
export function stepGame(s: PlayState): void {
  s.atePill = false;
  s.atePowerpill = false;
  s.ateGhost = false;
  s.atePickup = false;
  s.ateTool = 0;
  s.gotExtraLife = false;
  s.died = false;
  s.teleported = false;
  s.fired = false;
  s.bombExploded = false;
  s.warped = false;
  s.spawnAppeared = 'none';
  s.capturedPacman = false;
  s.playerEaten = false;
  s.scorePops.length = 0; // pops du frame précédent consommés

  if (s.gameOver || s.levelComplete || s.awaitingRespawn || s.reverseLost) return;

  updateBomb(s);
  s.levelTime++;
  // Décroissance de la série : retombe après COMBO_DECAY frames sans gain.
  if (s.combo > 0 && ++s.comboTimer > COMBO_DECAY) {
    s.combo = 0;
    s.comboTimer = 0;
  }

  // Pause après une mort : on fige, puis on signale à l'app (respawn ou changement de joueur).
  if (s.deathPause > 0) {
    s.deathPause--;
    if (s.deathPause === 0) s.awaitingRespawn = true;
    return;
  }

  const pac = s.pacman;

  // Apparition/expiration des pickups & outils, expiration des effets.
  // (Phase A du mode Reverse : pas d'apparition d'objets — viendra avec les outils Fantôme.)
  if (!s.reverse) updateSpawn(s);
  updateTools(s);

  // Mode Reverse : logique inversée dédiée (joueur = fantôme rouge, Pacman = IA).
  if (s.reverse) {
    stepReverse(s);
    return;
  }

  // Déplacement de Pacman. Vitesse : ×1,5 si CTRL (énergie) et/ou outil SPEED.
  pac.fast = s.wantFast && s.energy > 0;
  let pacSpeed = 1;
  if (pac.fast) pacSpeed *= 1.5;
  if (s.toolInUse[Tool.SPEED]) pacSpeed *= 1.5;
  s.moveAcc += pacSpeed;
  while (s.moveAcc >= 1 && !s.pacman.dead && !s.gameOver && !s.levelComplete) {
    s.moveAcc -= 1;
    stepOnce(s);
  }
  if (s.deathPause > 0 || s.gameOver || s.levelComplete || s.requestJump) return;

  // Une powerpill mangée ce tour effraie les fantômes.
  if (s.atePowerpill) frightenGhosts(s);

  // Pistolet.
  if (s.wantFire) fireBullet(s);
  moveBullet(s);

  // Déplacement des fantômes : 1 pas/frame, ou 1 pas/2 frames sous l'effet GLUE.
  s.ghostMoveAcc += (s.toolInUse[Tool.GLUE] ? 0.5 : 1) * s.ghostSpeed;
  const ctx: GhostContext = {
    level: s.level,
    pacman: pac,
    red: s.ghosts[0]!,
    difficulty: s.difficulty,
    rng: s.rng,
    panic: s.toolInUse[Tool.PANIC] ?? false,
  };
  while (s.ghostMoveAcc >= 1) {
    s.ghostMoveAcc -= 1;
    for (const g of s.ghosts) {
      stepGhost(g, ctx);
      if (collides(pac, g)) {
        handleCollision(s, g);
        if (s.deathPause > 0 || s.gameOver) return; // mort : on arrête la frame
      }
    }
  }
}

// ===================== Mode Reverse =====================
// Le joueur incarne le fantôme rouge (ghost[0]), les 3 autres sont des alliés IA,
// et le Pacman est piloté par une IA qui fuit, mange les pills et chasse les
// fantômes bleus quand il a gobé une super-pastille.

/** Cellule de la pill la plus proche du Pacman (BFS de distances). */
function nearestPillTarget(level: Level, from: MapCoord): MapCoord {
  const grid = getPath(level, from);
  let best: MapCoord | null = null;
  let bestD = Infinity;
  for (let y = 0; y < MAPY; y++) {
    for (let x = 0; x < MAPX; x++) {
      const c = level.map[y]![x]!;
      if (c.isPill && c.tile) {
        const d = grid[y]![x]!;
        if (d >= 0 && d < 999 && d < bestD) {
          bestD = d;
          best = { x, y };
        }
      }
    }
  }
  return best ?? from;
}

/** IA du Pacman (mode Reverse) : renvoie la direction à suivre. */
function pacmanAI(s: PlayState): Dir | -1 {
  const level = s.level;
  const pac = s.pacman;
  const self = { map: pac.map, dx: pac.dx, dy: pac.dy };

  // Fantôme bleu le plus proche (à chasser) et menace la plus proche (à fuir).
  let scaredG: Ghost | null = null;
  let dScared = Infinity;
  let threatG: Ghost | null = null;
  let dThreat = Infinity;
  for (const g of s.ghosts) {
    if (g.dead) continue;
    const d = Math.abs(g.map.x - pac.map.x) + Math.abs(g.map.y - pac.map.y);
    if (g.scared) {
      if (d < dScared) {
        dScared = d;
        scaredG = g;
      }
    } else if (d < dThreat) {
      dThreat = d;
      threatG = g;
    }
  }

  let eyes: GhostDir;
  if (scaredG && dScared <= REVERSE_HUNT) {
    eyes = dir(level, self, scaredG.map, false, s.rng); // chasse le fantôme vulnérable
  } else if (threatG && dThreat <= REVERSE_DANGER) {
    // Fuite : vise la case opposée à la menace (même principe que le PANIC).
    const fx = pac.map.x + (pac.map.x - threatG.map.x);
    const fy = pac.map.y + (pac.map.y - threatG.map.y);
    eyes = dir(level, self, { x: fx, y: fy }, false, s.rng);
  } else {
    eyes = dir(level, self, nearestPillTarget(level, pac.map), false, s.rng); // mange
  }
  return EYES_TO_DIR[eyes]!;
}

/** Capture du Pacman par un fantôme : points, décompte des vies, nouveau round. */
function capturePacman(s: PlayState): void {
  s.capturedPacman = true;
  s.score += applyMultipliers(s, REVERSE_CAPTURE_POINTS);
  s.scorePops.push({ x: s.pacman.x, y: s.pacman.y, value: REVERSE_CAPTURE_POINTS, big: true });
  s.pacmanLives--;
  if (s.pacmanLives <= 0) {
    s.levelComplete = true; // toutes les vies du Pacman épuisées → niveau gagné
    return;
  }
  // Nouveau round : Pacman et fantômes reprennent leurs positions de départ.
  s.pacman = makePacman(s.level);
  for (const g of s.ghosts) resetGhostPosition(g);
  s.pacmanDir = -1;
  s.desiredDir = -1;
  s.moveAcc = 0;
  s.ghostMoveAcc = 0;
}

/** Collision Pacman ↔ fantôme en mode Reverse (logique inversée). */
function handleCollisionReverse(s: PlayState, g: Ghost): void {
  if (g.scared) {
    // Fantôme vulnérable (joueur ou allié) mangé par le Pacman → retour au spawn.
    g.dead = true;
    g.scared = false;
    g.frozen = false;
    g.eyes = 0;
    if (g.index === 0) s.playerEaten = true;
  } else {
    capturePacman(s);
  }
}

/** Avance les entités du mode Reverse (appelée par stepGame). */
function stepReverse(s: PlayState): void {
  const pac = s.pacman;

  // IA du Pacman (immobile s'il est gelé par l'outil Gel).
  if (s.pacmanFrozen > 0) {
    s.pacmanFrozen--;
    s.pacmanDir = -1;
  } else {
    s.pacmanDir = pacmanAI(s);
    s.moveAcc += REVERSE_PAC_SPEED;
    while (s.moveAcc >= 1 && !s.pacman.dead && !s.levelComplete && !s.reverseLost) {
      s.moveAcc -= 1;
      stepOnce(s);
    }
  }
  if (s.levelComplete || s.reverseLost) return;

  // Super-pastille mangée par le Pacman → fantômes (dont le joueur) vulnérables.
  if (s.atePowerpill) frightenGhosts(s);

  // Déplacement des fantômes : ghost[0] piloté par le joueur, 1-3 en IA alliée.
  s.ghostMoveAcc += s.ghostSpeed;
  const ctx: GhostContext = {
    level: s.level,
    pacman: pac,
    red: s.ghosts[0]!,
    difficulty: s.difficulty,
    rng: s.rng,
    panic: false,
  };
  while (s.ghostMoveAcc >= 1) {
    s.ghostMoveAcc -= 1;
    for (let i = 0; i < s.ghosts.length; i++) {
      const g = s.ghosts[i]!;
      stepGhost(g, ctx, i === 0 ? s.desiredDir : -1);
      if (collides(pac, g)) {
        handleCollisionReverse(s, g);
        if (s.levelComplete || s.reverseLost) return;
      }
    }
  }
}
