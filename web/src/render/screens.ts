// Rendu des écrans hors-jeu 2D superposés (fin de niveau, game over, hall of
// fame). L'intro et le menu sont rendus en WebGL (voir render/pixi/menuScene).

// Centrés sur l'écran 16:9 complet (SCREEN_W).
import { SCREEN_W as WIDTH, HEIGHT } from '../core/constants';
import { t } from '../i18n';
import { CODEX, CODEX_PER_PAGE, codexPageCount, codexName, codexDesc } from './codex';
import { drawToolIcon } from './toolIcons';
import type { Session } from '../app/session';
import type { GameSprites } from '../platform/assets';

const PLANCHE_TOOLS = 25; // au-delà : outils dessinés (icônes vectorielles)

// Police lisible commune (Orbitron, chargée par PixiGame ; fallback monospace).
export const UI_FONT = 'menuFont, monospace';

function center(ctx: CanvasRenderingContext2D, text: string, y: number, size = 24, color = '#fff'): void {
  ctx.fillStyle = color;
  ctx.font = `${size}px ${UI_FONT}`;
  ctx.textAlign = 'center';
  ctx.fillText(text, WIDTH / 2, y);
  ctx.textAlign = 'left';
}

export function drawLevelIntro(ctx: CanvasRenderingContext2D, session: Session): void {
  ctx.fillStyle = 'rgba(0,0,0,0.6)';
  ctx.fillRect(0, HEIGHT / 2 - 52, WIDTH, 112);
  // Construit ici (et non figé dans la session) pour suivre la langue courante.
  const slot = session.players[session.current];
  const what = slot && slot.bonusLevel > 0 ? t().bonus(slot.bonusLevel) : t().level(slot?.clevel ?? 1);
  const msg = session.players.length > 1 ? `${t().player(session.current + 1)} — ${what}` : what;
  center(ctx, msg, HEIGHT / 2 - 8, 32, '#ffeb3b');
  center(ctx, t().ready, HEIGHT / 2 + 34, 26, '#3fd0ff');
}

/** Voile de pause par-dessus le jeu figé. */
export function drawPause(ctx: CanvasRenderingContext2D): void {
  ctx.fillStyle = 'rgba(0,0,0,0.55)';
  ctx.fillRect(0, 0, WIDTH, HEIGHT);
  center(ctx, t().paused, HEIGHT / 2 - 18, 56, '#ffe600');
  center(ctx, t().resumeHint, HEIGHT / 2 + 38, 18, '#cfd6ff');
  center(ctx, t().quitHint, HEIGHT / 2 + 66, 16, '#9a9ad0');
}

/** Récap de fin de niveau : détail des bonus + total. */
export function drawLevelClear(ctx: CanvasRenderingContext2D, session: Session): void {
  ctx.fillStyle = 'rgba(0,0,0,0.72)';
  ctx.fillRect(0, 0, WIDTH, HEIGHT);
  center(ctx, session.clearBonus ? t().bonusCleared : t().levelCleared, 128, 42, '#ffeb3b');

  let y = 214;
  const row = (label: string, val: number, color = '#cfd6ff'): void => {
    if (val <= 0) return;
    ctx.textAlign = 'left';
    ctx.fillStyle = color;
    ctx.font = `20px ${UI_FONT}`;
    ctx.fillText(label, WIDTH / 2 - 200, y);
    ctx.textAlign = 'right';
    ctx.fillStyle = '#fff';
    ctx.fillText(`+${val.toLocaleString('en-US')}`, WIDTH / 2 + 200, y);
    y += 36;
  };
  row(t().clearBaseLabel, session.clearBase);
  row(t().clearPerfectLabel, session.clearPerfect, '#39ff14');
  row(t().clearSpeedLabel, session.clearSpeed, '#00f0ff');
  row(t().clearLivesLabel, session.clearLives, '#ff2bd6');

  y += 12;
  ctx.textAlign = 'left';
  ctx.fillStyle = '#3fd0ff';
  ctx.font = `bold 26px ${UI_FONT}`;
  ctx.fillText(t().clearTotalLabel, WIDTH / 2 - 200, y);
  ctx.textAlign = 'right';
  ctx.fillStyle = '#ffe600';
  ctx.fillText(`+${session.clearGain.toLocaleString('en-US')}`, WIDTH / 2 + 200, y);
  ctx.textAlign = 'left';
}

export function drawGameOver(ctx: CanvasRenderingContext2D): void {
  ctx.fillStyle = 'rgba(0,0,0,0.7)';
  ctx.fillRect(0, 0, WIDTH, HEIGHT);
  center(ctx, t().gameOver, HEIGHT / 2, 48, '#ff5050');
}

export function drawHiscoreEntry(ctx: CanvasRenderingContext2D, session: Session): void {
  ctx.fillStyle = '#000';
  ctx.fillRect(0, 0, WIDTH, HEIGHT);
  center(ctx, t().newScore, 140, 36, '#ffeb3b');
  if (session.players.length > 1) center(ctx, t().playerLabel(session.hiscorePlayer + 1), 190, 20, '#fff');
  center(ctx, t().points(session.hiscoreScore), 230, 22, '#fff');
  center(ctx, t().enterName, 300, 20, '#9a9ad0');
  center(ctx, `${session.nameInput}_`, 340, 30, '#3fd0ff');
  center(ctx, t().validate, HEIGHT - 50, 16, '#9a9ad0');
}

export function drawHiscoreTable(ctx: CanvasRenderingContext2D, session: Session): void {
  ctx.fillStyle = '#000';
  ctx.fillRect(0, 0, WIDTH, HEIGHT);
  center(ctx, t().hallOfFame, 90, 40, '#ffeb3b');
  center(ctx, t().difficultyLine(t().diffName(session.difficulty)), 130, 16, '#9a9ad0');
  ctx.font = `22px ${UI_FONT}`;
  session.hiscoreTableData.forEach((e, i) => {
    const y = 200 + i * 50;
    ctx.textAlign = 'left';
    ctx.fillStyle = '#fff';
    ctx.fillText(`${i + 1}.`, 180, y);
    ctx.fillText(e.name, 230, y);
    ctx.textAlign = 'right';
    ctx.fillText(`${e.score}`, WIDTH - 220, y);
    ctx.textAlign = 'left';
    ctx.fillStyle = '#888';
    ctx.fillText(t().lvlShort(e.level), WIDTH - 200, y);
  });
  center(ctx, t().pressKey, HEIGHT - 40, 16, '#9a9ad0');
}

// --- Codex des outils ---

function wrapText(ctx: CanvasRenderingContext2D, text: string, x: number, y: number, maxW: number, lineH: number): void {
  let line = '';
  let yy = y;
  for (const word of text.split(' ')) {
    const test = line ? `${line} ${word}` : word;
    if (line && ctx.measureText(test).width > maxW) {
      ctx.fillText(line, x, yy);
      line = word;
      yy += lineH;
    } else {
      line = test;
    }
  }
  if (line) ctx.fillText(line, x, yy);
}

export function drawCodex(ctx: CanvasRenderingContext2D, sprites: GameSprites, page: number): void {
  ctx.fillStyle = '#000';
  ctx.fillRect(0, 0, WIDTH, HEIGHT);
  center(ctx, t().toolsTitle, 46, 32, '#ffeb3b');

  const colW = WIDTH / 2;
  CODEX.slice(page * CODEX_PER_PAGE, page * CODEX_PER_PAGE + CODEX_PER_PAGE).forEach((e, i) => {
    const x = i < 6 ? 40 : colW + 20;
    const y = 84 + (i % 6) * 78;
    if (e.toolImg >= PLANCHE_TOOLS) drawToolIcon(ctx, e.toolImg + 1, x, y, 50);
    else if (e.toolImg >= 0) sprites.tools.draw(ctx, e.toolImg, x, y);
    else if (e.pickupImg !== undefined) sprites.pickups.draw(ctx, e.pickupImg, x, y);
    ctx.textAlign = 'left';
    ctx.fillStyle = '#3fd0ff';
    ctx.font = `bold 18px ${UI_FONT}`;
    ctx.fillText(codexName(e), x + 60, y + 20);
    ctx.fillStyle = '#cfd6ff';
    ctx.font = `13px ${UI_FONT}`;
    wrapText(ctx, codexDesc(e), x + 60, y + 42, colW - 100, 16);
  });

  center(ctx, `${t().toolsHint}    ${page + 1}/${codexPageCount()}`, HEIGHT - 26, 15, '#9a9ad0');
}
