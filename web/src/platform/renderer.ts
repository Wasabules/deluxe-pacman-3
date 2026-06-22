// Adapter de rendu de l'overlay 2D (HUD + écrans hors-jeu), superposé au canvas
// WebGL (PixiJS) qui rend le jeu. Transparent en jeu (laisse voir le WebGL),
// opaque sur les écrans hors-jeu.

import { SCREEN_W, HEIGHT } from '../core/constants';

export class Renderer {
  readonly ctx: CanvasRenderingContext2D;

  constructor(uiCanvas: HTMLCanvasElement) {
    uiCanvas.width = SCREEN_W;
    uiCanvas.height = HEIGHT;
    const ctx = uiCanvas.getContext('2d'); // alpha:true (transparent)
    if (!ctx) throw new Error('Contexte Canvas 2D indisponible.');
    this.ctx = ctx;
    this.ctx.imageSmoothingEnabled = false;
  }

  /** Efface l'overlay (transparent) — laisse voir le jeu WebGL derrière. */
  clear(): void {
    this.ctx.clearRect(0, 0, SCREEN_W, HEIGHT);
  }

  /** Remplit l'overlay d'une couleur opaque (écrans hors-jeu). */
  fill(color = '#000'): void {
    this.ctx.fillStyle = color;
    this.ctx.fillRect(0, 0, SCREEN_W, HEIGHT);
  }
}
