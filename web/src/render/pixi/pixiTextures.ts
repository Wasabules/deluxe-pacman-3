// Conversion des spritesheets (images déjà chargées) en textures PixiJS découpées.
// Réutilise les images de loadGameSprites — pas de rechargement.

import { Texture, Rectangle, type TextureSource } from 'pixi.js';
import { TILE_SIZE, SPRITE_SIZE } from '../../core/constants';
import { DRAWN_TOOLS, drawToolIcon } from '../toolIcons';
import type { GameSprites } from '../../platform/assets';

/** Textures des outils dessinés (sans case dans la planche), dans l'ordre des enums. */
function drawnToolTextures(): Texture[] {
  return DRAWN_TOOLS.map((tool) => {
    const canvas = document.createElement('canvas');
    canvas.width = SPRITE_SIZE;
    canvas.height = SPRITE_SIZE;
    const ctx = canvas.getContext('2d')!;
    drawToolIcon(ctx, tool, 0, 0, SPRITE_SIZE);
    const t = Texture.from(canvas);
    t.source.scaleMode = 'nearest';
    return t;
  });
}

/** Découpe une source en `count` tuiles (grille de `cols` colonnes). */
function slice(source: TextureSource, tileW: number, tileH: number, cols: number, count: number): Texture[] {
  const out: Texture[] = [];
  for (let i = 0; i < count; i++) {
    const sx = (i % cols) * tileW;
    const sy = Math.floor(i / cols) * tileH;
    out.push(new Texture({ source, frame: new Rectangle(sx, sy, tileW, tileH) }));
  }
  return out;
}

function sourceOf(img: CanvasImageSource): TextureSource {
  const tex = Texture.from(img as HTMLImageElement);
  tex.source.scaleMode = 'nearest'; // rendu pixelisé, pas de lissage
  return tex.source;
}

export interface PixiTextures {
  backgrounds: Texture[]; // 20
  linesets: Texture[][]; // [set][0..15]
  pills: Texture[]; // 40 (5x8)
  ghostSpawn: Texture;
  pickupSpawn: Texture;
  teleport: Texture;
  pacman: Texture[]; // 25 (5x5)
  ghosts: Texture[]; // 20 (4 rangées x 5)
  ghostTop: Texture[][]; // [couleur][frame] haut (corps+yeux), 50x44
  ghostBottom: Texture[][]; // [couleur][frame] bas animé, 50x5
  ghostEyes: Texture[]; // 4 (rangée 0 de Blue_Eyes)
  blueGhost: Texture[]; // 4 (rangée 1 de Blue_Eyes)
  tools: Texture[]; // 25
  pickups: Texture[]; // 35
  iceCube: Texture;
  shield: Texture;
  bullet: Texture;
}

export function buildPixiTextures(sprites: GameSprites): PixiTextures {
  const bgSrc = sourceOf(sprites.backgrounds.img);
  const pillSrc = sourceOf(sprites.pills.img);
  const pacSrc = sourceOf(sprites.pacman.img);
  const ghostSrc = sourceOf(sprites.ghosts.img);
  const blueSrc = sourceOf(sprites.blueEyes.img);
  const toolSrc = sourceOf(sprites.tools.img);
  const pickupSrc = sourceOf(sprites.pickups.img);

  const linesets = sprites.linesets.map((ls) => slice(sourceOf(ls.img), TILE_SIZE, TILE_SIZE, 16, 16));

  // Composite des fantômes : haut (corps+yeux, 50x44) et bas animé (50x5).
  const ghostTop: Texture[][] = [];
  const ghostBottom: Texture[][] = [];
  for (let i = 0; i < 4; i++) {
    const top: Texture[] = [];
    const bottom: Texture[] = [];
    for (let f = 0; f < 5; f++) {
      top.push(new Texture({ source: ghostSrc, frame: new Rectangle(f * SPRITE_SIZE, i * SPRITE_SIZE, SPRITE_SIZE, 44) }));
      bottom.push(new Texture({ source: ghostSrc, frame: new Rectangle(f * SPRITE_SIZE, i * SPRITE_SIZE + 45, SPRITE_SIZE, 5) }));
    }
    ghostTop.push(top);
    ghostBottom.push(bottom);
  }

  const single = (img: CanvasImageSource): Texture => {
    const t = Texture.from(img as HTMLImageElement);
    t.source.scaleMode = 'nearest';
    return t;
  };

  return {
    backgrounds: slice(bgSrc, TILE_SIZE, TILE_SIZE, 20, 20),
    linesets,
    pills: slice(pillSrc, TILE_SIZE, TILE_SIZE, 8, 40),
    ghostSpawn: single(sprites.ghostSpawn),
    pickupSpawn: single(sprites.pickupSpawn),
    teleport: single(sprites.teleport),
    pacman: slice(pacSrc, SPRITE_SIZE, SPRITE_SIZE, 5, 25),
    ghosts: slice(ghostSrc, SPRITE_SIZE, SPRITE_SIZE, 5, 20),
    ghostTop,
    ghostBottom,
    ghostEyes: slice(blueSrc, SPRITE_SIZE, SPRITE_SIZE, 4, 4),
    blueGhost: slice(blueSrc, SPRITE_SIZE, SPRITE_SIZE, 4, 8).slice(4), // rangée 1
    tools: [...slice(toolSrc, SPRITE_SIZE, SPRITE_SIZE, 5, 25), ...drawnToolTextures()],
    pickups: slice(pickupSrc, SPRITE_SIZE, SPRITE_SIZE, 5, 35),
    iceCube: single(sprites.iceCube),
    shield: single(sprites.shield),
    bullet: single(sprites.bullet),
  };
}
