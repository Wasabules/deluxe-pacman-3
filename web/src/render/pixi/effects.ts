// Effets « game feel » (juice) : particules, tremblement d'écran, flash, combos.
// Branché sur les drapeaux transitoires du PlayState (atePill, ateGhost, died…),
// exactement comme l'AudioDirector — aucune logique de jeu, pure présentation.
// Aucune ressource GPU créée par frame (pools réutilisés) pour ne pas faire fuir
// le contexte WebGL.

import { Container, Graphics, Text } from 'pixi.js';
import { SCREEN_W, HEIGHT } from '../../core/constants';
import type { PlayState } from '../../core/game';

const PARTICLE_POOL = 200;
const COMBO_POOL = 6;
const TRAIL_POOL = 26; // segments de la traînée néon (mode rapide)

interface Particle {
  g: Graphics;
  x: number;
  y: number;
  vx: number;
  vy: number;
  life: number;
  maxLife: number;
  size: number;
  gravity: number;
  drag: number;
}

interface ComboLabel {
  t: Text;
  x: number;
  y: number;
  life: number;
}

interface EmitOptions {
  count: number;
  color: number | number[];
  speed: number;
  size: number;
  life: number;
  gravity?: number;
  drag?: number;
}

export class Effects {
  readonly trailLayer = new Container(); // traînée (sous les entités : inséré avant `world`)
  readonly gameLayer = new Container(); // particules + combos (repère jeu, suit le shake)
  readonly flashG = new Graphics(); // voile plein écran (repère écran)

  private readonly particles: Particle[] = [];
  private readonly combos: ComboLabel[] = [];
  private readonly trailSegs: { g: Graphics; life: number }[] = [];
  private trailIdx = 0;
  private shakeMag = 0;
  private flashColor = 0xffffff;
  private flashAlpha = 0;
  private rngSeed = 1;

  /** @param shakeTarget conteneur du jeu décalé lors du tremblement (sa position
   *  de base est mémorisée ici). */
  constructor(private readonly shakeTarget: Container) {
    this.baseX = shakeTarget.position.x;
    this.baseY = shakeTarget.position.y;

    for (let i = 0; i < PARTICLE_POOL; i++) {
      const g = new Graphics();
      g.circle(0, 0, 4).fill(0xffffff); // cercle unité blanc, teinté/échelonné ensuite
      g.visible = false;
      this.gameLayer.addChild(g);
      this.particles.push({ g, x: 0, y: 0, vx: 0, vy: 0, life: 0, maxLife: 1, size: 1, gravity: 0, drag: 0 });
    }

    // Traînée néon : dans son propre calque, inséré SOUS les entités par pixiGame
    // (entre le sol et `world`), en fusion additive pour un halo lumineux.
    for (let i = 0; i < TRAIL_POOL; i++) {
      const g = new Graphics();
      g.circle(0, 0, 13).fill(0xffffff);
      g.blendMode = 'add';
      g.visible = false;
      this.trailLayer.addChild(g);
      this.trailSegs.push({ g, life: 0 });
    }

    for (let i = 0; i < COMBO_POOL; i++) {
      const txt = new Text({
        text: '',
        style: { fontFamily: 'menuFont, sans-serif', fontSize: 22, fontWeight: '900', fill: 0xffe600, align: 'center' },
      });
      txt.anchor.set(0.5);
      txt.visible = false;
      this.gameLayer.addChild(txt);
      this.combos.push({ t: txt, x: 0, y: 0, life: 0 });
    }

    this.flashG.rect(0, 0, SCREEN_W, HEIGHT).fill(0xffffff);
    this.flashG.visible = false;
  }

  private baseX: number;
  private baseY: number;

  // PRNG déterministe léger (évite Math.random, interdit dans les workflows et
  // suffisant pour de la variation visuelle).
  private rnd(): number {
    this.rngSeed = (this.rngSeed * 1103515245 + 12345) & 0x7fffffff;
    return this.rngSeed / 0x7fffffff;
  }

  /** Lit les événements de la frame de jeu et déclenche les effets. À appeler une
   *  fois par tick (comme l'audio), avec l'état courant. */
  feed(play: PlayState): void {
    const px = play.pacman.x;
    const py = play.pacman.y;

    if (play.atePill) {
      this.emit(px, py, { count: 5, color: 0xffe600, speed: 60, size: 0.5, life: 0.35, drag: 4 });
    }
    if (play.atePowerpill) {
      this.emit(px, py, { count: 26, color: [0x00f0ff, 0xffffff, 0xff2bd6], speed: 150, size: 0.9, life: 0.6, drag: 2.5 });
      this.shake(5);
      this.flash(0x66e0ff, 0.28);
    }
    if (play.ateGhost) {
      this.emit(px, py, { count: 18, color: [0x00f0ff, 0xffffff, 0x9d4edd], speed: 130, size: 0.8, life: 0.5, drag: 3 });
      this.shake(3);
    }

    // Points flottants « +N » (issus du core, à leur position exacte).
    for (const pop of play.scorePops) this.popScore(pop.x, pop.y, pop.value, pop.big);
    if (play.atePickup) {
      this.emit(px, py, { count: 12, color: [0x39ff14, 0xffffff], speed: 110, size: 0.7, life: 0.5, drag: 3 });
    }
    if (play.ateTool) {
      this.emit(px, py, { count: 14, color: [0xffe600, 0x00f0ff, 0xffffff], speed: 120, size: 0.7, life: 0.5, drag: 3 });
    }
    if (play.gotExtraLife) {
      this.emit(px, py, { count: 30, color: [0xffe600, 0xff2bd6, 0x39ff14, 0x00f0ff], speed: 160, size: 0.9, life: 0.8, gravity: 120, drag: 1.5 });
    }
    if (play.died) {
      this.emit(px, py, { count: 40, color: [0xff3b3b, 0xff8a00, 0xffe600], speed: 200, size: 1.0, life: 0.9, gravity: 160, drag: 1.2 });
      this.shake(12);
      this.flash(0xff2020, 0.45);
    }
    if (play.bombExploded) {
      this.emit(play.bomb.x, play.bomb.y, { count: 46, color: [0xff8a00, 0xffe000, 0xff3b3b, 0xffffff], speed: 240, size: 1.1, life: 0.8, gravity: 110, drag: 1.1 });
      this.shake(15);
      this.flash(0xff8a33, 0.5);
    }
    if (play.warped) {
      this.emit(px, py, { count: 24, color: [0x00f0ff, 0xc77bff, 0xffffff], speed: 150, size: 0.8, life: 0.5, drag: 2.5 });
    }

    // Traînée néon : uniquement en mode rapide (« speed »).
    this.trail(px, py, play.pacman.fast && !play.pacman.dead);
  }

  /** Dépose un segment de traînée à la position courante (ring buffer). */
  private trail(x: number, y: number, active: boolean): void {
    if (!active) return;
    const seg = this.trailSegs[this.trailIdx]!;
    this.trailIdx = (this.trailIdx + 1) % this.trailSegs.length;
    seg.g.position.set(x, y);
    seg.g.tint = 0xffd21a; // jaune Pacman
    seg.life = 1;
    seg.g.visible = true;
  }

  private emit(x: number, y: number, o: EmitOptions): void {
    const colors = Array.isArray(o.color) ? o.color : [o.color];
    for (let i = 0; i < o.count; i++) {
      const p = this.particles.find((q) => q.life <= 0);
      if (!p) break; // pool épuisé : on saute (jamais critique visuellement)
      const ang = this.rnd() * Math.PI * 2;
      const spd = o.speed * (0.4 + 0.6 * this.rnd());
      p.x = x;
      p.y = y;
      p.vx = Math.cos(ang) * spd;
      p.vy = Math.sin(ang) * spd;
      p.maxLife = o.life * (0.7 + 0.6 * this.rnd());
      p.life = p.maxLife;
      p.size = o.size * (0.7 + 0.6 * this.rnd());
      p.gravity = o.gravity ?? 0;
      p.drag = o.drag ?? 0;
      p.g.tint = colors[Math.floor(this.rnd() * colors.length)]!;
      p.g.visible = true;
    }
  }

  private popScore(x: number, y: number, value: number, big: boolean): void {
    const c = this.combos.find((q) => q.life <= 0);
    if (!c) return;
    c.x = x;
    c.y = y - 14;
    c.life = 1;
    c.t.text = `+${value.toLocaleString('en-US')}`;
    c.t.style.fontSize = big ? 22 : 16;
    c.t.style.fill = big ? 0xffe600 : 0xffffff;
    c.t.visible = true;
  }

  /** Effet de démonstration (diagnostic) : gros éclat + tremblement + flash + combo. */
  demo(x: number, y: number): void {
    this.emit(x, y, { count: 50, color: [0x00f0ff, 0xffffff, 0xff2bd6, 0xffe600], speed: 190, size: 1, life: 0.9, gravity: 90, drag: 1.4 });
    this.shake(10);
    this.flash(0x66e0ff, 0.4);
    this.popScore(x, y, 5000, true);
  }

  shake(mag: number): void {
    if (mag > this.shakeMag) this.shakeMag = mag;
  }

  flash(color: number, alpha: number): void {
    this.flashColor = color;
    if (alpha > this.flashAlpha) this.flashAlpha = alpha;
  }

  /** Anime tous les effets et applique le tremblement. À appeler chaque frame. */
  update(dt: number): void {
    // Traînée néon (fond additif qui s'estompe).
    for (const s of this.trailSegs) {
      if (s.life <= 0) continue;
      s.life -= dt * 3.5;
      if (s.life <= 0) {
        s.g.visible = false;
        continue;
      }
      s.g.alpha = s.life * 0.5;
      s.g.scale.set(0.55 + s.life * 0.55);
    }

    // Particules.
    for (const p of this.particles) {
      if (p.life <= 0) continue;
      p.life -= dt;
      if (p.life <= 0) {
        p.g.visible = false;
        continue;
      }
      const damp = 1 - Math.min(1, p.drag * dt);
      p.vx *= damp;
      p.vy = p.vy * damp + p.gravity * dt;
      p.x += p.vx * dt;
      p.y += p.vy * dt;
      const k = p.life / p.maxLife;
      p.g.position.set(p.x, p.y);
      p.g.scale.set(p.size * (0.4 + 0.6 * k));
      p.g.alpha = Math.min(1, k * 1.5);
    }

    // Combos flottants (montent et s'estompent).
    for (const c of this.combos) {
      if (c.life <= 0) continue;
      c.life -= dt * 1.4;
      if (c.life <= 0) {
        c.t.visible = false;
        continue;
      }
      c.y -= 26 * dt;
      c.t.position.set(c.x, c.y);
      c.t.alpha = Math.min(1, c.life * 1.6);
      c.t.scale.set(1 + (1 - c.life) * 0.3);
    }

    // Flash plein écran.
    if (this.flashAlpha > 0) {
      this.flashAlpha = Math.max(0, this.flashAlpha - dt * 1.8);
      this.flashG.visible = true;
      this.flashG.tint = this.flashColor;
      this.flashG.alpha = this.flashAlpha;
    } else if (this.flashG.visible) {
      this.flashG.visible = false;
    }

    // Tremblement d'écran (décroît, décalage aléatoire du conteneur de jeu).
    if (this.shakeMag > 0.1) {
      this.shakeMag = Math.max(0, this.shakeMag - dt * 30);
      const ox = (this.rnd() * 2 - 1) * this.shakeMag;
      const oy = (this.rnd() * 2 - 1) * this.shakeMag;
      this.shakeTarget.position.set(this.baseX + ox, this.baseY + oy);
    } else if (this.shakeTarget.position.x !== this.baseX || this.shakeTarget.position.y !== this.baseY) {
      this.shakeTarget.position.set(this.baseX, this.baseY);
    }
  }

  /** Réinitialise tous les effets (changement d'écran / de niveau). */
  reset(): void {
    for (const s of this.trailSegs) {
      s.life = 0;
      s.g.visible = false;
    }
    for (const p of this.particles) {
      p.life = 0;
      p.g.visible = false;
    }
    for (const c of this.combos) {
      c.life = 0;
      c.t.visible = false;
    }
    this.shakeMag = 0;
    this.flashAlpha = 0;
    this.flashG.visible = false;
    this.shakeTarget.position.set(this.baseX, this.baseY);
  }
}
