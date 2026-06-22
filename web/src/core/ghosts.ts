// Logique des fantômes : déplacement, IA, états (normal/scared/frozen/eyes),
// collision avec Pacman. Transposition du bloc « GHOST TIMER » de dp2_main.c.

import { MAPX, MAPY, MOVE_PIXELS, TILE_SIZE, REDRAW_TIMER } from './constants';
import type { Level, MapCoord } from './types';
import type { Dir, Ghost, Pacman } from './entities';
import { dir, traversable, type GhostDir } from './pathfinding';

// Direction Pacman (0=haut 1=bas 2=gauche 3=droite) → « eyes » fantôme
// (0=bas 1=gauche 2=droite 3=haut). Sert au mode Reverse (joueur = fantôme rouge).
const DIR_TO_EYES = [3, 0, 1, 2] as const;

// Durée de frayeur en frames 60 Hz. Le C : stime = REDRAW_TIMER*0.75 = 375
// incréments, +1 tous les 16 ticks d'une horloge à REDRAW_TIMER (500) Hz, soit
// 375*16/500 = 12 s ⇒ 12 * 60 = 720 frames.
export const SCARED_FRAMES = Math.round(((REDRAW_TIMER * 0.75 * 16) / REDRAW_TIMER) * 60); // = 720
const ANIM_DELAY = 2; // frames entre deux images (≈ IMAGE_DELAY à 60 Hz)

export function createGhost(index: number, spawn: MapCoord): Ghost {
  return {
    index,
    map: { x: spawn.x, y: spawn.y },
    spawn: { x: spawn.x, y: spawn.y },
    x: spawn.x * TILE_SIZE,
    y: spawn.y * TILE_SIZE,
    ox: spawn.x * TILE_SIZE,
    oy: spawn.y * TILE_SIZE,
    dx: 1,
    dy: 0,
    eyes: 4,
    curImg: 0,
    inc: 1,
    animCntr: 0,
    dead: false,
    scared: false,
    frozen: false,
    stimer: 0,
    stime: SCARED_FRAMES,
    r: 19,
  };
}

function eyesToDelta(g: Ghost): void {
  g.dx = 0;
  g.dy = 0;
  switch (g.eyes) {
    case 0:
      g.dy = 1;
      break; // bas
    case 1:
      g.dx = -1;
      break; // gauche
    case 2:
      g.dx = 1;
      break; // droite
    case 3:
      g.dy = -1;
      break; // haut
  }
}

export interface GhostContext {
  level: Level;
  pacman: Pacman;
  red: Ghost; // ghost[0], utilisé par l'IA du cyan
  difficulty: number; // 0 facile, 1 moyen, 2 difficile
  rng: () => number;
  panic: boolean; // outil PANIC : les fantômes fuient Pacman
}

/** Choisit la direction (eyes) selon l'état et l'IA, quand le fantôme est aligné. */
function decide(g: Ghost, ctx: GhostContext): void {
  const { level, pacman, red, difficulty, rng } = ctx;
  const sd = difficulty;

  if (g.dead) {
    // Retour au spawn, avec une part d'errance selon la difficulté/le fantôme.
    let r: boolean;
    if (sd === 0) r = true;
    else if (sd === 1) r = rng() < 0.75; // rand()%4
    else if (g.index === 0) r = false; // rouge : tout droit chez lui
    else if (g.index === 3) r = rng() < 2 / 3; // violet : rand()%3
    else r = rng() < 0.5; // vert/cyan : rand()%2
    g.eyes = dir(level, g, g.spawn, r, rng);
    return;
  }
  if (g.frozen) {
    g.eyes = 4;
    return;
  }
  if (ctx.panic) {
    // Fuite : vise une case dans la direction opposée à Pacman.
    const fx = g.map.x + (g.map.x - pacman.map.x);
    const fy = g.map.y + (g.map.y - pacman.map.y);
    g.eyes = dir(level, g, { x: fx, y: fy }, false, rng);
    return;
  }
  if (g.scared) {
    g.eyes = dir(level, g, pacman.map, true, rng);
    return;
  }

  const target: MapCoord = { x: pacman.map.x, y: pacman.map.y };
  if (sd === 0) {
    g.eyes = dir(level, g, target, true, rng);
  } else if (sd === 1) {
    g.eyes = dir(level, g, target, rng() < 0.75, rng); // rand()%4 ≈ 75% aléatoire
  } else {
    // Difficile : IA par fantôme (façon Pac-Man).
    switch (g.index) {
      case 0: // ROUGE : chasse directe
        g.eyes = dir(level, g, target, false, rng);
        break;
      case 1: // VERT : vise 4 cases devant Pacman
        target.x += pacman.dx * 4;
        target.y += pacman.dy * 4;
        g.eyes = dir(level, g, target, false, rng);
        break;
      case 2: // CYAN : style Inky (vecteur depuis le rouge)
        target.x += pacman.dx * 2;
        target.y += pacman.dy * 2;
        target.x += target.x - red.map.x;
        target.y += target.y - red.map.y;
        g.eyes = dir(level, g, target, false, rng);
        break;
      case 3: // VIOLET : chasse, mais rentre s'il s'approche trop
        if (Math.abs(g.map.x - target.x) + Math.abs(g.map.y - target.y) < 8) {
          target.x = g.spawn.x;
          target.y = g.spawn.y;
        }
        g.eyes = dir(level, g, target, false, rng);
        break;
    }
  }
}

/** Applique la direction du joueur à un fantôme (mode Reverse). Le virage n'est
 *  retenu que si la case visée est libre ; sinon le fantôme garde sa direction. */
function applyPlayerDir(g: Ghost, d: Dir, level: Level): void {
  const e = DIR_TO_EYES[d];
  const dx = e === 1 ? -1 : e === 2 ? 1 : 0;
  const dy = e === 0 ? 1 : e === 3 ? -1 : 0;
  let tx = g.map.x + dx;
  let ty = g.map.y + dy;
  if (tx < 0) tx = MAPX - 1;
  else if (tx >= MAPX) tx = 0;
  if (ty < 0) ty = MAPY - 1;
  else if (ty >= MAPY) ty = 0;
  if (traversable(level, tx, ty)) g.eyes = e;
}

/** Avance un fantôme d'un pas (équivaut à un tick de ghost[0].timer).
 *  `playerDir` (mode Reverse) : si fourni (≠ -1), ce fantôme est piloté par le
 *  joueur au lieu de l'IA (sauf quand il est mort/gelé : retour au spawn en IA). */
export function stepGhost(g: Ghost, ctx: GhostContext, playerDir: Dir | -1 = -1): void {
  const { level } = ctx;
  g.ox = g.x;
  g.oy = g.y;

  // Décision uniquement quand aligné sur une cellule.
  if (g.x % TILE_SIZE === 0 && g.y % TILE_SIZE === 0 && g.map.x >= 0 && g.map.x < MAPX && g.map.y >= 0 && g.map.y < MAPY) {
    if (playerDir !== -1 && !g.dead && !g.frozen) applyPlayerDir(g, playerDir, level);
    else decide(g, ctx);
    eyesToDelta(g);
  }

  // Blocage par un mur droit devant (une fois centré sur la cellule).
  const tx = g.map.x + g.dx;
  const ty = g.map.y + g.dy;
  if (tx >= 0 && tx <= 22 && ty >= 0 && ty <= 16) {
    const cell = level.map[ty][tx]!;
    if (cell.tile && !cell.isPill) {
      if (g.dx && g.x % TILE_SIZE === 0) g.dx = 0;
      if (g.dy && g.y % TILE_SIZE === 0) g.dy = 0;
    }
  }

  if (!g.frozen) {
    g.x += g.dx * MOVE_PIXELS;
    if (g.x > 744) g.x = -40;
    else if (g.x < -40) g.x = 744;
    g.map.x = Math.trunc(g.x / 32);

    g.y += g.dy * MOVE_PIXELS;
    if (g.y > 552) g.y = -40;
    else if (g.y < -40) g.y = 552;
    g.map.y = Math.trunc(g.y / 32);

    // Animation (uniquement en mouvement).
    if (g.ox !== g.x || g.oy !== g.y) {
      g.animCntr++;
      if (g.animCntr >= ANIM_DELAY) {
        g.animCntr = 0;
        g.curImg += g.inc;
      }
    }
  }

  // Décompte de la frayeur.
  if (g.scared) {
    g.stimer++;
    if (g.stimer > g.stime) {
      g.scared = false;
      g.inc = 1;
      g.curImg = 0;
    }
  }

  if (!g.frozen) {
    if (g.curImg === 3 && g.scared) g.inc = -1;
    else if (g.curImg === 4) g.curImg = 0;
    if (g.curImg === 0) g.inc = 1;
  }

  // Résurrection sur le spawn.
  if (g.dead && g.map.x === g.spawn.x && g.map.y === g.spawn.y) g.dead = false;
}

const PACMAN_R = 21;

/** Collision circulaire Pacman ↔ fantôme (dp2_collision.c). */
export function collides(pac: Pacman, g: Ghost): boolean {
  if (g.dead) return false;
  const gx = g.x + 16;
  const gy = g.y + 16;
  const dx = pac.x - gx;
  const dy = pac.y - gy;
  const rt = PACMAN_R + g.r; // 40
  if (Math.abs(dx) > rt || Math.abs(dy) > rt) return false;
  return dx * dx + dy * dy <= rt * rt;
}

/** Réinitialise un fantôme aux positions de spawn (nouveau départ de niveau / après mort). */
export function resetGhostPosition(g: Ghost): void {
  g.map = { x: g.spawn.x, y: g.spawn.y };
  g.x = g.ox = g.spawn.x * TILE_SIZE;
  g.y = g.oy = g.spawn.y * TILE_SIZE;
  g.dx = 1;
  g.dy = 0;
  g.eyes = 4;
  g.dead = false;
  g.scared = false;
  g.frozen = false;
  g.stimer = 0;
  g.curImg = 0;
  g.inc = 1;
}

export type { GhostDir };
