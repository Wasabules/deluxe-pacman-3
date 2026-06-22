import '../style.css';
import { GameLoop } from './loop';
import { Session } from './session';
import { AudioDirector } from './audioDirector';
import { Renderer } from '../platform/renderer';
import { loadGameSprites, type GameSprites } from '../platform/assets';
import { fetchLevel, levelPath, bonusPath } from '../platform/levelLoader';
import { Input } from '../platform/input';
import { AudioManager } from '../platform/audio';
import { PixiGame } from '../render/pixi/pixiGame';
import type { MenuModel } from '../render/pixi/menuScene';
import { drawLevelIntro, drawGameOver, drawHiscoreEntry, drawHiscoreTable, drawPause, drawLevelClear, drawCodex, UI_FONT } from '../render/screens';
import type { HudData } from '../render/pixi/sideHud';
import { comboInfo } from '../core/game';
import type { PlayState } from '../core/game';
import type { Level } from '../core/types';
import { HEIGHT, SCREEN_W, TICK_HZ, MAX_LEVELS, MAX_BONUS } from '../core/constants';
import { t, cycleLang } from '../i18n';

function getCanvas(id: string): HTMLCanvasElement {
  const c = document.getElementById(id);
  if (!(c instanceof HTMLCanvasElement)) throw new Error(`Canvas <${id}> introuvable.`);
  return c;
}
const uiCanvas = getCanvas('ui');
const glCanvas = getCanvas('gl');
const renderer = new Renderer(uiCanvas);
const pixi = new PixiGame();
const input = new Input();

// PWA : enregistrement du service worker en production uniquement.
// En développement, on purge tout SW/cache existant pour ne jamais servir
// d'assets périmés (un SW cache-first casserait le rechargement à chaud).
if ('serviceWorker' in navigator) {
  if (import.meta.env.PROD) {
    window.addEventListener('load', () => {
      void navigator.serviceWorker.register('./sw.js').catch(() => {});
    });
  } else {
    void navigator.serviceWorker.getRegistrations().then((rs) => rs.forEach((r) => void r.unregister()));
    if ('caches' in window) void caches.keys().then((ks) => ks.forEach((k) => void caches.delete(k)));
  }
}

void bootstrap();

async function bootstrap(): Promise<void> {
  drawCenteredMessage(t().loadingResources);
  const sprites = await loadGameSprites();
  try {
    await pixi.init(glCanvas, sprites); // moteur de rendu WebGL/WebGPU (jeu)
  } catch (e) {
    console.error('Initialisation du rendu impossible (WebGL/WebGPU) :', e);
    drawRenderError();
    return; // sans rendu, inutile d'aller plus loin
  }

  // L'audio se charge en arrière-plan : le jeu démarre sans attendre les sons.
  const audio = new AudioManager();
  void audio.init();
  const director = new AudioDirector(audio);

  let shadows = true;

  // Cache des niveaux par chemin : le premier fetch est mémorisé, donc un
  // pré-chargement (prefetch du niveau suivant) rend la transition instantanée.
  const levelCache = new Map<string, Promise<Level>>();
  const cachedFetch = (path: string): Promise<Level> => {
    let p = levelCache.get(path);
    if (!p) {
      p = fetchLevel(path).catch((e) => {
        levelCache.delete(path); // échec : ne pas mémoriser, on pourra réessayer
        throw e;
      });
      levelCache.set(path, p);
    }
    return p;
  };

  const session = new Session({
    loadLevel: (clevel, bonus) => cachedFetch(bonus > 0 ? bonusPath(bonus) : levelPath(clevel)),
    maxLevels: MAX_LEVELS,
    maxBonus: MAX_BONUS,
    difficulty: 2,
  });

  window.addEventListener('keydown', (e) => {
    const k = e.key.toLowerCase();
    if (k === 'f') toggleFullscreen();
  });

  // --- Menu d'accueil animé : état + navigation ---
  // 'main' = Jouer/Paramètres/Classement ; 'players' = choix du nb de joueurs.
  const menuState = { mode: 'main' as 'main' | 'modes' | 'players' | 'settings', selIndex: 0 };

  // Réglages (ordre = affichage menu) : 0 difficulté, 1 volume sons,
  // 2 volume musique, 3 ombres, 4 langue, 5 retour.
  const VOL_STEP = 0.1;
  function changeSetting(dir: number): void {
    switch (menuState.selIndex) {
      case 0:
        session.difficulty = (session.difficulty + dir + 3) % 3;
        break;
      case 1:
        audio.setSoundVolume(audio.soundVolume + dir * VOL_STEP);
        break;
      case 2:
        audio.setMusicVolume(audio.musicVolume + dir * VOL_STEP);
        break;
      case 3:
        shadows = !shadows;
        break;
      case 4:
        cycleLang(dir);
        break;
    }
  }

  function tickMenuInput(): void {
    // La condition est réévaluée à chaque tour : si une touche change d'écran
    // (intro→menu→jeu), les touches suivantes sont traitées dans le bon état.
    while (session.screen === 'intro' || session.screen === 'menu') {
      const k = input.takeKey();
      if (!k) break;
      if (session.screen === 'intro') {
        session.toMenu();
        menuState.mode = 'main';
        menuState.selIndex = 0;
        continue;
      }
      if (menuState.mode === 'main') {
        // Menu principal : nombre d'options variable (« Continuer » conditionnel).
        const acts = mainActions();
        const N = acts.length;
        if (k === 'arrowup' || k === 'w') menuState.selIndex = (menuState.selIndex + N - 1) % N;
        else if (k === 'arrowdown' || k === 's') menuState.selIndex = (menuState.selIndex + 1) % N;
        else if (k === 'enter' || k === ' ') acts[Math.min(menuState.selIndex, N - 1)]!();
        else if (k === 'p') {
          menuState.mode = 'settings';
          menuState.selIndex = 0;
        }
      } else if (menuState.mode === 'modes') {
        // Choix du mode : 0 = Classique → joueurs, 1 = Time Attack, 2 = Survie, 3 = Défi.
        if (k === 'arrowup' || k === 'w') menuState.selIndex = (menuState.selIndex + 3) % 4;
        else if (k === 'arrowdown' || k === 's') menuState.selIndex = (menuState.selIndex + 1) % 4;
        else if (k === 'enter' || k === ' ') {
          if (menuState.selIndex === 0) {
            menuState.mode = 'players';
            menuState.selIndex = 0;
          } else if (menuState.selIndex === 1) session.startGame(1, 'timeattack');
          else if (menuState.selIndex === 2) session.startGame(1, 'survival');
          else session.startGame(1, 'daily');
        } else if (k === 'escape') {
          menuState.mode = 'main';
          menuState.selIndex = 0;
        }
      } else if (menuState.mode === 'players') {
        if (k === 'arrowup' || k === 'w') menuState.selIndex = (menuState.selIndex + 3) % 4;
        else if (k === 'arrowdown' || k === 's') menuState.selIndex = (menuState.selIndex + 1) % 4;
        else if (k === 'enter' || k === ' ') session.startGame(menuState.selIndex + 1, 'classic');
        else if (k === 'escape') {
          menuState.mode = 'modes';
          menuState.selIndex = 0; // revient au choix du mode
        }
      } else {
        const N = 6; // nombre de lignes de réglages
        if (k === 'arrowup' || k === 'w') menuState.selIndex = (menuState.selIndex + N - 1) % N;
        else if (k === 'arrowdown' || k === 's') menuState.selIndex = (menuState.selIndex + 1) % N;
        else if (k === 'arrowleft' || k === 'a') changeSetting(-1);
        else if (k === 'arrowright' || k === 'd') changeSetting(1);
        else if (k === 'escape') {
          menuState.mode = 'main';
          menuState.selIndex = 1; // revient sur « Paramètres »
        } else if (k === 'enter' || k === ' ') {
          if (menuState.selIndex === 5) {
            menuState.mode = 'main';
            menuState.selIndex = 1;
          } else changeSetting(1);
        }
      }
    }
  }

  // Menu principal : « Continuer » n'apparaît que si une partie est sauvegardée.
  // mainActions() et mainItems() doivent rester alignés (même ordre/condition).
  function mainActions(): (() => void)[] {
    const acts: (() => void)[] = [];
    if (session.hasSavedGame()) acts.push(() => session.continueGame());
    acts.push(() => {
      menuState.mode = 'modes';
      menuState.selIndex = 0;
    });
    acts.push(() => session.showTools());
    acts.push(() => {
      menuState.mode = 'settings';
      menuState.selIndex = 0;
    });
    acts.push(() => session.showLeaderboard());
    return acts;
  }

  function mainItems(): string[] {
    const items: string[] = [];
    if (session.hasSavedGame()) items.push(t().continueLabel);
    items.push(t().play, t().toolsMenu, t().settings, t().leaderboard);
    return items;
  }

  function menuModel(): MenuModel {
    return {
      prompt: session.screen === 'intro',
      players: menuState.selIndex + 1,
      mode: menuState.mode,
      mainItems: mainItems(),
      selIndex: menuState.selIndex,
      difficulty: session.difficulty,
      soundVol: audio.soundVolume,
      musicVol: audio.musicVolume,
      shadows,
    };
  }

  // Hook de debug (inoffensif) pour la console et les tests headless.
  (window as unknown as Record<string, unknown>).__dp2 = {
    session,
    audio,
    fx: () => pixi.debugFx(), // diagnostic : déclenche un effet « juice » de démo
    // Diagnostic du chargement des sprites de murs.
    diag() {
      const dim = (s: { img: CanvasImageSource } | undefined) => {
        if (!s) return null;
        const im = s.img as HTMLImageElement;
        return { w: im.naturalWidth ?? im.width, h: im.naturalHeight ?? im.height, complete: im.complete };
      };
      return {
        userAgent: navigator.userAgent,
        linesetsCount: sprites.linesets.length,
        line0: dim(sprites.linesets[0]),
        backgrounds: dim(sprites.backgrounds),
        pills: dim(sprites.pills),
        screen: session.screen,
        currentLineSet: session.play?.level.lineSet ?? null,
        shadows,
      };
    },
  };

  const loop = new GameLoop(
    TICK_HZ,
    () => {
      input.pollGamepad();
      tickMenuInput(); // navigation du menu animé (écrans intro/menu)
      session.tick(input);
      director.update(session);
      // Effets visuels + capture des positions pour l'interpolation du rendu,
      // une fois par tick (comme l'audio).
      if (session.play && session.screen === 'playing') {
        pixi.feedEffects(session.play);
        pixi.captureMotion(session.play);
      }
    },
    (alpha, fps) => draw(alpha, fps),
  );
  loop.start();

  // --- Transitions animées entre écrans (fondu au noir entrant) ---
  // On ne fond qu'entre groupes différents : pas entre écrans de gameplay
  // (bascules trop fréquentes), ni au sein du menu (intro↔menu = même visuel).
  const FLOW_SCREENS = new Set(['playing', 'levelIntro', 'paused', 'levelFlash', 'levelClear', 'loading']);
  const MENU_SCREENS = new Set(['intro', 'menu']);
  const sameGroup = (a: string, b: string): boolean =>
    (FLOW_SCREENS.has(a) && FLOW_SCREENS.has(b)) || (MENU_SCREENS.has(a) && MENU_SCREENS.has(b));
  let transition = 0;
  let goTimer = 0; // « GO! » au démarrage du niveau
  let prevDrawScreen: string | null = null;
  let lastDrawMs = 0;

  function draw(alpha: number, fps: number): void {
    const now = performance.now();
    const dtDraw = lastDrawMs ? Math.min(0.05, (now - lastDrawMs) / 1000) : 0.016;
    lastDrawMs = now;
    const screen = session.screen;
    if (screen !== prevDrawScreen) {
      if (prevDrawScreen !== null && !sameGroup(screen, prevDrawScreen)) transition = 1;
      if (prevDrawScreen === 'levelIntro' && screen === 'playing') goTimer = 0.7; // déclenche « GO! »
      prevDrawScreen = screen;
    }
    drawContent(alpha, fps);
    // « GO! » fugace au début du niveau.
    if (goTimer > 0) {
      goTimer = Math.max(0, goTimer - dtDraw);
      if (screen === 'playing') drawGo(renderer.ctx, goTimer);
    }
    // Voile de transition par-dessus tout (overlay #ui, au-dessus du WebGL).
    if (transition > 0) {
      transition = Math.max(0, transition - dtDraw * 3);
      const c = renderer.ctx;
      c.fillStyle = `rgba(0,0,0,${transition.toFixed(3)})`;
      c.fillRect(0, 0, SCREEN_W, HEIGHT);
    }
  }

  function drawContent(alpha: number, fps: number): void {
    const ctx = renderer.ctx;
    const screen = session.screen;

    // Menu d'accueil animé (intro/menu) : rendu WebGL, overlay transparent.
    if (screen === 'intro' || screen === 'menu') {
      pixi.renderMenu(menuModel(), performance.now());
      renderer.clear();
      return;
    }

    const inGame =
      screen === 'playing' ||
      screen === 'levelIntro' ||
      screen === 'paused' ||
      screen === 'levelFlash' ||
      screen === 'levelClear' ||
      screen === 'gameOver';

    // Autres écrans hors-jeu : on masque le rendu WebGL et l'overlay devient opaque.
    if (!inGame) {
      pixi.hide();
      renderer.fill('#000');
      if (screen === 'loading') drawCenteredMessageOn(ctx, t().loadingLevel);
      else if (screen === 'hiscoreEntry') drawHiscoreEntry(ctx, session);
      else if (screen === 'hiscoreTable') drawHiscoreTable(ctx, session);
      else if (screen === 'tools') drawCodex(ctx, sprites, session.toolPage);
      return;
    }

    const play = session.play;
    if (!play) {
      pixi.hide();
      renderer.fill('#000');
      return;
    }

    // Jeu rendu en WebGL (décor + entités + HUD latéral). Pendant le clignotement
    // de fin de niveau : décor sans entités (phase allumée) ou écran noir (éteinte).
    const hud = hudData(play);
    const now = performance.now();
    if (screen === 'levelFlash') {
      if (session.flashOn) pixi.render(play, shadows, hud, now, alpha, false);
      else pixi.hide();
    } else {
      pixi.render(play, shadows, hud, now, alpha, true);
    }

    // Overlay 2D : surimpressions + ligne d'aide discrète.
    renderer.clear();
    drawHelpLine(ctx, fps);
    if (screen === 'levelIntro') drawLevelIntro(ctx, session);
    if (screen === 'gameOver') drawGameOver(ctx);
    if (screen === 'paused') drawPause(ctx);
    if (screen === 'levelClear') drawLevelClear(ctx, session);
  }

  /** Assemble les données du HUD latéral à partir de l'état de jeu + session. */
  function hudData(play: PlayState): HudData {
    const active = play.toolInUse.findIndex((on, i) => on && i > 0);
    const clevel = session.players[session.current]?.clevel ?? 1;
    const ci = comboInfo(play);
    return {
      score: play.score,
      levelLabel: play.bonusLevel > 0 ? t().bonus(play.bonusLevel) : t().level(clevel),
      playerLabel: session.players.length > 1 ? t().player(session.current + 1) : '',
      lives: Math.max(0, play.lives),
      energy: play.energy,
      extra: play.extra,
      activeTool: active > 0 ? active : 0,
      toolFrac: play.toolTimer / (12 * 60),
      combo: play.combo,
      comboMult: ci.mult,
      comboProgress: ci.progress,
      timeLeft: session.mode === 'timeattack' ? session.timeLeft : -1,
    };
  }

  function drawHelpLine(ctx: CanvasRenderingContext2D, fps: number): void {
    ctx.fillStyle = '#9a9ad0';
    ctx.font = `11px ${UI_FONT}`;
    ctx.textAlign = 'center';
    ctx.fillText(t().helpLine(fps.toFixed(0)), SCREEN_W / 2, HEIGHT - 6);
    ctx.textAlign = 'left';
  }
}

function toggleFullscreen(): void {
  if (document.fullscreenElement) void document.exitFullscreen();
  else void document.documentElement.requestFullscreen().catch(() => {});
}

function drawCenteredMessage(msg: string): void {
  renderer.fill('#000');
  drawCenteredMessageOn(renderer.ctx, msg);
}

/** Écran d'aide affiché si aucun contexte graphique ne peut être créé. */
function drawRenderError(): void {
  const ctx = renderer.ctx;
  renderer.fill('#000');
  ctx.textAlign = 'center';
  ctx.fillStyle = '#ff5050';
  ctx.font = `22px ${UI_FONT}`;
  ctx.fillText(t().renderError1, SCREEN_W / 2, HEIGHT / 2 - 18);
  ctx.fillStyle = '#cfd6ff';
  ctx.font = `15px ${UI_FONT}`;
  ctx.fillText(t().renderError2, SCREEN_W / 2, HEIGHT / 2 + 20);
  ctx.textAlign = 'left';
}

function drawCenteredMessageOn(ctx: CanvasRenderingContext2D, msg: string): void {
  ctx.fillStyle = '#fff';
  ctx.font = `16px ${UI_FONT}`;
  ctx.textAlign = 'center';
  ctx.fillText(msg, SCREEN_W / 2, HEIGHT / 2);
  ctx.textAlign = 'left';
}

/** « GO! » fugace (grandit + s'estompe) au tout début d'un niveau. */
function drawGo(ctx: CanvasRenderingContext2D, timer: number): void {
  ctx.save();
  ctx.globalAlpha = Math.min(1, timer * 3);
  ctx.fillStyle = '#39ff14';
  ctx.font = `bold ${Math.round(64 + (0.7 - timer) * 30)}px ${UI_FONT}`;
  ctx.textAlign = 'center';
  ctx.fillText(t().go, SCREEN_W / 2, HEIGHT / 2 + 20);
  ctx.restore();
}

// Réexport pour le typage du hook de debug.
export type { GameSprites };
