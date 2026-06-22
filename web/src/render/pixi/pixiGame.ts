// Orchestration du rendu de jeu en PixiJS (WebGL).
// Deux calques pour le rendu 2.5D :
//   - ground : sol (fond, spawns, téléports, ombres, pills) — toujours dessous.
//   - world  : murs extrudés + entités, MÉLANGÉS et triés par profondeur (zIndex)
//     pour que les murs proches recouvrent Pacman et les fantômes.
// Le HUD et les écrans hors-jeu restent sur un canvas 2D superposé.

import { Application, Container, Sprite } from 'pixi.js';
import type { GameSprites } from '../../platform/assets';
import { buildPixiTextures, type PixiTextures } from './pixiTextures';
import { buildGround, buildWalls } from './decorView';
import { EntityView } from './entityView';
import { Effects } from './effects';
import { MenuScene, type MenuModel } from './menuScene';
import { SideHud, type HudData } from './sideHud';
import { SCREEN_W, HEIGHT, GAME_OFFSET_X, PLAYFIELD_X, PLAYFIELD_Y, PLAYFIELD_W, PLAYFIELD_H } from '../../core/constants';
import type { PlayState } from '../../core/game';
import type { Level } from '../../core/types';

/** Choisit le backend de rendu : WebGL s'il fonctionne, sinon WebGPU en repli
 *  (utile quand ANGLE/EGL échoue mais que D3D12/Vulkan est dispo). */
function detectRenderer(): 'webgl' | 'webgpu' {
  try {
    const c = document.createElement('canvas');
    if (c.getContext('webgl2') || c.getContext('webgl')) return 'webgl';
  } catch {
    /* getContext a levé : WebGL indisponible */
  }
  if (typeof navigator !== 'undefined' && 'gpu' in navigator) return 'webgpu';
  return 'webgl'; // dernier recours (échouera proprement → message d'aide)
}

export class PixiGame {
  readonly app = new Application();
  private textures!: PixiTextures;
  private entities!: EntityView;
  private effects!: Effects;
  private menu!: MenuScene;
  private hud!: SideHud;
  private readonly scene = new Container(); // jeu, centré dans l'écran 16:9
  private readonly ground = new Container(); // sol (sous tout)
  private readonly world = new Container(); // murs + entités, triés par profondeur
  private groundDecor: Container | null = null;
  private wallSprites: Sprite[] = [];
  private bgLevel: Level | null = null;
  private bgShadows = true;
  private menuLast = 0;
  private hudLast = 0;

  /** Initialise PixiJS sur le canvas, à partir des sprites déjà chargés.
   *  @throws si ni WebGL ni WebGPU ne peuvent créer de contexte. */
  async init(canvas: HTMLCanvasElement, sprites: GameSprites): Promise<void> {
    await this.app.init({
      canvas,
      width: SCREEN_W,
      height: HEIGHT,
      background: 0x000000,
      backgroundAlpha: 0,
      antialias: false,
      autoStart: false, // on rend manuellement depuis la boucle de jeu
      // Si WebGL ne crée pas de contexte (pilote/accélération matérielle KO),
      // on tente WebGPU (backend D3D12/Vulkan distinct) plutôt qu'un écran noir.
      preference: detectRenderer(),
    });
    // Polices arcade lisibles (Orbitron) : 700 pour les textes, 900 pour le titre.
    // Best-effort ; fallback CSS sinon.
    try {
      const body = new FontFace('menuFont', 'url(Fonts/Orbitron-700.woff2)');
      const title = new FontFace('titleFont', 'url(Fonts/Orbitron-900.woff2)');
      await Promise.all([body.load(), title.load()]);
      document.fonts.add(body);
      document.fonts.add(title);
    } catch {
      /* fallback système (police arcade indisponible) */
    }
    this.textures = buildPixiTextures(sprites);
    this.app.stage.eventMode = 'static'; // active les events (lien crédit du menu)
    this.world.sortableChildren = true; // tri par zIndex = profondeur

    // Jeu centré dans l'écran 16:9 (décalé de GAME_OFFSET_X).
    this.scene.position.set(GAME_OFFSET_X + PLAYFIELD_X, PLAYFIELD_Y);
    this.scene.addChild(this.ground);
    this.scene.addChild(this.world);
    this.app.stage.addChild(this.scene);

    this.entities = new EntityView(this.textures, this.ground, this.world);

    // Effets « juice » : la traînée passe SOUS les entités (insérée juste après
    // le sol, avant `world`) ; les particules/combos restent au-dessus de tout ;
    // le voile de flash est au-dessus du jeu mais sous le HUD.
    this.effects = new Effects(this.scene);
    this.scene.addChildAt(this.effects.trailLayer, this.scene.getChildIndex(this.world));
    this.scene.addChild(this.effects.gameLayer);
    this.app.stage.addChild(this.effects.flashG);

    // HUD latéral synthwave (par-dessus le jeu, sous le menu).
    this.hud = new SideHud(this.textures);
    this.hud.container.visible = false;
    this.app.stage.addChild(this.hud.container);

    this.menu = new MenuScene();
    this.menu.container.visible = false;
    this.app.stage.addChild(this.menu.container);
  }

  /** Affiche et anime le menu d'accueil (écrans intro/menu). */
  renderMenu(model: MenuModel, nowMs: number): void {
    this.scene.visible = false;
    this.hud.container.visible = false;
    this.effects.flashG.visible = false;
    this.menu.container.visible = true;
    const dt = this.menuLast ? Math.min(0.05, (nowMs - this.menuLast) / 1000) : 0.016;
    this.menuLast = nowMs;
    this.menu.update(dt, model);
    this.app.render();
  }

  /** Reconstruit le décor et les pills quand le niveau (ou les ombres) change. */
  private syncLevel(play: PlayState, shadows: boolean): void {
    if (this.bgLevel === play.level && this.bgShadows === shadows) return;
    this.bgLevel = play.level;
    this.bgShadows = shadows;

    // Sol : on remplace l'ancien décor sol (sous les pills/ombres = index 0).
    if (this.groundDecor) {
      this.ground.removeChild(this.groundDecor);
      this.groundDecor.destroy({ children: true });
    }
    this.groundDecor = buildGround(this.textures, play.level, shadows);
    this.ground.addChildAt(this.groundDecor, 0);

    // Murs : on retire les anciens (en gardant les entités), on ajoute les nouveaux.
    for (const s of this.wallSprites) {
      this.world.removeChild(s);
      s.destroy();
    }
    this.wallSprites = buildWalls(this.textures, play.level);
    for (const s of this.wallSprites) this.world.addChild(s);

    this.entities.rebuildPills(play.level);
    this.entities.snap(play); // fige les entités aux positions de départ du nouveau niveau
    this.effects.reset(); // pas d'effets résiduels d'un niveau à l'autre
  }

  /** Injecte les événements de jeu (gobages, mort…) dans les effets. À appeler
   *  une fois par tick logique (comme l'audio), avant le rendu. */
  feedEffects(play: PlayState): void {
    this.effects.feed(play);
  }

  /** Mémorise les positions après un pas de simulation, pour l'interpolation du
   *  rendu (écran à taux > 60 Hz). À appeler une fois par tick, en jeu. */
  captureMotion(play: PlayState): void {
    this.entities.captureMotion(play);
  }

  /** Diagnostic : déclenche un effet de démonstration au centre du labyrinthe. */
  debugFx(): void {
    this.effects.demo(PLAYFIELD_W / 2, PLAYFIELD_H / 2);
  }

  /** Diagnostic : position écran du sprite Pacman (test d'interpolation/snap). */
  debugPacmanXY(): { x: number; y: number } {
    return this.entities.pacmanScreenPos;
  }

  /** Affiche une frame de jeu. `entitiesVisible=false` (flash) : décor seul.
   *  @param alpha fraction d'interpolation (0..1) entre deux pas de simulation. */
  render(play: PlayState, shadows: boolean, hud: HudData, nowMs: number, alpha: number, entitiesVisible = true): void {
    this.scene.visible = true;
    this.menu.container.visible = false;
    this.hud.container.visible = true;
    this.syncLevel(play, shadows);
    if (entitiesVisible) this.entities.update(play, alpha);
    else this.entities.hideEntities();
    const dt = this.hudLast ? Math.min(0.05, (nowMs - this.hudLast) / 1000) : 0.016;
    this.hudLast = nowMs;
    this.hud.update(hud, dt);
    this.effects.update(dt);
    this.app.render();
  }

  /** Masque la scène (écrans hors-jeu) et rend un cadre vide/transparent. */
  hide(): void {
    this.scene.visible = false;
    this.hud.container.visible = false;
    this.menu.container.visible = false;
    this.effects.flashG.visible = false;
    this.app.render();
  }
}
