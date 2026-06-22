// Menu d'accueil arcade animé (PixiJS + WebGL) :
//  - fond synthwave 2.5D : grille en perspective qui défile, soleil rétro, étoiles
//  - titre néon « DELUXE PACMAN 3 » avec glow pulsé
//  - poursuite Pacman / 4 fantômes (clin d'œil arcade), dessinés en néon 8-bit
//  - effet CRT (scanlines + bombement) pour le rendu borne d'arcade
//  - sélection du nombre de joueurs et panneau de paramètres

import { Container, Graphics, Text, type TextStyleFontWeight } from 'pixi.js';
import { GlowFilter, CRTFilter } from 'pixi-filters';
// Le menu occupe tout l'écran 16:9 : on utilise SCREEN_W comme « largeur ».
import { SCREEN_W as WIDTH, HEIGHT } from '../../core/constants';
import { t } from '../../i18n';

const NEON = {
  cyan: 0x00f0ff,
  magenta: 0xff2bd6,
  yellow: 0xffe600,
  green: 0x39ff14,
  pink: 0xff2a6d,
  purple: 0x9d4edd,
  red: 0xff3b3b,
  white: 0xffffff,
};
const GHOST_COLORS = [NEON.red, NEON.green, NEON.cyan, NEON.purple];

const HORIZON = 312;
const FONT = 'menuFont, "Trebuchet MS", system-ui, sans-serif';
const TITLE_FONT = 'titleFont, "Arial Black", system-ui, sans-serif';
// Site de jeux de Neil Roy, auteur original de Deluxe Pacman 2.
const CREDIT_URL = 'https://nitehackr.github.io/games_index.html';

/** État affiché par le menu (fourni par l'app). */
export interface MenuModel {
  prompt: boolean; // écran d'accueil (« appuyez sur une touche »)
  players: number; // 1..4
  mode: 'main' | 'modes' | 'players' | 'settings'; // main = Continuer?/Jouer/Outils/…
  mainItems: string[]; // libellés du menu principal (« Continuer » en option)
  selIndex: number; // ligne sélectionnée dans le mode courant
  difficulty: number;
  soundVol: number; // 0..1
  musicVol: number; // 0..1
  shadows: boolean;
}

function neonText(content: string, size: number, color: number, weight: TextStyleFontWeight = 'bold', family: string = FONT): Text {
  return new Text({
    text: content,
    style: { fontFamily: family, fontSize: size, fontWeight: weight, fill: color, align: 'center', letterSpacing: 1 },
  });
}

export class MenuScene {
  readonly container = new Container();

  private readonly grid = new Graphics();
  private readonly sun = new Graphics();
  private readonly sky = new Graphics();
  private readonly stars = new Container();
  private readonly chase = new Container();
  private readonly title: Text;
  private readonly titleGlow: GlowFilter;
  private readonly subtitle: Text;
  private readonly promptText: Text;
  private readonly credit: Text; // « Merci à Neil Roy » cliquable (accueil)
  private readonly mainBox = new Container(); // menu principal : Jouer/Paramètres/Classement
  private readonly mainRows: Text[] = [];
  private readonly mainHint: Text;
  private readonly modesBox = new Container(); // choix du mode de jeu
  private readonly modesHeader: Text;
  private readonly modeRows: Text[] = [];
  private readonly modesHint: Text;
  private readonly menuBox = new Container(); // choix du nombre de joueurs
  private readonly playersHeader: Text;
  private readonly playerRows: Text[] = [];
  private readonly playHint: Text;
  private readonly settingsBox = new Container();
  private readonly settingsHeader: Text;
  private readonly settingRows: { label: Text; value: Text }[] = [];
  private readonly volBars = new Graphics(); // jauges de volume (sons / musique)
  private readonly settingsHint: Text;
  private readonly crt: CRTFilter;

  // Clés des libellés de réglages (ordre = affichage). Le menu principal, lui,
  // a un nombre d'options variable (« Continuer » conditionnel) → voir mainItems.
  private static readonly SETTING_KEYS = ['difficulty', 'sounds', 'music', 'shadows', 'language', 'back'] as const;
  private static readonly ROW_DY = 38; // espacement vertical des lignes de réglage

  private t = 0;
  private readonly starData: { g: Graphics; ph: number; sp: number }[] = [];
  private pac = { x: -60, mouth: 0 };
  // Graphismes de la poursuite, créés UNE fois (leurs filtres glow sont des
  // ressources GPU : les recréer chaque frame ferait fuir le contexte WebGL).
  private readonly pacG = new Graphics();
  private readonly pillsG = new Graphics();
  private readonly ghostG: Graphics[] = [];

  constructor() {
    // --- Ciel dégradé (bandes) + soleil rétro ---
    for (let i = 0; i < HORIZON; i += 2) {
      const k = i / HORIZON;
      const r = Math.round(20 + k * 20);
      const g = Math.round(0 + k * 10);
      const b = Math.round(40 + k * 60);
      this.sky.rect(0, i, WIDTH, 2).fill((r << 16) | (g << 8) | b);
    }
    this.container.addChild(this.sky);

    // Soleil avec bandes horizontales (style outrun).
    const cx = WIDTH / 2;
    const sy = HORIZON - 70;
    this.sun.circle(cx, sy, 90).fill(NEON.yellow);
    this.sun.circle(cx, sy, 90).fill(NEON.pink);
    for (let i = 0; i < 8; i++) {
      this.sun.rect(cx - 95, sy + 18 + i * 11, 190, 6 + i).fill(0x150022);
    }
    this.sun.alpha = 0.9;
    this.container.addChild(this.sun);

    this.container.addChild(this.stars);
    for (let i = 0; i < 60; i++) {
      const g = new Graphics();
      const x = Math.floor((i * 9871) % WIDTH);
      const y = Math.floor((i * 4567) % (HORIZON - 30));
      g.circle(x, y, i % 3 === 0 ? 1.6 : 1).fill(NEON.white);
      this.stars.addChild(g);
      this.starData.push({ g, ph: (i * 1.7) % 6.28, sp: 1 + (i % 4) * 0.6 });
    }

    this.container.addChild(this.grid);
    this.container.addChild(this.chase);

    // Poursuite : pills + Pacman + 4 fantômes, graphismes persistants.
    this.chase.addChild(this.pillsG);
    this.pacG.filters = [new GlowFilter({ color: NEON.yellow, distance: 10, outerStrength: 2 })];
    for (let i = 0; i < 4; i++) {
      const g = new Graphics();
      g.filters = [new GlowFilter({ color: GHOST_COLORS[i]!, distance: 8, outerStrength: 1.6 })];
      this.ghostG.push(g);
      this.chase.addChild(g);
    }
    this.chase.addChild(this.pacG);

    // --- Titre néon (Orbitron 900, large → taille adaptée) ---
    this.title = neonText('DELUXE PACMAN 3', 40, NEON.white, '900', TITLE_FONT);
    this.title.anchor.set(0.5);
    this.title.position.set(cx, 108);
    this.titleGlow = new GlowFilter({ color: NEON.cyan, distance: 22, outerStrength: 3, innerStrength: 0.6 });
    this.title.filters = [this.titleGlow];
    this.container.addChild(this.title);

    this.subtitle = neonText('', 18, NEON.magenta);
    this.subtitle.anchor.set(0.5);
    this.subtitle.position.set(cx, 158);
    this.container.addChild(this.subtitle);

    // --- Accueil (prompt clignotant) ---
    this.promptText = neonText('', 26, NEON.yellow);
    this.promptText.anchor.set(0.5);
    this.promptText.position.set(cx, 250);
    this.container.addChild(this.promptText);

    // Crédit cliquable vers le site de l'auteur original (uniquement à l'accueil).
    this.credit = neonText('', 16, NEON.cyan);
    this.credit.anchor.set(0.5);
    this.credit.position.set(cx, HEIGHT - 26);
    this.credit.eventMode = 'static';
    this.credit.cursor = 'pointer';
    this.credit.on('pointerover', () => (this.credit.style.fill = NEON.yellow));
    this.credit.on('pointerout', () => (this.credit.style.fill = NEON.cyan));
    this.credit.on('pointertap', () => window.open(CREDIT_URL, '_blank', 'noopener,noreferrer'));
    this.container.addChild(this.credit);

    // --- Menu principal : (Continuer ?) / Jouer / Outils / Paramètres / Classement ---
    // 5 lignes pré-créées ; positions + visibilité fixées dynamiquement (update).
    this.mainBox.position.set(cx, 228);
    this.container.addChild(this.mainBox);
    for (let i = 0; i < 5; i++) {
      const row = neonText('', 26, NEON.white);
      row.anchor.set(0.5);
      this.mainBox.addChild(row);
      this.mainRows.push(row);
    }
    this.mainHint = neonText('', 15, NEON.green);
    this.mainHint.anchor.set(0.5);
    this.mainBox.addChild(this.mainHint);

    // --- Sélection du mode de jeu ---
    this.modesBox.position.set(cx, 168);
    this.container.addChild(this.modesBox);
    this.modesHeader = neonText('', 22, NEON.cyan);
    this.modesHeader.anchor.set(0.5);
    this.modesHeader.position.set(0, -8);
    this.modesBox.addChild(this.modesHeader);
    for (let i = 0; i < 5; i++) {
      const row = neonText('', 23, NEON.white);
      row.anchor.set(0.5);
      row.position.set(0, 28 + i * 36);
      this.modesBox.addChild(row);
      this.modeRows.push(row);
    }
    this.modesHint = neonText('', 15, NEON.green);
    this.modesHint.anchor.set(0.5);
    this.modesHint.position.set(0, 28 + 5 * 36 + 14);
    this.modesBox.addChild(this.modesHint);

    // --- Sélection du nombre de joueurs ---
    this.menuBox.position.set(cx, 210);
    this.container.addChild(this.menuBox);
    this.playersHeader = neonText('', 22, NEON.cyan);
    this.playersHeader.anchor.set(0.5);
    this.playersHeader.position.set(0, -8);
    this.menuBox.addChild(this.playersHeader);
    for (let i = 1; i <= 4; i++) {
      const row = neonText('', 24, NEON.white);
      row.anchor.set(0.5);
      row.position.set(0, 24 + (i - 1) * 34);
      this.menuBox.addChild(row);
      this.playerRows.push(row);
    }
    this.playHint = neonText('', 15, NEON.green);
    this.playHint.anchor.set(0.5);
    this.playHint.position.set(0, 24 + 4 * 34 + 14);
    this.menuBox.addChild(this.playHint);

    // --- Panneau paramètres ---
    this.settingsBox.position.set(cx, 176);
    this.settingsBox.visible = false;
    this.container.addChild(this.settingsBox);
    this.settingsHeader = neonText('', 26, NEON.magenta);
    this.settingsHeader.anchor.set(0.5);
    this.settingsHeader.position.set(0, -10);
    this.settingsBox.addChild(this.settingsHeader);
    this.settingsBox.addChild(this.volBars);
    MenuScene.SETTING_KEYS.forEach((_key, i) => {
      const label = neonText('', 22, NEON.white);
      label.anchor.set(0, 0.5);
      label.position.set(-200, 36 + i * MenuScene.ROW_DY);
      const value = neonText('', 22, NEON.yellow);
      value.anchor.set(1, 0.5);
      value.position.set(200, 36 + i * MenuScene.ROW_DY);
      this.settingsBox.addChild(label, value);
      this.settingRows.push({ label, value });
    });
    this.settingsHint = neonText('', 13, NEON.green);
    this.settingsHint.anchor.set(0.5);
    this.settingsHint.position.set(0, 36 + 6 * MenuScene.ROW_DY);
    this.settingsBox.addChild(this.settingsHint);

    // --- Effet CRT global ---
    // Bombement (curvature) volontairement nul : c'est l'option la plus coûteuse
    // et instable selon le GPU ; on garde les scanlines + le grain + le vignettage.
    this.crt = new CRTFilter({ lineWidth: 2, lineContrast: 0.2, noise: 0.08, vignetting: 0.3, curvature: 0 });
    this.container.filters = [this.crt];
  }

  /** Met à jour l'animation et l'affichage à partir du modèle. */
  update(dt: number, m: MenuModel): void {
    this.t += dt;
    this.crt.time += dt * 8;
    this.crt.seed = (this.t * 13.7) % 1;

    this.drawGrid();
    this.drawChase(dt);

    // Étoiles scintillantes.
    for (const s of this.starData) s.g.alpha = 0.35 + 0.4 * (0.5 + 0.5 * Math.sin(this.t * s.sp + s.ph));

    // Titre : glow pulsé + balancement.
    this.titleGlow.outerStrength = 2.4 + 1.6 * (0.5 + 0.5 * Math.sin(this.t * 3));
    this.title.scale.set(1 + 0.02 * Math.sin(this.t * 2));
    this.subtitle.alpha = 0.6 + 0.4 * Math.sin(this.t * 4);

    // Bascule accueil / menu principal / choix joueurs / paramètres.
    this.promptText.visible = m.prompt;
    this.promptText.alpha = 0.5 + 0.5 * Math.sin(this.t * 6);
    this.mainBox.visible = !m.prompt && m.mode === 'main';
    this.modesBox.visible = !m.prompt && m.mode === 'modes';
    this.menuBox.visible = !m.prompt && m.mode === 'players';
    this.settingsBox.visible = !m.prompt && m.mode === 'settings';

    // Textes traduisibles (langue réactive : on réassigne via t(), Pixi ignore
    // les valeurs inchangées donc aucun surcoût quand la langue ne change pas).
    const tr = t();
    this.subtitle.text = `* ${tr.arcadeEdition} *`;
    this.promptText.text = tr.pressAnyKey;
    this.credit.text = tr.thanksNeil;
    this.credit.visible = m.prompt; // seulement sur l'écran d'accueil
    this.mainHint.text = tr.selectHint;
    this.modesHeader.text = tr.modeTitle;
    this.modesHint.text = tr.playHint;
    this.playersHeader.text = tr.numberOfPlayers;
    this.playHint.text = tr.playHint;

    // Sélection du mode de jeu.
    const modeLabels = [tr.modeClassic, tr.modeTimeAttack, tr.modeSurvival, tr.modeDaily, tr.modeReverse];
    this.modeRows.forEach((row, i) => {
      const sel = m.mode === 'modes' && m.selIndex === i;
      row.text = `${sel ? '> ' : '  '}${modeLabels[i]}${sel ? ' <' : ''}`;
      row.style.fill = sel ? NEON.yellow : NEON.white;
      row.scale.set(sel ? 1.1 : 1);
    });
    this.settingsHeader.text = tr.settings;
    this.settingsHint.text = tr.settingsHint;

    // Sous-titre visible seulement à l'accueil (libère la place pour le menu).
    this.subtitle.visible = m.prompt;

    // Menu principal (nombre d'options variable, centré verticalement).
    const n = m.mainItems.length;
    const dy = 40;
    const y0 = -((n - 1) / 2) * dy;
    this.mainRows.forEach((row, i) => {
      row.visible = i < n;
      if (i >= n) return;
      row.position.set(0, y0 + i * dy);
      const sel = m.mode === 'main' && m.selIndex === i;
      row.text = `${sel ? '> ' : '  '}${m.mainItems[i]}${sel ? ' <' : ''}`;
      row.style.fill = sel ? NEON.yellow : NEON.white;
      row.scale.set(sel ? 1.12 : 1);
    });
    this.mainHint.position.set(0, y0 + n * dy + 4);

    // Sélection des joueurs.
    this.playerRows.forEach((row, i) => {
      const sel = m.mode === 'players' && m.selIndex === i;
      row.text = `${sel ? '> ' : '  '}${tr.players(i + 1)}${sel ? ' <' : ''}`;
      row.style.fill = sel ? NEON.yellow : m.players === i + 1 ? NEON.cyan : 0x8aa0c0;
      row.scale.set(sel ? 1.12 : 1);
    });

    // Paramètres : libellés + valeurs (volumes en %, ombres ON/OFF, etc.).
    const pct = (v: number) => (v <= 0 ? tr.off : `${Math.round(v * 100)}%`);
    const labelKeys = [tr.difficulty, tr.sounds, tr.music, tr.shadows, tr.language, tr.back];
    const vals = [
      tr.diffName(m.difficulty).toUpperCase(),
      pct(m.soundVol),
      pct(m.musicVol),
      m.shadows ? tr.on : tr.off,
      tr.langName,
      tr.ok,
    ];
    this.settingRows.forEach((r, i) => {
      const sel = m.mode === 'settings' && m.selIndex === i;
      r.label.text = labelKeys[i]!;
      r.value.text = vals[i]!;
      r.label.style.fill = sel ? NEON.yellow : NEON.white;
      r.value.style.fill = sel ? NEON.yellow : NEON.cyan;
      r.label.x = sel ? -190 : -200;
    });

    // Jauges de volume (lignes Sons = index 1, Musique = index 2).
    this.volBars.clear();
    const drawBar = (i: number, v: number, sel: boolean) => {
      const y = 36 + i * MenuScene.ROW_DY + 13;
      const x = 40;
      const w = 118;
      this.volBars.roundRect(x, y, w, 6, 3).fill(0x1a2348);
      if (v > 0) this.volBars.roundRect(x, y, w * Math.min(1, v), 6, 3).fill(sel ? NEON.yellow : NEON.cyan);
    };
    drawBar(1, m.soundVol, m.mode === 'settings' && m.selIndex === 1);
    drawBar(2, m.musicVol, m.mode === 'settings' && m.selIndex === 2);
  }

  /** Grille synthwave en perspective qui défile vers l'écran. */
  private drawGrid(): void {
    const g = this.grid;
    g.clear();
    const cx = WIDTH / 2;
    const cols = 12;
    const spread = 88;

    // Lignes "fuyantes" (convergent vers le point de fuite).
    for (let i = -cols; i <= cols; i++) {
      g.moveTo(cx + i * spread, HEIGHT).lineTo(cx, HORIZON);
    }
    g.stroke({ width: 1.5, color: NEON.purple, alpha: 0.85 });

    // Lignes horizontales défilantes (espacement en perspective).
    const lines = 14;
    const scroll = (this.t * 0.35) % 1;
    for (let k = 0; k < lines; k++) {
      const z = (k + scroll) / lines;
      const t = z * z; // perspective
      const y = HORIZON + (HEIGHT - HORIZON) * t;
      const halfW = spread * cols * t;
      g.moveTo(cx - halfW, y).lineTo(cx + halfW, y);
    }
    g.stroke({ width: 1.5, color: NEON.cyan, alpha: 0.7 });

    // Ligne d'horizon lumineuse.
    g.moveTo(0, HORIZON).lineTo(WIDTH, HORIZON).stroke({ width: 3, color: NEON.magenta, alpha: 0.9 });
  }

  /** Pacman néon qui mange des pills, poursuivi par 4 fantômes (redraw, pas de recréation). */
  private drawChase(dt: number): void {
    const y = HORIZON - 18;
    const speed = 150;
    this.pac.x += speed * dt;
    this.pac.mouth = Math.abs(Math.sin(this.t * 10)) * 0.32;
    const loop = WIDTH + 260;
    if (this.pac.x > WIDTH + 120) this.pac.x = -140;

    // Pills devant Pacman (un seul Graphics).
    this.pillsG.clear();
    for (let px = 40; px < WIDTH; px += 44) {
      if (px < this.pac.x + 10) continue;
      this.pillsG.circle(px, y, 5).fill(NEON.yellow);
    }
    this.pillsG.alpha = 0.9;

    // Pacman.
    const m = this.pac.mouth * Math.PI;
    this.pacG.clear();
    this.pacG.moveTo(this.pac.x, y).arc(this.pac.x, y, 18, m, Math.PI * 2 - m).fill(NEON.yellow);

    // Fantômes poursuivants.
    for (let i = 0; i < 4; i++) {
      const gx = this.pac.x - 64 - i * 42 + Math.sin(this.t * 4 + i) * 4;
      const x = (((gx % loop) + loop) % loop) - 130;
      this.drawGhostShape(this.ghostG[i]!, x, y, GHOST_COLORS[i]!);
    }
  }

  private drawGhostShape(g: Graphics, x: number, y: number, color: number): void {
    g.clear();
    g.moveTo(x - 15, y + 16);
    g.lineTo(x - 15, y - 4);
    g.arc(x, y - 4, 15, Math.PI, 0);
    g.lineTo(x + 15, y + 16);
    g.lineTo(x + 9, y + 10).lineTo(x + 3, y + 16).lineTo(x - 3, y + 10).lineTo(x - 9, y + 16);
    g.fill(color);
    g.circle(x - 6, y - 4, 4).circle(x + 6, y - 4, 4).fill(NEON.white);
    g.circle(x - 5, y - 3, 2).circle(x + 7, y - 3, 2).fill(0x0020ff);
  }

  destroy(): void {
    this.container.destroy({ children: true });
  }
}
