// Boucle de jeu à pas de temps FIXE (fixed timestep).
//
// La simulation avance par pas constants de 1/hz seconde, indépendamment du
// taux de rafraîchissement de l'écran. C'est ce qui garantit un gameplay
// déterministe et « à l'identique » par rapport aux timers du jeu original.

export type UpdateFn = (dt: number) => void;
export type RenderFn = (alpha: number, fps: number) => void;

export class GameLoop {
  private readonly dt: number;
  private last = 0;
  private acc = 0;
  private running = false;

  private fps = 0;
  private fpsAcc = 0;
  private fpsFrames = 0;

  constructor(
    hz: number,
    private readonly update: UpdateFn,
    private readonly render: RenderFn,
  ) {
    this.dt = 1 / hz;
  }

  start(): void {
    if (this.running) return;
    this.running = true;
    this.last = performance.now();
    requestAnimationFrame(this.frame);
  }

  stop(): void {
    this.running = false;
  }

  private readonly frame = (now: number): void => {
    if (!this.running) return;

    let frameTime = (now - this.last) / 1000;
    this.last = now;
    // Garde-fou anti « spirale de la mort » si l'onglet a été en arrière-plan.
    if (frameTime > 0.25) frameTime = 0.25;

    // Un échec ponctuel d'un step ou d'un frame (ex. perte/restauration du
    // contexte WebGL) ne doit JAMAIS arrêter la boucle : on l'absorbe et on
    // re-planifie toujours la frame suivante.
    try {
      this.acc += frameTime;
      while (this.acc >= this.dt) {
        this.update(this.dt);
        this.acc -= this.dt;
      }

      this.fpsAcc += frameTime;
      this.fpsFrames++;
      if (this.fpsAcc >= 0.5) {
        this.fps = this.fpsFrames / this.fpsAcc;
        this.fpsAcc = 0;
        this.fpsFrames = 0;
      }

      // alpha = fraction de pas restante, pour interpoler le rendu plus tard.
      const alpha = this.acc / this.dt;
      this.render(alpha, this.fps);
    } catch (e) {
      console.error('Frame ignorée (la boucle continue) :', e);
    }

    requestAnimationFrame(this.frame);
  };
}
