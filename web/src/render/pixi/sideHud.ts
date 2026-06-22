// HUD latéral « synthwave » affiché sur les bords de l'écran 16:9 (zones SIDE_W
// de chaque côté de la zone de jeu). Style néon cohérent avec le menu, avec des
// animations très discrètes (léger pouls de glow) pour ne pas distraire.

import { Container, Graphics, Sprite, Text, type TextStyleFontWeight } from 'pixi.js';
import { GlowFilter } from 'pixi-filters';
import { SIDE_W, SCREEN_W, HEIGHT, MAX_ENERGY } from '../../core/constants';
import { t } from '../../i18n';
import type { PixiTextures } from './pixiTextures';

const NEON = { cyan: 0x00f0ff, magenta: 0xff2bd6, yellow: 0xffe600, green: 0x39ff14, dim: 0x6a7aa8 };
const FONT = 'menuFont, "Trebuchet MS", sans-serif';
const EXTRA_LETTERS = ['E', 'X', 'T', 'R', 'A'];

/** Données affichées par le HUD, fournies chaque frame. */
export interface HudData {
  score: number;
  levelLabel: string;
  playerLabel: string; // '' si un seul joueur
  lives: number;
  energy: number; // valeur brute (0..MAX_ENERGY)
  extra: number; // bits des lettres EXTRA
  activeTool: number; // 0 = aucun, sinon id d'outil (1..31)
  toolFrac: number; // durée restante (0..1)
  combo: number; // série de gains (0 = pas de série)
  comboMult: number; // multiplicateur de série courant
  comboProgress: number; // progression (0..1) vers le palier suivant
  timeLeft: number; // frames restantes (Time Attack), -1 si hors mode chronométré
  compact?: boolean; // appareil tactile : HUD épuré (libellés textuels masqués)
}

function label(text: string, size: number, color: number, weight: TextStyleFontWeight = 'bold'): Text {
  return new Text({ text, style: { fontFamily: FONT, fontSize: size, fontWeight: weight, fill: color, letterSpacing: 1 } });
}

export class SideHud {
  readonly container = new Container();
  private readonly border = new Graphics();
  private readonly borderGlow: GlowFilter;
  private readonly scoreLabel: Text;
  private readonly scoreVal: Text;
  private readonly levelVal: Text;
  private readonly playerVal: Text;
  private readonly comboLabel: Text;
  private readonly comboBar = new Graphics();
  private readonly timeLabelTxt: Text;
  private readonly timeVal: Text;
  private readonly livesLabel: Text;
  private readonly lives = new Graphics();
  private readonly energyLabel: Text;
  private readonly energy = new Graphics();
  private readonly extraText: Text[] = [];
  private readonly toolLabel: Text;
  private readonly toolIcon: Sprite;
  private readonly toolBar = new Graphics();
  private time = 0;
  private prevScore = -1;
  private scorePulse = 0; // « pop » du score à chaque gain

  constructor(private readonly tex: PixiTextures) {
    const rightX = SCREEN_W - SIDE_W;

    // Fond des deux panneaux (dégradé sombre + ligne d'horizon néon).
    const bg = new Graphics();
    for (const px of [0, rightX]) {
      for (let i = 0; i < HEIGHT; i += 3) {
        const k = i / HEIGHT;
        const c = (Math.round(18 + k * 8) << 16) | (Math.round(2 + k * 4) << 8) | Math.round(34 + k * 30);
        bg.rect(px, i, SIDE_W, 3).fill(c);
      }
    }
    this.container.addChild(bg);

    // Bordures néon (côté zone de jeu) avec glow pulsé.
    this.borderGlow = new GlowFilter({ color: NEON.cyan, distance: 12, outerStrength: 2, innerStrength: 0.4 });
    this.border.filters = [this.borderGlow];
    this.drawBorders();
    this.container.addChild(this.border);

    // --- Panneau gauche : SCORE + niveau ---
    const lx = SIDE_W / 2;
    this.scoreLabel = this.add(label('', 15, NEON.cyan), lx, 40, 0.5);
    this.scoreVal = this.add(label('0', 22, NEON.yellow), lx, 70, 0.5);
    this.playerVal = this.add(label('', 14, NEON.magenta), lx, 120, 0.5);
    this.levelVal = this.add(label('', 16, NEON.green), lx, 144, 0.5);
    // Série / combo (sous le niveau).
    this.comboLabel = this.add(label('', 22, NEON.yellow, '900'), lx, 196, 0.5);
    this.container.addChild(this.comboBar);
    // Chrono (mode Time Attack).
    this.timeLabelTxt = this.add(label('', 14, NEON.cyan), lx, 256, 0.5);
    this.timeVal = this.add(label('', 26, NEON.yellow, '900'), lx, 284, 0.5);

    // --- Panneau droit : vies, énergie, EXTRA, outil ---
    const rx = rightX + SIDE_W / 2;
    this.livesLabel = this.add(label('', 15, NEON.cyan), rx, 40, 0.5);
    this.lives.position.set(rightX, 0);
    this.container.addChild(this.lives);

    this.energyLabel = this.add(label('', 14, NEON.cyan), rx, 150, 0.5);
    this.energy.position.set(rightX, 0);
    this.container.addChild(this.energy);

    // Lettres EXTRA (forment le mot elles-mêmes ; pas de libellé séparé).
    EXTRA_LETTERS.forEach((ltr, i) => {
      this.extraText.push(this.add(label(ltr, 22, 0x444a66, '900'), rx - 40 + i * 20, 345, 0.5));
    });

    this.toolLabel = this.add(label('', 14, NEON.cyan), rx, 430, 0.5);
    this.toolIcon = new Sprite();
    this.toolIcon.anchor.set(0.5);
    this.toolIcon.position.set(rx, 470);
    this.toolIcon.visible = false;
    this.container.addChild(this.toolIcon);
    this.toolBar.position.set(rightX, 0);
    this.container.addChild(this.toolBar);
  }

  private add(t: Text, x: number, y: number, anchor = 0): Text {
    t.anchor.set(anchor, 0.5);
    t.position.set(x, y);
    this.container.addChild(t);
    return t;
  }

  private drawBorders(): void {
    const g = this.border;
    g.clear();
    g.rect(SIDE_W - 2, 0, 2, HEIGHT).fill(NEON.cyan);
    g.rect(SCREEN_W - SIDE_W, 0, 2, HEIGHT).fill(NEON.cyan);
  }

  update(d: HudData, dt: number): void {
    this.time += dt;
    this.borderGlow.outerStrength = 1.6 + 0.7 * (0.5 + 0.5 * Math.sin(this.time * 2));

    // Libellés traduisibles (réassignés via t() : réactif au changement de langue).
    const tr = t();
    this.scoreLabel.text = tr.score;
    this.livesLabel.text = tr.lives;
    this.energyLabel.text = tr.energy;
    this.toolLabel.text = tr.tool;

    // HUD épuré sur mobile : on masque les petits libellés textuels (les valeurs
    // et icônes restent), pour alléger l'affichage sur un petit écran.
    const showLabels = !d.compact;
    this.scoreLabel.visible = showLabels;
    this.livesLabel.visible = showLabels;
    this.energyLabel.visible = showLabels;
    this.toolLabel.visible = showLabels;

    this.scoreVal.text = d.score.toLocaleString('en-US');
    this.playerVal.text = d.playerLabel;
    this.levelVal.text = d.levelLabel;

    // Jauge de série : ×N + barre de progression vers le palier suivant.
    const showCombo = d.combo > 0;
    this.comboLabel.visible = showCombo;
    this.comboBar.clear();
    if (showCombo) {
      this.comboLabel.text = `×${d.comboMult}`;
      const hot = d.comboMult >= 4 ? NEON.magenta : d.comboMult >= 2 ? NEON.yellow : NEON.cyan;
      this.comboLabel.style.fill = hot;
      const bx = SIDE_W / 2 - 40;
      const bw = 80;
      this.comboBar.roundRect(bx, 214, bw, 6, 3).fill(0x1a2348);
      this.comboBar.roundRect(bx, 214, bw * Math.min(1, d.comboProgress), 6, 3).fill(hot);
    }

    // Chrono (Time Attack) : MM:SS, rouge sous les 30 s.
    const showTime = d.timeLeft >= 0;
    this.timeLabelTxt.visible = showTime;
    this.timeVal.visible = showTime;
    if (showTime) {
      this.timeLabelTxt.text = tr.timeLabel;
      const secs = Math.ceil(d.timeLeft / 60);
      const mm = Math.floor(secs / 60);
      const ss = secs % 60;
      this.timeVal.text = `${mm}:${ss.toString().padStart(2, '0')}`;
      this.timeVal.style.fill = secs <= 30 ? 0xff3b3b : NEON.yellow;
    }

    // « Pop » du score : impulsion proportionnelle au gain (plafonnée), qui
    // décroît. Les petits gains (pills) sont quasi imperceptibles.
    if (this.prevScore >= 0 && d.score > this.prevScore) {
      this.scorePulse = Math.min(0.5, Math.max(this.scorePulse, (d.score - this.prevScore) / 800));
    }
    this.prevScore = d.score;
    this.scorePulse = Math.max(0, this.scorePulse - dt * 4);
    this.scoreVal.scale.set(1 + this.scorePulse);

    // Vies : petits Pacman néon (coordonnées locales au conteneur, calé à droite).
    this.lives.clear();
    const n = Math.max(0, Math.min(5, d.lives));
    for (let i = 0; i < n; i++) {
      const cx = SIDE_W / 2 - (n - 1) * 12 + i * 24;
      const cy = 78;
      this.lives.moveTo(cx, cy).arc(cx, cy, 9, 0.25 * Math.PI, 1.75 * Math.PI).fill(NEON.yellow);
    }

    // Énergie : barre verticale.
    this.energy.clear();
    const bh = 150;
    const by = 172;
    const bx = SIDE_W / 2 - 14;
    this.energy.rect(bx, by, 28, bh).fill(0x101830);
    const f = Math.max(0, Math.min(1, d.energy / MAX_ENERGY));
    this.energy.rect(bx, by + bh * (1 - f), 28, bh * f).fill(0xff8a00);
    this.energy.rect(bx - 1, by - 1, 30, bh + 2).stroke({ width: 1, color: NEON.dim });

    // EXTRA.
    EXTRA_LETTERS.forEach((_, i) => {
      this.extraText[i]!.style.fill = d.extra & (1 << i) ? NEON.yellow : 0x444a66;
    });

    // Outil actif.
    this.toolBar.clear();
    if (d.activeTool > 0) {
      this.toolIcon.visible = true;
      this.toolIcon.texture = this.tex.tools[d.activeTool - 1]!;
      const bw = 60;
      const ox = SIDE_W / 2 - bw / 2;
      this.toolBar.rect(ox, 500, bw * Math.max(0, Math.min(1, d.toolFrac)), 5).fill(NEON.cyan);
    } else {
      this.toolIcon.visible = false;
    }
  }
}
