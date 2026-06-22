import { describe, it, expect } from 'vitest';
import { decodeLevel, encodeLevel } from '../src/core/level';
import { MAPX, MAPY, MAP_ID, MAP_VER } from '../src/core/constants';
import type { Level, Tile } from '../src/core/types';

function makeLevel(): Level {
  const map: Tile[][] = [];
  for (let y = 0; y < MAPY; y++) {
    const row: Tile[] = [];
    for (let x = 0; x < MAPX; x++) {
      row.push({
        tile: (x + y) & 0xff,
        isPill: (x + y) % 3 === 0,
        isPowerpill: (x + y) % 7 === 0,
        isProtected: x === 0 || y === 0,
      });
    }
    map.push(row);
  }
  return {
    mapId: MAP_ID,
    mapVer: MAP_VER,
    validated: true,
    lineSet: 4,
    player: { x: 11, y: 8 },
    ghosts: [
      { x: 1, y: 1 },
      { x: 21, y: 1 },
      { x: 1, y: 15 },
      { x: 21, y: 15 },
    ],
    pickup: { x: 11, y: 4 },
    teleport: [
      { x: 0, y: 8 },
      { x: 22, y: 8 },
    ],
    background: 2,
    map,
    pills: 137,
  };
}

describe('format de niveau .dp2', () => {
  it('round-trip encode → decode conserve toutes les données', () => {
    const level = makeLevel();
    const decoded = decodeLevel(encodeLevel(level));
    expect(decoded).toEqual(level);
  });

  it('produit exactement 1591 octets pour un niveau v3', () => {
    const expected = MAP_ID.length + 1 + 1 + 1 + 2 + 8 + 2 + 4 + 1 + MAPY * MAPX * 4 + 2;
    expect(encodeLevel(makeLevel()).byteLength).toBe(expected);
    expect(expected).toBe(1591);
  });

  it('rejette un identifiant de niveau invalide', () => {
    const buf = encodeLevel(makeLevel());
    buf[0] = 'X'.charCodeAt(0);
    expect(() => decodeLevel(buf)).toThrow(/map id/i);
  });

  it('rejette une version de niveau trop récente', () => {
    const buf = encodeLevel(makeLevel());
    buf[MAP_ID.length] = MAP_VER + 1; // octet de version
    expect(() => decodeLevel(buf)).toThrow(/version/i);
  });
});
