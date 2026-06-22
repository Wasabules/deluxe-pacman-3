// Construit le décor d'un niveau pour le rendu 2.5D, en deux parties :
//  - buildGround : le « sol » (fond tuilé, spawns, téléports, ombres des murs).
//    Toujours dessiné sous tout le reste.
//  - buildWalls  : les murs extrudés (relief), renvoyés comme une liste de sprites
//    portant un `zIndex` = profondeur (Y de la base). Ils sont mélangés aux
//    entités dans un calque trié, pour que les murs proches recouvrent Pacman.

import { Container, Sprite, type Texture } from 'pixi.js';
import { MAPX, MAPY, TILE_SIZE, TELEPORT_UNSET } from '../../core/constants';
import type { Level } from '../../core/types';
import type { PixiTextures } from './pixiTextures';

const GHOST_TINT = [0xff8080, 0x80ff80, 0x80ffff, 0xff80ff];
const PICKUP_TINT = 0xbfbfbf;

const WALL_HEIGHT = 12; // hauteur d'extrusion (px) — relief 2.5D
const BASE_TINT = 0x14163a; // teinte de la base du mur (paroi très sombre)
const TOP_TINT = 0xffffff; // teinte du sommet (couleur normale)

/** Profondeur (Y au sol) d'une cellule, pour le tri 2.5D. */
export function depthOfCell(y: number): number {
  return (y + 1) * TILE_SIZE;
}

function lerpColor(a: number, b: number, t: number): number {
  const ar = (a >> 16) & 0xff, ag = (a >> 8) & 0xff, ab = a & 0xff;
  const br = (b >> 16) & 0xff, bg = (b >> 8) & 0xff, bb = b & 0xff;
  return (Math.round(ar + (br - ar) * t) << 16) | (Math.round(ag + (bg - ag) * t) << 8) | Math.round(ab + (bb - ab) * t);
}

const isWall = (level: Level, x: number, y: number): boolean => {
  const c = level.map[y]?.[x];
  return !!c && !c.isPill && c.tile > 0;
};

const lineTex = (textures: PixiTextures, level: Level, x: number, y: number): Texture => {
  const ls = textures.linesets[level.lineSet] ?? textures.linesets[0]!;
  return ls[level.map[y][x]!.tile - 1]!;
};

/** Calque « sol » : fond, spawns, téléports, ombres des murs (forme réelle). */
export function buildGround(textures: PixiTextures, level: Level, shadows: boolean): Container {
  const root = new Container();

  const bgTex = textures.backgrounds[level.background] ?? textures.backgrounds[0]!;
  for (let y = 0; y < MAPY; y++) {
    for (let x = 0; x < MAPX; x++) {
      const s = new Sprite(bgTex);
      s.position.set(x * TILE_SIZE, y * TILE_SIZE);
      root.addChild(s);
    }
  }

  for (let i = 0; i < 4; i++) {
    const g = level.ghosts[i]!;
    const s = new Sprite(textures.ghostSpawn);
    s.position.set(g.x * TILE_SIZE, g.y * TILE_SIZE);
    s.tint = GHOST_TINT[i]!;
    root.addChild(s);
  }
  const ps = new Sprite(textures.pickupSpawn);
  ps.position.set(level.pickup.x * TILE_SIZE, level.pickup.y * TILE_SIZE);
  ps.tint = PICKUP_TINT;
  root.addChild(ps);

  for (const t of level.teleport) {
    if (t.x < TELEPORT_UNSET) {
      const s = new Sprite(textures.teleport);
      s.position.set(t.x * TILE_SIZE, t.y * TILE_SIZE);
      root.addChild(s);
    }
  }

  // Ombres des murs : copie du sprite de mur teintée en noir, décalée.
  // Regroupées dans un container à alpha global → pas d'accumulation aux recouvrements.
  if (shadows) {
    const shadowLayer = new Container();
    for (let y = 0; y < MAPY; y++) {
      for (let x = 0; x < MAPX; x++) {
        if (!isWall(level, x, y)) continue;
        const s = new Sprite(lineTex(textures, level, x, y));
        s.position.set(x * TILE_SIZE + 4, y * TILE_SIZE + 6);
        s.tint = 0x000000;
        shadowLayer.addChild(s);
      }
    }
    shadowLayer.alpha = 0.3;
    root.addChild(shadowLayer);
  }

  return root;
}

/** Murs extrudés : liste de sprites avec `zIndex` = profondeur, à insérer dans le calque trié. */
export function buildWalls(textures: PixiTextures, level: Level): Sprite[] {
  const out: Sprite[] = [];
  for (let y = 0; y < MAPY; y++) {
    for (let x = 0; x < MAPX; x++) {
      if (!isWall(level, x, y)) continue;
      const tex = lineTex(textures, level, x, y);
      const z = depthOfCell(y);
      for (let k = 0; k <= WALL_HEIGHT; k++) {
        const s = new Sprite(tex);
        s.position.set(x * TILE_SIZE, y * TILE_SIZE - k);
        s.tint = lerpColor(BASE_TINT, TOP_TINT, k / WALL_HEIGHT);
        // Profondeur de la cellule + micro-décalage par couche : garantit l'ordre
        // base→sommet (k croissant = plus haut = dessus), tout en restant trié
        // avec les entités par la profondeur entière de la cellule.
        s.zIndex = z + k * 0.01;
        out.push(s);
      }
    }
  }
  return out;
}
