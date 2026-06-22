import { describe, it, expect, beforeEach } from 'vitest';
import { createPlayState, applyTool, pillPoints, type PlayState } from '../src/core/game';
import { Tool } from '../src/core/tools';
import { MAPX, MAPY, MAP_ID, MAP_VER } from '../src/core/constants';
import type { Level, Tile } from '../src/core/types';

function emptyLevel(): Level {
  const map: Tile[][] = [];
  for (let y = 0; y < MAPY; y++) {
    const row: Tile[] = [];
    for (let x = 0; x < MAPX; x++) row.push({ tile: 0, isPill: false, isPowerpill: false, isProtected: false });
    map.push(row);
  }
  return {
    mapId: MAP_ID,
    mapVer: MAP_VER,
    validated: true,
    lineSet: 0,
    player: { x: 11, y: 8 },
    ghosts: [
      { x: 1, y: 1 },
      { x: 21, y: 1 },
      { x: 1, y: 15 },
      { x: 21, y: 15 },
    ],
    pickup: { x: 11, y: 4 },
    teleport: [
      { x: 99, y: 99 },
      { x: 99, y: 99 },
    ],
    background: 0,
    map,
    pills: 50,
  };
}

const fixedRng = (v: number) => () => v;

describe('outils — effets de score instantanés', () => {
  let s: PlayState;
  beforeEach(() => {
    s = createPlayState(emptyLevel());
  });

  it('AUTISM/PRECIOUS/GOLDLEAF donnent des points fixes', () => {
    applyTool(s, Tool.AUTISM);
    expect(s.score).toBe(30000);
    applyTool(s, Tool.PRECIOUS);
    expect(s.score).toBe(30000 + 50000);
    applyTool(s, Tool.GOLDLEAF);
    expect(s.score).toBe(30000 + 50000 + 40000);
  });

  it('DYNAMITE tue tous les fantômes et rapporte 5000 chacun', () => {
    applyTool(s, Tool.DYNAMITE);
    expect(s.ghosts.every((g) => g.dead)).toBe(true);
    expect(s.score).toBe(4 * 5000);
  });

  it('FREEZE gèle les fantômes vivants', () => {
    applyTool(s, Tool.FREEZE);
    expect(s.ghosts.every((g) => g.frozen)).toBe(true);
    expect(s.toolInUse[Tool.FREEZE]).toBe(true);
  });
});

describe('outils — lettres EXTRA', () => {
  it('réunir E+X+T+R+A donne une vie et remet les lettres à zéro', () => {
    const s = createPlayState(emptyLevel());
    const start = s.lives;
    for (const t of [Tool.EXTRA_E, Tool.EXTRA_X, Tool.EXTRA_T, Tool.EXTRA_R]) applyTool(s, t);
    expect(s.lives).toBe(start); // pas encore complet
    expect(s.extra).toBe(0b1111);
    applyTool(s, Tool.EXTRA_A);
    expect(s.lives).toBe(start + 1);
    expect(s.extra).toBe(0);
    expect(s.gotExtraLife).toBe(true);
  });
});

describe('outils — multiplicateurs et diamants sur les pills', () => {
  it('une pill vaut 100 par défaut', () => {
    const s = createPlayState(emptyLevel());
    expect(pillPoints(s)).toBe(100);
  });

  it('TIMES2/5/7 multiplient la valeur', () => {
    const s = createPlayState(emptyLevel());
    s.toolInUse[Tool.TIMES5] = true;
    expect(pillPoints(s)).toBe(500);
  });

  it('les diamants fixent la valeur (avant multiplicateur)', () => {
    const s = createPlayState(emptyLevel());
    s.toolInUse[Tool.BLUE_DIAMOND] = true;
    expect(pillPoints(s)).toBe(1000);
    s.toolInUse[Tool.BLUE_DIAMOND] = false;
    s.toolInUse[Tool.PINK_DIAMOND] = true;
    expect(pillPoints(s)).toBe(2000);
  });

  it('PRESENT donne entre 1000 et 5000 (rng contrôlé)', () => {
    const s = createPlayState(emptyLevel(), { rng: fixedRng(0) }); // rng=0 → minimum
    s.toolInUse[Tool.PRESENT] = true;
    expect(pillPoints(s)).toBe(1000);
  });
});

describe('outils — SPEED/GLUE arment un effet persistant', () => {
  it('SPEED active toolInUse et le minuteur', () => {
    const s = createPlayState(emptyLevel());
    applyTool(s, Tool.SPEED);
    expect(s.toolInUse[Tool.SPEED]).toBe(true);
    expect(s.toolTimer).toBeGreaterThan(0);
  });
});
