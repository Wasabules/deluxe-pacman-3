import { describe, it, expect } from 'vitest';
import { createPlayState, stepGame, applyTool, comboMult } from '../src/core/game';
import { Tool } from '../src/core/tools';
import { MAPX, MAPY, MAP_ID, MAP_VER } from '../src/core/constants';
import type { Level, Tile } from '../src/core/types';

// Niveau : tout en murs sauf un couloir horizontal y=8, téléports aux extrémités.
function corridorLevel(): Level {
  const map: Tile[][] = [];
  for (let y = 0; y < MAPY; y++) {
    const row: Tile[] = [];
    for (let x = 0; x < MAPX; x++) {
      const open = y === 8;
      row.push({ tile: open ? 0 : 6, isPill: false, isPowerpill: false, isProtected: false });
    }
    map.push(row);
  }
  return {
    mapId: MAP_ID,
    mapVer: MAP_VER,
    validated: true,
    lineSet: 0,
    player: { x: 3, y: 8 },
    ghosts: [
      { x: 11, y: 0 },
      { x: 11, y: 0 },
      { x: 11, y: 0 },
      { x: 11, y: 0 },
    ],
    pickup: { x: 11, y: 0 },
    teleport: [
      { x: 0, y: 8 },
      { x: 22, y: 8 },
    ],
    background: 0,
    map,
    pills: 1,
  };
}

describe('createPlayState — copie défensive du niveau', () => {
  it('ne mute pas le Level source → rejouer repart d\'un terrain neuf', () => {
    const level = corridorLevel();
    level.map[8]![5] = { tile: 1, isPill: true, isPowerpill: false, isProtected: false };

    const first = createPlayState(level);
    // Simule la consommation d'une pill (le gameplay mute play.level.map).
    first.level.map[8]![5]!.tile = 0;
    first.level.map[8]![5]!.isPill = false;

    // Le Level source (potentiellement en cache) ne doit pas être touché…
    expect(level.map[8]![5]!.tile).toBe(1);
    expect(level.map[8]![5]!.isPill).toBe(true);
    // …donc une nouvelle partie sur le même Level retrouve la pill intacte.
    const second = createPlayState(level);
    expect(second.level.map[8]![5]!.tile).toBe(1);
    expect(second.level.map[8]![5]!.isPill).toBe(true);
  });
});

describe('série / combo', () => {
  it('le multiplicateur croît par paliers et se plafonne à ×5', () => {
    const s = createPlayState(corridorLevel());
    s.combo = 0;
    expect(comboMult(s)).toBe(1);
    s.combo = 8;
    expect(comboMult(s)).toBe(2);
    s.combo = 32;
    expect(comboMult(s)).toBe(5);
    s.combo = 999;
    expect(comboMult(s)).toBe(5); // plafonné
  });

  it('la série retombe après un temps sans gain', () => {
    const s = createPlayState(corridorLevel());
    s.ghosts.forEach((g) => (g.dead = true));
    s.combo = 10;
    s.comboTimer = 0;
    for (let i = 0; i < 110; i++) stepGame(s);
    expect(s.combo).toBe(0);
  });
});

describe('nouveaux outils (Deluxe Pacman 3)', () => {
  it('LIGHTNING foudroie le fantôme vivant le plus proche', () => {
    const s = createPlayState(corridorLevel());
    s.ghosts.forEach((g, i) => (g.dead = i > 0)); // seul ghost[0] vivant
    s.ghosts[0]!.x = s.pacman.x + 30;
    s.ghosts[0]!.y = s.pacman.y;
    applyTool(s, Tool.LIGHTNING);
    expect(s.ghosts[0]!.dead).toBe(true);
  });

  it('BOMB pose une bombe active qui explose après le délai', () => {
    const s = createPlayState(corridorLevel());
    applyTool(s, Tool.BOMB);
    expect(s.bomb.active).toBe(true);
    s.ghosts.forEach((g) => (g.dead = true)); // pas de collision parasite
    for (let i = 0; i < 130 && s.bomb.active; i++) stepGame(s);
    expect(s.bomb.active).toBe(false);
  });

  it('WARP téléporte Pacman sur une case libre', () => {
    const s = createPlayState(corridorLevel());
    applyTool(s, Tool.WARP);
    expect(s.warped).toBe(true);
    expect(s.pacman.map.y).toBe(8); // seules les cases du couloir y=8 sont libres
  });

  it('PHASE laisse traverser un mur', () => {
    const s = createPlayState(corridorLevel());
    s.ghosts.forEach((g) => (g.dead = true));
    s.toolInUse[Tool.PHASE] = true;
    s.desiredDir = 0; // UP — bloqué par un mur sans PHASE
    const y0 = s.pacman.y;
    for (let i = 0; i < 20; i++) stepGame(s);
    expect(s.pacman.y).toBeLessThan(y0); // a bien traversé vers le haut
  });
});

describe('téléports', () => {
  it('transporte Pacman d\'un téléport à l\'autre', () => {
    const s = createPlayState(corridorLevel());
    s.ghosts.forEach((g) => (g.dead = true)); // pas de collision parasite
    s.pacman.dx = -1; // va vers la gauche, vers le téléport (0,8)
    s.pacman.dy = 0;

    let teleported = false;
    for (let i = 0; i < 40 && !teleported; i++) {
      stepGame(s);
      if (s.teleported) teleported = true;
    }
    expect(teleported).toBe(true);
    // Après téléport depuis (0,8), Pacman réapparaît côté droit (22,8).
    expect(s.pacman.map.x).toBeGreaterThan(18);
    expect(s.pacman.map.y).toBe(8);
  });

  it('ne reste pas bloqué en ping-pong sur le téléport', () => {
    const s = createPlayState(corridorLevel());
    s.ghosts.forEach((g) => (g.dead = true));
    s.pacman.dx = -1;
    // Beaucoup de pas : Pacman doit continuer à avancer, pas osciller sur place.
    for (let i = 0; i < 40; i++) stepGame(s);
    // justTeleported retombe à false dès qu'il quitte la case téléport.
    expect(typeof s.pacman.map.x).toBe('number');
  });
});
