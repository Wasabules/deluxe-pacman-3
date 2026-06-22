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
    // Fantômes au loin : pas de menace → le Pacman va manger (déterministe).
    for (const g of s.ghosts) {
      g.map = { x: 1, y: 1 };
      g.x = TILE_SIZE;
      g.y = TILE_SIZE;
    }
    let guard = 0;
    while (!s.reverseLost && !s.levelComplete && guard++ < 400) stepGame(s);
    expect(s.reverseLost).toBe(true);
    expect(s.levelComplete).toBe(false);
  });
});

describe('mode Reverse — robustesse aux tunnels', () => {
  it('ne plante pas quand le Pacman a une position de grille hors limites (wrap)', () => {
    const level = openLevel();
    level.pills = 1;
    level.map[8]![3] = pill();
    const s = createPlayState(level, { reverse: true });
    for (const g of s.ghosts) {
      g.map = { x: 1, y: 1 }; // fantômes loin → Pacman en mode « manger » (utilise getPath)
      g.x = 32;
      g.y = 32;
    }
    s.pacman.map = { x: MAPX, y: MAPY }; // hors grille, comme juste après un wrap de tunnel
    expect(() => stepGame(s)).not.toThrow();
  });
});

describe('mode Reverse — outils Fantôme', () => {
  // Place l'outil sur la case d'apparition et fait approcher le joueur par la gauche.
  function approachTool(toolId: number) {
    const level = openLevel();
    level.pickup = { x: 15, y: 8 };
    const s = createPlayState(level, { reverse: true });
    s.reverseTool = toolId;
    s.reverseToolTimer = 0;
    s.ghosts[0]!.map = { x: 13, y: 8 };
    s.ghosts[0]!.x = 13 * TILE_SIZE;
    s.ghosts[0]!.y = 8 * TILE_SIZE;
    for (let i = 1; i < 4; i++) {
      s.ghosts[i]!.map = { x: 1, y: 1 }; // alliés à l'écart
      s.ghosts[i]!.x = TILE_SIZE;
      s.ghosts[i]!.y = TILE_SIZE;
    }
    s.desiredDir = 3; // droite → le joueur avance vers la case d'apparition
    let guard = 0;
    while (s.reverseTool !== 0 && guard++ < 120) stepGame(s);
    return s;
  }

  it('Vitesse (1) déclenche un boost de vitesse du joueur', () => {
    expect(approachTool(1).ghostBoost).toBeGreaterThan(0);
  });
  it('Gel (2) immobilise le Pacman', () => {
    expect(approachTool(2).pacmanFrozen).toBeGreaterThan(0);
  });
  it('Radar (3) active le radar', () => {
    expect(approachTool(3).radarTimer).toBeGreaterThan(0);
  });
  it('Téléport (4) rapproche le joueur du Pacman', () => {
    const s = approachTool(4);
    const red = s.ghosts[0]!;
    const p = s.pacman.map;
    expect(Math.abs(red.map.x - p.x) + Math.abs(red.map.y - p.y)).toBeLessThanOrEqual(5);
  });
});

describe('mode Reverse — super-pastille', () => {
  it('une power-pill rend le fantôme-joueur vulnérable', () => {
    const level = openLevel();
    level.pills = 2; // power-pill + une pill normale ailleurs (évite reverseLost immédiat)
    level.map[8]![6] = pill(true); // power-pill proche du Pacman
    level.map[8]![2] = pill(); // pill normale plus loin
    const s = createPlayState(level, { reverse: true });
    // Fantômes au loin : aucun ne mange le Pacman au moment du « frighten »
    // (sinon il redeviendrait non-scared le même frame). Rend le test déterministe.
    for (const g of s.ghosts) {
      g.map = { x: 1, y: 1 };
      g.x = 32;
      g.y = 32;
    }
    let guard = 0;
    while (!s.atePowerpill && guard++ < 400) stepGame(s);
    expect(s.atePowerpill).toBe(true);
    expect(s.ghosts[0]!.scared).toBe(true); // le joueur doit fuir
  });
});
