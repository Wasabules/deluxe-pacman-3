// Recherche de chemin des fantômes, transposée de dp2_map.c (get_path / dir).
//
// get_path : flood-fill BFS depuis la destination. Chaque case reçoit sa distance
// (en nombre de pas) jusqu'à la destination, en tenant compte du wrap des bords.
//   - case traversable non atteinte : -1
//   - mur : 999
//   - destination : 0
// dir : à partir de cette grille, choisit la direction de plus court chemin
// pour un fantôme (ou une direction au hasard si random_dir).

import { MAPX, MAPY } from './constants';
import type { Level, MapCoord } from './types';

export type PathGrid = number[][]; // [MAPY][MAPX]

export function traversable(level: Level, x: number, y: number): boolean {
  const c = level.map[y][x]!;
  return c.tile === 0 || c.isPill;
}

/** Construit la grille de distances vers `dest` (flood-fill BFS avec wrap). */
export function getPath(level: Level, dest: MapCoord): PathGrid {
  const grid: PathGrid = [];
  for (let y = 0; y < MAPY; y++) {
    const row: number[] = [];
    for (let x = 0; x < MAPX; x++) row.push(traversable(level, x, y) ? -1 : 999);
    grid.push(row);
  }

  // Destination sur un mur => emplacement invalide (signalé par 1, cf. C).
  if (grid[dest.y][dest.x] !== -1) {
    grid[dest.y][dest.x] = 1;
    return grid;
  }

  grid[dest.y][dest.x] = 0;
  // File FIFO (le stack du C est en réalité une file : push en tête, pop en queue).
  const queue: MapCoord[] = [{ x: dest.x, y: dest.y }];
  let head = 0;
  while (head < queue.length) {
    const cur = queue[head++]!;
    const d = grid[cur.y][cur.x]! + 1;
    // Voisins N, O, E, S avec wrap.
    const neighbors = [
      { x: cur.x, y: cur.y - 1 },
      { x: cur.x - 1, y: cur.y },
      { x: cur.x + 1, y: cur.y },
      { x: cur.x, y: cur.y + 1 },
    ];
    for (const n of neighbors) {
      if (n.x < 0) n.x = MAPX - 1;
      else if (n.x >= MAPX) n.x = 0;
      if (n.y < 0) n.y = MAPY - 1;
      else if (n.y >= MAPY) n.y = 0;
      if (grid[n.y][n.x] === -1) {
        grid[n.y][n.x] = d;
        queue.push(n);
      }
    }
  }
  return grid;
}

// dir() : 0 = bas, 1 = gauche, 2 = droite, 3 = haut.
export type GhostDir = 0 | 1 | 2 | 3;

export interface DirGhost {
  map: MapCoord;
  dx: number;
  dy: number;
}

function clampToGrid(p: MapCoord): MapCoord {
  let x = p.x;
  let y = p.y;
  if (x < 0) x = 0;
  else if (x >= MAPX) x = MAPX - 1;
  if (y < 0) y = 0;
  else if (y >= MAPY) y = MAPY - 1;
  return { x, y };
}

/**
 * Choisit la direction du fantôme vers `destIn`.
 * @param rng générateur 0..1 (pour le mouvement aléatoire) ; rand()%4 du C.
 */
export function dir(level: Level, ghost: DirGhost, destIn: MapCoord, randomDir: boolean, rng: () => number): GhostDir {
  const dest = clampToGrid(destIn);
  const g = clampToGrid(ghost.map);

  // Direction interdite = demi-tour par rapport au déplacement courant.
  let invalid: GhostDir = 0;
  let dirCount = 0;
  let valid: GhostDir = 0;

  // Comptage des sorties possibles (hors demi-tour), pour les couloirs.
  switch (ghost.dx) {
    case -1:
      invalid = 2;
      if (g.x === 0 || traversable(level, g.x - 1, g.y)) {
        dirCount++;
        valid = 1;
      }
      break;
    case 0:
      if (g.x === 0 || traversable(level, g.x - 1, g.y)) {
        dirCount++;
        valid = 1;
      }
      if (g.x === MAPX - 1 || traversable(level, g.x + 1, g.y)) {
        dirCount++;
        valid = 2;
      }
      break;
    case 1:
      invalid = 1;
      if (g.x === MAPX - 1 || traversable(level, g.x + 1, g.y)) {
        dirCount++;
        valid = 2;
      }
      break;
  }
  switch (ghost.dy) {
    case -1:
      invalid = 0;
      if (g.y === 0 || traversable(level, g.x, g.y - 1)) {
        dirCount++;
        valid = 3;
      }
      break;
    case 0:
      if (g.y === 0 || traversable(level, g.x, g.y - 1)) {
        dirCount++;
        valid = 3;
      }
      if (g.y === MAPY - 1 || traversable(level, g.x, g.y + 1)) {
        dirCount++;
        valid = 0;
      }
      break;
    case 1:
      invalid = 3;
      if (g.y === MAPY - 1 || traversable(level, g.x, g.y + 1)) {
        dirCount++;
        valid = 0;
      }
      break;
  }
  if (dirCount === 1) return valid; // une seule issue
  if (dirCount === 0) return invalid; // cul-de-sac : demi-tour

  const grid = getPath(level, dest);
  if (grid[dest.y][dest.x] === 1) randomDir = true; // destination invalide

  const validDir = [false, false, false, false];
  let v = 999;
  let start = 0;

  // bas (0)
  let t = g.y < MAPY - 1 ? grid[g.y + 1][g.x]! : grid[0][g.x]!;
  if (t >= 0 && t < 999) {
    validDir[0] = true;
    if (t < v) {
      v = t;
      start = 0;
    }
  } else if (t === -1) validDir[0] = true;

  // gauche (1)
  t = g.x >= 1 ? grid[g.y][g.x - 1]! : grid[g.y][MAPX - 1]!;
  if (t >= 0 && t < 999) {
    validDir[1] = true;
    if (t < v) {
      v = t;
      start = 1;
    }
  } else if (t === -1) validDir[1] = true;

  // droite (2)
  t = g.x < MAPX - 1 ? grid[g.y][g.x + 1]! : grid[g.y][0]!;
  if (t >= 0 && t < 999) {
    validDir[2] = true;
    if (t < v) {
      v = t;
      start = 2;
    }
  } else if (t === -1) validDir[2] = true;

  // haut (3)
  t = g.y >= 1 ? grid[g.y - 1][g.x]! : grid[MAPY - 1][g.x]!;
  if (t >= 0 && t < 999) {
    validDir[3] = true;
    if (t < v) {
      v = t;
      start = 3;
    }
  } else if (t === -1) validDir[3] = true;

  if (randomDir) start = Math.floor(rng() * 4) % 4;

  for (let i = start; i < 4; i++) if (i !== invalid && validDir[i]) return i as GhostDir;
  for (let i = 0; i < start; i++) if (i !== invalid && validDir[i]) return i as GhostDir;

  return invalid;
}
