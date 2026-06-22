// Vue PixiJS des entités et des pills, pour le rendu 2.5D.
//  - pills et ombres au sol → calque « ground » (sous tout).
//  - Pacman, fantômes, spawn, balle → calque « world » trié par profondeur
//    (zIndex = Y des pieds), pour passer derrière les murs plus proches.

import { Container, Sprite, Graphics } from 'pixi.js';
import { MAPX, MAPY, TILE_SIZE } from '../../core/constants';
import { Tool } from '../../core/tools';
import { depthOfCell } from './decorView';
import type { PlayState } from '../../core/game';
import type { Level } from '../../core/types';
import type { PixiTextures } from './pixiTextures';

const GHOST_OFFSET = -9;
const clamp = (v: number, lo: number, hi: number): number => (v < lo ? lo : v > hi ? hi : v);

// Interpolation du rendu : la simulation avance à 60 Hz, mais le rendu peut
// tourner plus vite (écran 144 Hz). On lisse la position affichée entre le pas
// précédent et le pas courant. Au-delà de MAX_STEP (téléport, wrap, respawn),
// on ne lisse PAS (sinon l'entité « glisserait » à travers l'écran).
const MAX_STEP = 24;
interface Motion {
  px: number;
  py: number;
  x: number;
  y: number;
}
type XY = { x: number; y: number };

function capture(m: Motion, x: number, y: number, snap: boolean): void {
  const jump = snap || Number.isNaN(m.x) || Math.abs(x - m.x) > MAX_STEP || Math.abs(y - m.y) > MAX_STEP;
  m.px = jump ? x : m.x;
  m.py = jump ? y : m.y;
  m.x = x;
  m.y = y;
}

function lerp(m: Motion, a: number, fx: number, fy: number): XY {
  if (Number.isNaN(m.x)) return { x: fx, y: fy }; // pas encore capturé
  return { x: m.px + (m.x - m.px) * a, y: m.py + (m.y - m.py) * a };
}

interface GhostSprites {
  container: Container;
  top: Sprite;
  bottom: Sprite;
  full: Sprite;
}

export class EntityView {
  private readonly pacman: Sprite;
  private readonly ghosts: GhostSprites[] = [];
  private readonly spawn: Sprite;
  private readonly bullet: Sprite;
  private readonly bombG = new Graphics(); // bombe posée (outil BOMB)
  private readonly pillLayer = new Container();
  private readonly shadows = new Graphics(); // ombres au sol des entités
  private pillSprites: (Sprite | null)[][] = [];

  // Tampons d'interpolation (position pas précédent → pas courant).
  private readonly pacM: Motion = { px: NaN, py: NaN, x: NaN, y: NaN };
  private readonly ghostM: Motion[] = Array.from({ length: 4 }, () => ({ px: NaN, py: NaN, x: NaN, y: NaN }));
  private readonly bulletM: Motion = { px: NaN, py: NaN, x: NaN, y: NaN };

  /** @param ground calque sol (pills, ombres) ; @param world calque trié (entités). */
  constructor(
    private readonly tex: PixiTextures,
    private readonly ground: Container,
    private readonly world: Container,
  ) {
    this.ground.addChild(this.pillLayer);
    this.ground.addChild(this.shadows);

    this.spawn = new Sprite();
    this.spawn.visible = false;
    this.world.addChild(this.spawn);

    for (let i = 0; i < 4; i++) {
      const c = new Container();
      const top = new Sprite();
      const bottom = new Sprite();
      bottom.position.set(0, 44);
      const full = new Sprite();
      c.addChild(top, bottom, full);
      this.world.addChild(c);
      this.ghosts.push({ container: c, top, bottom, full });
    }

    this.pacman = new Sprite({ texture: tex.pacman[1], anchor: 0.5 });
    this.world.addChild(this.pacman);

    this.bullet = new Sprite({ anchor: 0.5, texture: tex.bullet });
    this.bullet.visible = false;
    this.world.addChild(this.bullet);

    this.bombG.visible = false;
    this.world.addChild(this.bombG);
  }

  /** (Re)crée la grille de sprites de pills pour un niveau. */
  rebuildPills(level: Level): void {
    this.pillLayer.removeChildren();
    this.pillSprites = [];
    for (let y = 0; y < MAPY; y++) {
      const row: (Sprite | null)[] = [];
      for (let x = 0; x < MAPX; x++) {
        const cell = level.map[y][x]!;
        if (cell.isPill && cell.tile) {
          const s = new Sprite(this.tex.pills[cell.tile - 1]);
          s.position.set(x * TILE_SIZE, y * TILE_SIZE);
          this.pillLayer.addChild(s);
          row.push(s);
        } else {
          row.push(null);
        }
      }
      this.pillSprites.push(row);
    }
  }

  /** Capture les positions après un pas de simulation (appelé à chaque tick). */
  captureMotion(play: PlayState): void {
    capture(this.pacM, play.pacman.x, play.pacman.y, play.pacman.dead);
    for (let i = 0; i < 4; i++) {
      const g = play.ghosts[i]!;
      capture(this.ghostM[i]!, g.x, g.y, g.dead);
    }
    capture(this.bulletM, play.bullet.x, play.bullet.y, !play.bullet.active);
  }

  /** Fige les tampons d'interpolation sur les positions courantes (snap immédiat).
   *  Appelé au chargement d'un niveau / changement de joueur : sans cela, pendant
   *  le « READY? », Pacman et les fantômes resteraient affichés aux positions du
   *  niveau précédent (les tampons ne sont sinon réécrits qu'en cours de partie). */
  snap(play: PlayState): void {
    capture(this.pacM, play.pacman.x, play.pacman.y, true);
    for (let i = 0; i < 4; i++) capture(this.ghostM[i]!, play.ghosts[i]!.x, play.ghosts[i]!.y, true);
    capture(this.bulletM, play.bullet.x, play.bullet.y, true);
  }

  /** @param alpha fraction (0..1) entre le pas précédent et le pas courant. */
  update(play: PlayState, alpha: number): void {
    const pp = lerp(this.pacM, alpha, play.pacman.x, play.pacman.y);
    const gp = this.ghostM.map((m, i) => lerp(m, alpha, play.ghosts[i]!.x, play.ghosts[i]!.y));
    const bp = lerp(this.bulletM, alpha, play.bullet.x, play.bullet.y);
    this.updatePills(play);
    this.updateShadows(play, pp, gp);
    this.updateSpawn(play);
    this.updateGhosts(play, gp);
    this.updatePacman(play, pp);
    this.updateBullet(play, bp);
    this.updateBomb(play);
  }

  private updateBomb(play: PlayState): void {
    const b = play.bomb;
    this.bombG.visible = b.active;
    if (!b.active) return;
    this.bombG.clear();
    this.bombG.position.set(b.x, b.y);
    this.bombG.zIndex = b.y + 10;
    this.bombG.circle(0, 4, 11).fill(0x1c1c22).stroke({ width: 1.5, color: 0x3a3a48 });
    // Mèche qui clignote de plus en plus vite à l'approche de l'explosion.
    const speed = Math.max(3, Math.floor(b.timer / 12));
    if (Math.floor(b.timer / speed) % 2 === 0) {
      this.bombG.circle(6, -9, 3).fill(0xff5a00);
    }
  }

  /** Position écran (px) du sprite Pacman après interpolation (diagnostic/tests). */
  get pacmanScreenPos(): XY {
    return { x: this.pacman.position.x, y: this.pacman.position.y };
  }

  /** Cache les entités mobiles (phase éteinte du clignotement de fin de niveau). */
  hideEntities(): void {
    this.pacman.visible = false;
    this.bullet.visible = false;
    this.bombG.visible = false;
    this.spawn.visible = false;
    for (const gs of this.ghosts) gs.container.visible = false;
    this.shadows.clear();
  }

  private updateShadows(play: PlayState, pp: XY, gp: XY[]): void {
    const g = this.shadows;
    g.clear();
    if (!play.pacman.dead) g.ellipse(pp.x + 2, pp.y + 16, 15, 6).fill(0x000010);
    for (let i = 0; i < 4; i++) {
      if (!play.ghosts[i]!.dead) g.ellipse(gp[i]!.x + 18, gp[i]!.y + 32, 14, 6).fill(0x000010);
    }
    g.alpha = 0.4;
  }

  private updatePills(play: PlayState): void {
    const diamond = play.toolInUse[Tool.BLUE_DIAMOND] ? 4 : play.toolInUse[Tool.PINK_DIAMOND] ? 5 : 0;
    for (let y = 0; y < MAPY; y++) {
      for (let x = 0; x < MAPX; x++) {
        const s = this.pillSprites[y]?.[x];
        if (!s) continue;
        const cell = play.level.map[y][x]!;
        const alive = cell.isPill && cell.tile > 0;
        s.visible = alive;
        if (!alive) continue;
        if (diamond && !cell.isPowerpill) {
          s.texture = this.tex.tools[diamond - 1]!;
          s.position.set(x * TILE_SIZE - 9, y * TILE_SIZE - 9);
        } else {
          s.texture = this.tex.pills[cell.tile - 1]!;
          s.position.set(x * TILE_SIZE, y * TILE_SIZE);
        }
      }
    }
  }

  private updateSpawn(play: PlayState): void {
    if (play.spawnKind === 'none') {
      this.spawn.visible = false;
      return;
    }
    this.spawn.visible = true;
    this.spawn.texture = play.spawnKind === 'pickup' ? this.tex.pickups[play.spawnCurrent]! : this.tex.tools[play.spawnCurrent - 1]!;
    this.spawn.position.set(play.level.pickup.x * TILE_SIZE - 9, play.level.pickup.y * TILE_SIZE - 9);
    this.spawn.zIndex = depthOfCell(play.level.pickup.y);
  }

  private updatePacman(play: PlayState, pp: XY): void {
    const p = play.pacman;
    this.pacman.visible = !p.dead;
    this.pacman.texture = this.tex.pacman[clamp(p.curImg, 0, 24)]!;
    this.pacman.position.set(pp.x, pp.y);
    this.pacman.rotation = p.rot;
    this.pacman.scale.set(p.flip ? -1 : 1, 1);
    this.pacman.zIndex = pp.y + 14; // profondeur = pieds de Pacman
    this.pacman.alpha = play.toolInUse[Tool.PHASE] ? 0.5 : 1; // semi-transparent en PHASE
  }

  private updateGhosts(play: PlayState, gp: XY[]): void {
    for (let i = 0; i < 4; i++) {
      const g = play.ghosts[i]!;
      const gs = this.ghosts[i]!;
      const ip = gp[i]!;
      gs.container.visible = true;
      gs.container.position.set(ip.x + GHOST_OFFSET, ip.y + GHOST_OFFSET);
      gs.container.zIndex = ip.y + 26; // profondeur = pieds du fantôme

      const blinking = g.scared && g.stime - g.stimer <= g.stime / 3 && Math.floor(g.stimer / 4) % 2 === 1;
      const normal = !g.dead && !g.scared;
      gs.top.visible = normal;
      gs.bottom.visible = normal;
      gs.full.visible = !normal;

      if (g.dead) {
        gs.full.texture = this.tex.ghostEyes[clamp(g.eyes, 0, 3)]!;
      } else if (blinking) {
        gs.full.texture = this.tex.ghosts[i * 5 + 4]!;
      } else if (g.scared) {
        gs.full.texture = this.tex.blueGhost[clamp(g.curImg, 0, 3)]!;
      } else {
        gs.top.texture = this.tex.ghostTop[i]![clamp(g.eyes, 0, 4)]!;
        gs.bottom.texture = this.tex.ghostBottom[i]![clamp(g.curImg, 0, 4)]!;
      }
    }
  }

  private updateBullet(play: PlayState, bp: XY): void {
    const b = play.bullet;
    this.bullet.visible = b.active;
    if (b.active) {
      this.bullet.position.set(bp.x, bp.y);
      this.bullet.rotation = b.rot;
      this.bullet.scale.set(b.flip ? -1 : 1, 1);
      this.bullet.zIndex = bp.y + 14;
    }
  }
}
