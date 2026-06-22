// Logique pure de l'éditeur de niveaux (remplace Pace 2 / p2_edit.c).
// Auto-tiling des murs, gomme, remplissage de pills, placement des spawns,
// validation. Aucune dépendance navigateur : testable.

import { MAPX, MAPY, MAP_ID, MAP_VER, TELEPORT_UNSET } from '../core/constants';
import type { Level, MapCoord, Tile } from '../core/types';
import { getPath } from '../core/pathfinding';

// Bits de connexion d'un mur (cf. place_line) :
//   1 = haut · 2 = droite · 4 = bas · 8 = gauche.
// La tuile stockée vaut bits+1 (0 = vide). L'index sprite = tuile-1 = bits.

export type SpawnKind = 'player' | 'ghost0' | 'ghost1' | 'ghost2' | 'ghost3' | 'pickup' | 'teleport';

export function initEmptyLevel(): Level {
  const map: Tile[][] = [];
  for (let y = 0; y < MAPY; y++) {
    const row: Tile[] = [];
    for (let x = 0; x < MAPX; x++) row.push({ tile: 0, isPill: false, isPowerpill: false, isProtected: false });
    map.push(row);
  }
  const unset = (): MapCoord => ({ x: TELEPORT_UNSET, y: TELEPORT_UNSET });
  return {
    mapId: MAP_ID,
    mapVer: MAP_VER,
    validated: false,
    lineSet: 0,
    player: unset(),
    ghosts: [unset(), unset(), unset(), unset()],
    pickup: unset(),
    teleport: [unset(), unset()],
    background: 0,
    map,
    pills: 0,
  };
}

const inBounds = (x: number, y: number) => x >= 0 && x < MAPX && y >= 0 && y < MAPY;

/** Recalcule le drapeau « protégé » : une case est protégée ssi un spawn s'y trouve. */
export function recomputeProtected(level: Level): void {
  const spawns = new Set<string>();
  const add = (c: MapCoord) => {
    if (c.x < TELEPORT_UNSET) spawns.add(`${c.x},${c.y}`);
  };
  add(level.player);
  level.ghosts.forEach(add);
  add(level.pickup);
  level.teleport.forEach(add);
  for (let y = 0; y < MAPY; y++) {
    for (let x = 0; x < MAPX; x++) level.map[y][x]!.isProtected = spawns.has(`${x},${y}`);
  }
}

/** Trace un segment de mur de (ox,oy) vers (x,y), avec auto-connexion (place_line). */
export function placeLine(level: Level, x: number, y: number, ox: number, oy: number): void {
  if (!inBounds(x, y)) return;
  if (!inBounds(ox, oy)) {
    ox = x;
    oy = y;
  }
  const cell = level.map[y][x]!;
  if (cell.isProtected) return;
  if (level.map[oy][ox]!.isProtected) {
    ox = x;
    oy = y;
  }

  let lineTile = cell.tile;
  if (cell.isPill || lineTile === 0) {
    cell.isPill = false;
    cell.isPowerpill = false;
    cell.tile = 1; // point sans connexion
    lineTile = 0;
  } else {
    lineTile--;
  }

  if (oy === y && ox === x) return; // pas de mouvement : simple point

  const connect = (cx: number, cy: number, bitNew: number, bitOld: number): void => {
    let n = lineTile | bitNew;
    level.map[y][x]!.tile = n + 1;
    let o = level.map[cy][cx]!.tile;
    if (o) o--;
    o |= bitOld;
    level.map[cy][cx]!.tile = o + 1;
  };

  if (oy === y) {
    if (x - ox === 1) connect(ox, oy, 8, 2); // déplacement vers la droite
    else if (x - ox === -1) connect(ox, oy, 2, 8); // vers la gauche
  } else if (ox === x) {
    if (y - oy === 1) connect(ox, oy, 1, 4); // vers le bas
    else if (y - oy === -1) connect(ox, oy, 4, 1); // vers le haut
  }
}

/** Efface une case (mur, pill ou spawn) et sévère les connexions voisines (remove_tile). */
export function removeTile(level: Level, x: number, y: number): void {
  if (!inBounds(x, y)) return;

  // Retire un éventuel spawn sur la case.
  if (level.player.x === x && level.player.y === y) level.player = { x: TELEPORT_UNSET, y: TELEPORT_UNSET };
  level.ghosts.forEach((g, i) => {
    if (g.x === x && g.y === y) level.ghosts[i] = { x: TELEPORT_UNSET, y: TELEPORT_UNSET };
  });
  level.teleport.forEach((t, i) => {
    if (t.x === x && t.y === y) level.teleport[i] = { x: TELEPORT_UNSET, y: TELEPORT_UNSET };
  });
  if (level.pickup.x === x && level.pickup.y === y) level.pickup = { x: TELEPORT_UNSET, y: TELEPORT_UNSET };

  const cell = level.map[y][x]!;
  if (!cell.isPill && cell.tile) {
    // Sévère le bit de connexion correspondant chez chaque voisin (avec wrap).
    sever(level, x - 1 < 0 ? MAPX - 1 : x - 1, y, 0b1101); // voisin gauche : ôte « droite »
    sever(level, x + 1 >= MAPX ? 0 : x + 1, y, 0b0111); // voisin droit : ôte « gauche »
    sever(level, x, y - 1 < 0 ? MAPY - 1 : y - 1, 0b1011); // voisin haut : ôte « bas »
    sever(level, x, y + 1 >= MAPY ? 0 : y + 1, 0b1110); // voisin bas : ôte « haut »
  }

  cell.tile = 0;
  cell.isPill = false;
  cell.isPowerpill = false;
  cell.isProtected = false;
  recomputeProtected(level);
}

function sever(level: Level, x: number, y: number, mask: number): void {
  const c = level.map[y][x]!;
  if (c.isPill || !c.tile) return;
  c.tile = ((c.tile - 1) & mask) + 1;
}

/** Place une pill (ou powerpill) sur une case non protégée. */
export function placePill(level: Level, x: number, y: number, pillTile: number, power: boolean): void {
  if (!inBounds(x, y)) return;
  const cell = level.map[y][x]!;
  if (cell.isProtected) return;
  cell.tile = pillTile + 1;
  cell.isPill = true;
  cell.isPowerpill = power;
}

/** Place un spawn ; déplace l'ancien et protège la case. */
export function placeSpawn(level: Level, kind: SpawnKind, x: number, y: number): void {
  if (!inBounds(x, y)) return;
  const cell = level.map[y][x]!;
  cell.tile = 0;
  cell.isPill = false;
  cell.isPowerpill = false;
  const at: MapCoord = { x, y };
  switch (kind) {
    case 'player':
      level.player = at;
      break;
    case 'ghost0':
    case 'ghost1':
    case 'ghost2':
    case 'ghost3':
      level.ghosts[Number(kind.slice(5))] = at;
      break;
    case 'pickup':
      level.pickup = at;
      break;
    case 'teleport': {
      // Remplit le premier emplacement libre, sinon décale.
      const i = level.teleport.findIndex((t) => t.x >= TELEPORT_UNSET);
      if (i >= 0) level.teleport[i] = at;
      else level.teleport = [level.teleport[1]!, at];
      break;
    }
  }
  recomputeProtected(level);
}

/** Remplit les cases vides accessibles avec des pills (pill_fill). */
export function pillFill(level: Level, pillTile: number): void {
  let path: number[][] | null = null;
  if (level.player.x < TELEPORT_UNSET) path = getPath(level, level.player);
  for (let y = 0; y < MAPY; y++) {
    for (let x = 0; x < MAPX; x++) {
      const c = level.map[y][x]!;
      if (!c.tile || (c.isPill && !c.isPowerpill)) {
        if (c.isProtected) continue;
        if (path && path[y][x] === -1) continue; // isolé
        c.isPill = true;
        c.isPowerpill = false;
        c.tile = pillTile + 1;
      }
    }
  }
}

/** Compte les pills et met à jour level.pills. */
export function syncPills(level: Level): number {
  let n = 0;
  for (let y = 0; y < MAPY; y++) for (let x = 0; x < MAPX; x++) if (level.map[y][x]!.isPill) n++;
  level.pills = n;
  return n;
}

export interface ValidationResult {
  valid: boolean;
  message: string;
  errors: boolean[][]; // [MAPY][MAPX]
}

function emptyErrors(): boolean[][] {
  return Array.from({ length: MAPY }, () => new Array<boolean>(MAPX).fill(false));
}

/** Valide le niveau (transposé de validate dans p2_edit.c). */
export function validate(level: Level): ValidationResult {
  const errors = emptyErrors();
  syncPills(level);

  if (level.player.x >= TELEPORT_UNSET) return { valid: false, message: 'Pacman manquant', errors };

  const path = getPath(level, level.player);

  // Pills toutes accessibles ?
  let isolatedPills = 0;
  let teleports = 0;
  for (let y = 0; y < MAPY; y++) {
    for (let x = 0; x < MAPX; x++) {
      if (path[y][x] === -1 && level.map[y][x]!.isPill) {
        errors[y][x] = true;
        isolatedPills++;
      }
      if (level.teleport[0]!.x === x && level.teleport[0]!.y === y) teleports++;
      if (level.teleport[1]!.x === x && level.teleport[1]!.y === y) teleports++;
    }
  }
  if (isolatedPills) return { valid: false, message: isolatedPills > 1 ? 'Pills isolées' : 'Pill isolée', errors };

  if (teleports === 1) return { valid: false, message: 'Téléport manquant (il en faut 0 ou 2)', errors };

  for (const t of level.teleport) {
    if (t.x < TELEPORT_UNSET && path[t.y][t.x] === -1) {
      errors[t.y][t.x] = true;
      return { valid: false, message: 'Téléport isolé', errors };
    }
  }

  // Bords : une case franchissable doit avoir son vis-à-vis (wrap).
  let wrapErr = 0;
  const movable = (v: number) => v >= 0 && v < 999;
  for (let y = 0; y < MAPY; y++) {
    for (let x = 0; x < MAPX; x++) {
      if ((x === 0 || x === MAPX - 1) && movable(path[y][x]!) && !movable(path[y][MAPX - 1 - x]!)) {
        errors[y][x] = errors[y][MAPX - 1 - x] = true;
        wrapErr++;
      }
      if ((y === 0 || y === MAPY - 1) && movable(path[y][x]!) && !movable(path[MAPY - 1 - y][x]!)) {
        errors[y][x] = errors[MAPY - 1 - y][x] = true;
        wrapErr++;
      }
    }
  }
  if (wrapErr) return { valid: false, message: wrapErr > 1 ? 'Erreurs de bord (wrap)' : 'Erreur de bord (wrap)', errors };

  if (level.pickup.x >= TELEPORT_UNSET) return { valid: false, message: 'Spawn pickup manquant', errors };
  if (path[level.pickup.y][level.pickup.x] === -1) {
    errors[level.pickup.y][level.pickup.x] = true;
    return { valid: false, message: 'Spawn pickup isolé', errors };
  }

  const colours = ['rouge', 'vert', 'cyan', 'violet'];
  for (let i = 0; i < 4; i++) {
    const g = level.ghosts[i]!;
    if (g.x >= TELEPORT_UNSET) return { valid: false, message: `Fantôme ${colours[i]} manquant`, errors };
    if (path[g.y][g.x] === -1) {
      errors[g.y][g.x] = true;
      return { valid: false, message: `Fantôme ${colours[i]} isolé`, errors };
    }
    if (Math.abs(level.player.x - g.x) <= 1 && Math.abs(level.player.y - g.y) <= 1) {
      errors[g.y][g.x] = true;
      return { valid: false, message: `Fantôme ${colours[i]} trop près de Pacman`, errors };
    }
  }

  if (level.pills === 0) return { valid: false, message: 'Aucune pill', errors };

  level.validated = true;
  return { valid: true, message: 'Niveau valide ✓', errors };
}
