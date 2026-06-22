// Entrée clavier. Mappe les touches vers une direction désirée persistante
// (comme la variable `direction` du C) + l'état des modificateurs (CTRL, espace).

import type { Dir } from '../core/entities';

// e.key (minuscule) → direction. Flèches, WASD et pavé numérique.
const KEY_DIR: Record<string, Dir> = {
  arrowup: 0,
  w: 0,
  '8': 0,
  arrowdown: 1,
  s: 1,
  '2': 1,
  arrowleft: 2,
  a: 2,
  '4': 2,
  arrowright: 3,
  d: 3,
  '6': 3,
};

export class Input {
  desiredDir: Dir | -1 = -1;
  private readonly down = new Set<string>();
  // File des touches « tout juste pressées » (front montant), pour les menus.
  private readonly queue: string[] = [];

  constructor(target: Window = window) {
    target.addEventListener('keydown', (e) => this.onKey(e, true));
    target.addEventListener('keyup', (e) => this.onKey(e, false));
  }

  private onKey(e: KeyboardEvent, isDown: boolean): void {
    const k = e.key.toLowerCase();
    const dir = KEY_DIR[k];
    if (dir !== undefined) {
      if (isDown) this.desiredDir = dir;
      e.preventDefault();
    }
    if (k === ' ' || k === 'control') e.preventDefault();
    if (isDown && !e.repeat) this.queue.push(k);
    if (isDown) this.down.add(k);
    else this.down.delete(k);
  }

  /** Dépile la prochaine touche pressée (front montant), ou null. */
  takeKey(): string | null {
    return this.queue.shift() ?? null;
  }

  clearKeys(): void {
    this.queue.length = 0;
  }

  /** Injecte une touche logique dans la file (utilisé par les contrôles tactiles). */
  pushKey(k: string): void {
    this.queue.push(k);
  }

  // --- Tactile (croix/boutons à l'écran ; cf. platform/touch.ts) ---
  touchFast = false;
  touchFire = false;

  /** CTRL maintenu → mode rapide. */
  get fast(): boolean {
    return this.down.has('control') || this.gpFast || this.touchFast;
  }

  /** Espace → tir. */
  get firing(): boolean {
    return this.down.has(' ') || this.gpFire || this.touchFire;
  }

  isDown(key: string): boolean {
    return this.down.has(key.toLowerCase());
  }

  // --- Manette (Gamepad API, mapping façon Xbox comme le jeu original) ---
  private gpFast = false;
  private gpFire = false;
  private readonly gpPrev: boolean[] = [];

  /** À appeler une fois par frame avant de lire l'entrée. */
  pollGamepad(): void {
    const pads = navigator.getGamepads?.();
    const gp = pads && Array.from(pads).find((p) => p);
    if (!gp) {
      this.gpFast = false;
      this.gpFire = false;
      return;
    }

    // Direction : stick gauche ou croix directionnelle.
    const ax = gp.axes[0] ?? 0;
    const ay = gp.axes[1] ?? 0;
    const pressed = (i: number) => gp.buttons[i]?.pressed ?? false;
    const T = 0.5;
    if (ay < -T || pressed(12)) this.desiredDir = 0; // haut
    else if (ay > T || pressed(13)) this.desiredDir = 1; // bas
    else if (ax < -T || pressed(14)) this.desiredDir = 2; // gauche
    else if (ax > T || pressed(15)) this.desiredDir = 3; // droite

    this.gpFast = pressed(2); // X → vitesse
    this.gpFire = pressed(0); // A → tir (et validation menu via front montant ci-dessous)

    // Boutons « menu » : front montant injecté dans la file de touches.
    const edge = (i: number, key: string) => {
      const now = pressed(i);
      if (now && !this.gpPrev[i]) this.queue.push(key);
      this.gpPrev[i] = now;
    };
    edge(0, 'enter'); // A
    edge(7, 'enter'); // Start
    edge(1, 'escape'); // B
    edge(12, 'arrowup');
    edge(13, 'arrowdown');
    edge(14, 'arrowleft');
    edge(15, 'arrowright');
  }
}
