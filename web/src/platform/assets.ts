// Chargement et découpage des ressources graphiques.
// Adapter navigateur : le `core` ne dépend jamais de ce module.

import { TILE_SIZE, SPRITE_SIZE } from '../core/constants';

const GFX = 'Graphics/';

export function loadImage(url: string): Promise<HTMLImageElement> {
  return new Promise((resolve, reject) => {
    const img = new Image();
    img.onload = () => resolve(img);
    img.onerror = () => reject(new Error(`Échec du chargement de l'image : ${url}`));
    img.src = url;
  });
}

async function tryLoadImage(url: string): Promise<HTMLImageElement | null> {
  try {
    return await loadImage(url);
  } catch {
    return null;
  }
}

/** Feuille de sprites découpée en tuiles de taille fixe disposées en grille. */
export class Sheet {
  constructor(
    readonly img: CanvasImageSource,
    readonly tileW: number,
    readonly tileH: number,
    readonly cols: number,
  ) {}

  /** Dessine la sous-tuile `index` (parcours gauche→droite, haut→bas) en (dx, dy). */
  draw(ctx: CanvasRenderingContext2D, index: number, dx: number, dy: number): void {
    const sx = (index % this.cols) * this.tileW;
    const sy = Math.floor(index / this.cols) * this.tileH;
    ctx.drawImage(this.img, sx, sy, this.tileW, this.tileH, dx, dy, this.tileW, this.tileH);
  }
}


/** Toutes les ressources graphiques découpées, prêtes à dessiner. */
export interface GameSprites {
  backgrounds: Sheet; // 20 tuiles 32x32
  pills: Sheet; // 5x8 tuiles 32x32
  linesets: Sheet[]; // un Sheet par jeu de lignes (16 tuiles 32x32 chacun)
  ghostSpawn: HTMLImageElement;
  pickupSpawn: HTMLImageElement;
  teleport: HTMLImageElement;
  pacman: Sheet; // 5x5 frames 50x50
  ghosts: Sheet; // 4x5 frames 50x50 (1 rangée par couleur)
  blueEyes: Sheet; // 2x4 frames 50x50 (rangée 0 = yeux, rangée 1 = fantôme bleu)
  tools: Sheet; // 5x5 frames 50x50
  pickups: Sheet; // 7x5 frames 50x50
  iceCube: HTMLImageElement;
  shield: HTMLImageElement;
  bullet: HTMLImageElement;
  heartRed: HTMLImageElement;
  heartBlue: HTMLImageElement;
}

export async function loadGameSprites(): Promise<GameSprites> {
  const [
    backgroundsImg,
    pillsImg,
    ghostSpawn,
    pickupSpawn,
    teleport,
    pacmanImg,
    ghostsImg,
    blueEyesImg,
    toolsImg,
    pickupsImg,
    iceCube,
    shield,
    bullet,
    heartRed,
    heartBlue,
  ] = await Promise.all([
    loadImage(GFX + 'Backgrounds.png'),
    loadImage(GFX + 'Pill_Sheet.png'),
    loadImage(GFX + 'Ghost_Spawn.png'),
    loadImage(GFX + 'Pickup_Spawn.png'),
    loadImage(GFX + 'Teleport.png'),
    loadImage(GFX + 'Pacman.png'),
    loadImage(GFX + 'Ghosts.png'),
    loadImage(GFX + 'Blue_Eyes.png'),
    loadImage(GFX + 'Tools.png'),
    loadImage(GFX + 'Pickups.png'),
    loadImage(GFX + 'Ice_Cube.png'),
    loadImage(GFX + 'Shield.png'),
    loadImage(GFX + 'Bullet.png'),
    loadImage(GFX + 'Heart_Red.png'),
    loadImage(GFX + 'Heart_Blue.png'),
  ]);

  // Jeux de lignes : Line00.png, Line01.png, … jusqu'au premier manquant (comme le C).
  const linesets: Sheet[] = [];
  for (let i = 0; i < 100; i++) {
    const n = i.toString().padStart(2, '0');
    const img = await tryLoadImage(`${GFX}Lines/Line${n}.png`);
    if (!img) break;
    linesets.push(new Sheet(img, TILE_SIZE, TILE_SIZE, 16));
  }

  return {
    backgrounds: new Sheet(backgroundsImg, TILE_SIZE, TILE_SIZE, 20),
    pills: new Sheet(pillsImg, TILE_SIZE, TILE_SIZE, 8),
    linesets,
    ghostSpawn,
    pickupSpawn,
    teleport,
    pacman: new Sheet(pacmanImg, SPRITE_SIZE, SPRITE_SIZE, 5),
    ghosts: new Sheet(ghostsImg, SPRITE_SIZE, SPRITE_SIZE, 5),
    blueEyes: new Sheet(blueEyesImg, SPRITE_SIZE, SPRITE_SIZE, 4),
    tools: new Sheet(toolsImg, SPRITE_SIZE, SPRITE_SIZE, 5),
    pickups: new Sheet(pickupsImg, SPRITE_SIZE, SPRITE_SIZE, 5),
    iceCube,
    shield,
    bullet,
    heartRed,
    heartBlue,
  };
}
