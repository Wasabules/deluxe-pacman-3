import { describe, it, expect } from 'vitest';
import { getPath, dir } from '../src/core/pathfinding';
import { MAPX, MAPY, MAP_ID, MAP_VER } from '../src/core/constants';
import type { Level, Tile } from '../src/core/types';

// Construit un niveau depuis un schéma ASCII (lignes de MAPX caractères, MAPY lignes).
//   '#' = mur, ' ' ou '.' = traversable.
function levelFromAscii(rows: string[]): Level {
  const map: Tile[][] = [];
  for (let y = 0; y < MAPY; y++) {
    const row: Tile[] = [];
    for (let x = 0; x < MAPX; x++) {
      const ch = rows[y]?.[x] ?? ' ';
      const wall = ch === '#';
      row.push({ tile: wall ? 6 : 0, isPill: false, isPowerpill: false, isProtected: false });
    }
    map.push(row);
  }
  return {
    mapId: MAP_ID,
    mapVer: MAP_VER,
    validated: true,
    lineSet: 0,
    player: { x: 1, y: 1 },
    ghosts: [
      { x: 1, y: 1 },
      { x: 1, y: 1 },
      { x: 1, y: 1 },
      { x: 1, y: 1 },
    ],
    pickup: { x: 1, y: 1 },
    teleport: [
      { x: 99, y: 99 },
      { x: 99, y: 99 },
    ],
    background: 0,
    map,
    pills: 0,
  };
}

const seq = (vals: number[]): (() => number) => {
  let i = 0;
  return () => vals[i++ % vals.length]!;
};

describe('getPath (flood-fill BFS)', () => {
  it('attribue des distances croissantes depuis la destination', () => {
    const level = levelFromAscii([]); // tout ouvert
    const grid = getPath(level, { x: 5, y: 5 });
    expect(grid[5][5]).toBe(0);
    expect(grid[5][6]).toBe(1);
    expect(grid[5][7]).toBe(2);
    expect(grid[7][5]).toBe(2);
    // distance de Manhattan en espace ouvert
    expect(grid[8][9]).toBe(Math.abs(8 - 5) + Math.abs(9 - 5));
  });

  it('marque les murs à 999 et les contourne', () => {
    // un mur vertical en x=3 (sauf une ouverture) force le détour
    const rows: string[] = [];
    for (let y = 0; y < MAPY; y++) rows.push(y === 0 ? '   '.padEnd(MAPX) : '###'.padEnd(MAPX, ' ').slice(0, 3).padEnd(MAPX));
    // construit : colonne x=2 pleine de murs sauf à y=0
    const walls = Array.from({ length: MAPY }, (_, y) => (y === 0 ? ' ' : '#'));
    const grid2rows = Array.from({ length: MAPY }, (_, y) => {
      let r = '';
      for (let x = 0; x < MAPX; x++) r += x === 2 ? walls[y] : ' ';
      return r;
    });
    void rows;
    const level = levelFromAscii(grid2rows);
    const grid = getPath(level, { x: 0, y: 5 });
    expect(grid[5][2]).toBe(999); // mur
    // x=4,y=5 est de l'autre côté : doit passer par l'ouverture en y=0 → long détour
    expect(grid[5][4]).toBeGreaterThan(5);
  });

  it('utilise le wrap des bords pour raccourcir', () => {
    const level = levelFromAscii([]); // tout ouvert
    const grid = getPath(level, { x: 0, y: 5 });
    // la case x=MAPX-1 est à 1 pas via le wrap, pas MAPX-1 pas
    expect(grid[5][MAPX - 1]).toBe(1);
  });

  it('signale une destination sur un mur (valeur 1)', () => {
    const rows = Array.from({ length: MAPY }, (_, y) => (y === 3 ? '   #'.padEnd(MAPX) : ' '.repeat(MAPX)));
    const level = levelFromAscii(rows);
    const grid = getPath(level, { x: 3, y: 3 });
    expect(grid[3][3]).toBe(1);
  });
});

describe('dir (choix de direction)', () => {
  it('suit le plus court chemin vers la cible en espace ouvert', () => {
    const level = levelFromAscii([]);
    const ghost = { map: { x: 10, y: 5 }, dx: -1, dy: 0 }; // va vers la gauche
    // cible à gauche → doit continuer à gauche (1)
    const d = dir(level, ghost, { x: 2, y: 5 }, false, seq([0]));
    expect(d).toBe(1);
  });

  it("dans un couloir, ne fait pas demi-tour quand une seule issue existe", () => {
    // couloir horizontal sur la ligne y=5, murs autour
    const rows = Array.from({ length: MAPY }, (_, y) => (y === 5 ? ' '.repeat(MAPX) : '#'.repeat(MAPX)));
    const level = levelFromAscii(rows);
    const ghost = { map: { x: 5, y: 5 }, dx: 1, dy: 0 }; // se déplace vers la droite
    // seule issue = continuer à droite (2), même si la cible est derrière
    const d = dir(level, ghost, { x: 1, y: 5 }, false, seq([0]));
    expect(d).toBe(2);
  });
});
