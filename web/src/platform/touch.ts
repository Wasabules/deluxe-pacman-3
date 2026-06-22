// Contrôles tactiles (Android/iOS) : croix directionnelle à glissement + boutons
// d'action. Superposés en HTML par-dessus le jeu (overlay #touch), ils pilotent
// le même objet `Input` que le clavier/manette — le `core` reste inchangé.
//
// - Croix : zone ronde façon joystick. La direction suit le pouce (glissement
//   continu), avec zone morte centrale. Comme au clavier, `desiredDir` est
//   persistant : relâcher n'arrête pas Pacman, il continue jusqu'au prochain mur.
// - Tir (●) : maintenu = tir continu en jeu ; un appui = « valider » dans les menus.
// - Boost (»») : maintenu = mode rapide (comme CTRL).
// - Pause (‖) : injecte « escape » (pause en jeu, retour dans les menus).
//
// Multi-touch géré nativement par les Pointer Events (croix + bouton simultanés).

import type { Input } from './input';

const DIR_KEY = ['arrowup', 'arrowdown', 'arrowleft', 'arrowright'] as const;

export class TouchControls {
  readonly root: HTMLDivElement;
  private dpadId = -1; // pointeur qui pilote la croix (-1 = aucun)
  private curDir = -1; // direction visuelle courante de la croix

  constructor(private readonly input: Input) {
    this.root = document.createElement('div');
    this.root.id = 'touch';
    this.root.innerHTML = `
      <button class="tb fs" type="button" aria-label="Plein écran">
        <svg viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor"
             stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M4 9V4h5M20 9V4h-5M4 15v5h5M20 15v5h-5" />
        </svg>
      </button>
      <div class="dpad" aria-label="Direction">
        <span class="arrow up"></span><span class="arrow down"></span>
        <span class="arrow left"></span><span class="arrow right"></span>
        <span class="hub"></span>
      </div>
      <button class="tb pause" type="button" aria-label="Pause">‖</button>
      <div class="actions">
        <button class="tb boost" type="button" aria-label="Boost">»</button>
        <button class="tb fire" type="button" aria-label="Tir / Valider">●</button>
      </div>`;
    document.body.appendChild(this.root);

    this.bindFullscreen(this.q('.fs'));
    this.bindDpad(this.q('.dpad'));
    this.bindHold(this.q('.boost'), (on) => (input.touchFast = on));
    this.bindHold(this.q('.fire'), (on) => {
      input.touchFire = on;
      if (on) input.pushKey('enter'); // sert aussi de validation dans les menus
    });
    this.bindTap(this.q('.pause'), () => input.pushKey('escape'));
  }

  /** Affiche/masque l'overlay tactile (caché dans les écrans textuels plein écran). */
  setVisible(v: boolean): void {
    this.root.style.display = v ? '' : 'none';
  }

  private q(sel: string): HTMLElement {
    return this.root.querySelector(sel) as HTMLElement;
  }

  private setDir(d: number): void {
    if (d === this.curDir) return;
    this.curDir = d;
    this.q('.dpad').dataset.dir = d >= 0 ? String(d) : '';
    if (d >= 0) {
      this.input.desiredDir = d as 0 | 1 | 2 | 3;
      this.input.pushKey(DIR_KEY[d]!); // navigation des menus (front montant)
    }
  }

  private bindDpad(el: HTMLElement): void {
    const dirAt = (e: PointerEvent): number => {
      const r = el.getBoundingClientRect();
      const dx = e.clientX - (r.left + r.width / 2);
      const dy = e.clientY - (r.top + r.height / 2);
      if (dx * dx + dy * dy < 12 * 12) return -1; // zone morte centrale
      return Math.abs(dx) > Math.abs(dy) ? (dx > 0 ? 3 : 2) : dy > 0 ? 1 : 0;
    };
    el.addEventListener('pointerdown', (e) => {
      this.dpadId = e.pointerId;
      try {
        el.setPointerCapture(e.pointerId);
      } catch {
        /* pointeur déjà relâché / synthétique : sans capture, on reste fonctionnel */
      }
      this.setDir(dirAt(e));
      e.preventDefault();
    });
    el.addEventListener('pointermove', (e) => {
      if (e.pointerId === this.dpadId) this.setDir(dirAt(e));
    });
    const end = (e: PointerEvent): void => {
      if (e.pointerId !== this.dpadId) return;
      this.dpadId = -1;
      this.setDir(-1); // éteint le surlignage ; desiredDir reste (Pacman continue)
    };
    el.addEventListener('pointerup', end);
    el.addEventListener('pointercancel', end);
  }

  private bindHold(btn: HTMLElement, set: (on: boolean) => void): void {
    const on = (e: PointerEvent): void => {
      try {
        btn.setPointerCapture(e.pointerId);
      } catch {
        /* idem : capture facultative */
      }
      btn.classList.add('on');
      set(true);
      e.preventDefault();
    };
    const off = (): void => {
      btn.classList.remove('on');
      set(false);
    };
    btn.addEventListener('pointerdown', on);
    btn.addEventListener('pointerup', off);
    btn.addEventListener('pointercancel', off);
  }

  private bindTap(btn: HTMLElement, fn: () => void): void {
    btn.addEventListener('pointerdown', (e) => {
      btn.classList.add('on');
      fn();
      e.preventDefault();
    });
    const off = (): void => btn.classList.remove('on');
    btn.addEventListener('pointerup', off);
    btn.addEventListener('pointercancel', off);
  }

  // Bouton plein écran : masque les barres du navigateur sur mobile. On le cache
  // une fois en plein écran (on en sort via le geste système ou la pause).
  private bindFullscreen(btn: HTMLElement): void {
    const el = document.documentElement as HTMLElement & { webkitRequestFullscreen?: () => Promise<void> };
    const doc = document as Document & { webkitFullscreenElement?: Element };
    const request = el.requestFullscreen?.bind(el) ?? el.webkitRequestFullscreen?.bind(el);
    if (!request) {
      btn.style.display = 'none'; // API absente (ex. iOS Safari → installer en PWA)
      return;
    }
    this.bindTap(btn, () => void Promise.resolve(request()).catch(() => {}));
    const sync = (): void => {
      const fs = document.fullscreenElement ?? doc.webkitFullscreenElement ?? null;
      btn.style.display = fs ? 'none' : '';
    };
    document.addEventListener('fullscreenchange', sync);
    document.addEventListener('webkitfullscreenchange', sync);
  }
}

/** Détecte un appareil principalement tactile (téléphone/tablette).
 *
 * On teste le pointeur *primaire* (`pointer: coarse`) plutôt que la simple
 * présence d'un écran tactile : un PC tactile piloté à la souris a un pointeur
 * fin et ne doit donc pas afficher la croix. Repli sur maxTouchPoints si la
 * Media Query n'est pas disponible (très anciens navigateurs). */
export function isTouchDevice(): boolean {
  if (typeof matchMedia === 'function') return matchMedia('(pointer: coarse)').matches;
  return 'ontouchstart' in window || navigator.maxTouchPoints > 0;
}
