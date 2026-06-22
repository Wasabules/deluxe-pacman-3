import { describe, it, expect } from 'vitest';
import { createPlayState, stepGame } from '../src/core/game';
import { MAPX, MAPY, MAP_ID, MAP_VER, TILE_SIZE } from '../src/core/constants';
import type { Level, Tile } from '../src/core/types';

// Salle ouverte : bordure en murs, intérieur libre. Pas de pills par défaut.
function openLevel(): Level {
  const map: Tile[][] = [];
  for (let y = 0; y < MAPY; y++) {
    const row: Tile[] = [];
    for (let x = 0; x < MAPX; x++) {
      const wall = x === 0 || y === 0 || x === MAPX - 1 || y === MAPY - 1;
      row.push({ tile: wall ? 6 : 0, isPill: false, isPowerpill: false, isProtected: false });
    }
    map.push(row);
  }
  return {
    mapId: MAP_ID,
    mapVer: MAP_VER,
    validated: true,
    lineSet: 0,
    player: { x: 5, y: 8 },
    ghosts: [
      { x: 10, y: 8 },
      { x: 3, y: 3 },
      { x: 18, y: 3 },
      { x: 18, y: 13 },
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

const pill = (powerpill = false): Tile => ({ tile: 1, isPill: true, isPowerpill: powerpill, isProtected: false });

describe('mode Reverse — initialisation', () => {
  it('démarre avec reverse=true et 3 vies de Pacman', () => {
    const s = createPlayState(openLevel(), { reverse: true });
    expect(s.reverse).toBe(true);
    expect(s.pacmanLives).toBe(3);
    expect(s.reverseLost).toBe(false);
  });
});

describe('mode Reverse — contrôle du fantôme rouge par le joueur', () => {
  it('desiredDir pilote le fantôme rouge (et non Pacman)', () => {
    const s = createPlayState(openLevel(), { reverse: true });
    const red = s.ghosts[0]!;
    const startX = red.x;
    s.desiredDir = 3; // droite
    for (let i = 0; i < 8; i++) stepGame(s);
    expect(red.x).toBeGreaterThan(startX); // le rouge a bien été à droite
  });
});

describe('mode Reverse — capture du Pacman', () => {
  it('une collision décrémente les vies du Pacman', () => {
    const s = createPlayState(openLevel(), { reverse: true });
    s.ghosts[0]!.x = s.pacman.x - TILE_SIZE / 2;
    s.ghosts[0]!.y = s.pacman.y - TILE_SIZE / 2;
    s.ghosts[0]!.scared = false;
    stepGame(s);
    expect(s.capturedPacman).toBe(true);
    expect(s.pacmanLives).toBe(2);
  });

  it('3 captures → niveau gagné (levelComplete)', () => {
    const s = createPlayState(openLevel(), { reverse: true });
    let captures = 0;
    let guard = 0;
    while (!s.levelComplete && guard++ < 3000) {
      s.ghosts[0]!.x = s.pacman.x - TILE_SIZE / 2;
      s.ghosts[0]!.y = s.pacman.y - TILE_SIZE / 2;
      s.ghosts[0]!.scared = false;
      const before = s.pacmanLives;
      stepGame(s);
      if (s.pacmanLives < before) captures++;
    }
    expect(captures).toBe(3);
    expect(s.levelComplete).toBe(true);
    expect(s.pacmanLives).toBe(0);
  });
});

describe('mode Reverse — le Pacman IA mange les pills', () => {
  it('vider le labyrinthe fait perdre le joueur (reverseLost)', () => {
    const level = openLevel();
    level.pills = 1;
    level.map[8]![6] = pill(); // juste à droite du Pacman (5,8)
    const s = createPlayState(level, { reverse: true });
    let guard = 0;
    while (!s.reverseLost && !s.levelComplete && guard++ < 400) stepGame(s);
    expect(s.reverseLost).toBe(true);
    expect(s.levelComplete).toBe(false);
  });
});

describe('mode Reverse — super-pastille', () => {
  it('une power-pill rend le fantôme-joueur vulnérable', () => {
    const level = openLevel();
    level.pills = 2; // power-pill + une pill normale ailleurs (évite reverseLost immédiat)
    level.map[8]![6] = pill(true); // power-pill proche du Pacman
    level.map[8]![2] = pill(); // pill normale plus loin
    const s = createPlayState(level, { reverse: true });
    let guard = 0;
    while (!s.atePowerpill && guard++ < 400) stepGame(s);
    expect(s.atePowerpill).toBe(true);
    expect(s.ghosts[0]!.scared).toBe(true); // le joueur doit fuir
  });
});
