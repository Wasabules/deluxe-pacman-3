// Icônes dessinées (vectoriel Canvas 2D) des outils ajoutés en Deluxe Pacman 3,
// qui n'ont pas de case dans la planche Tools.png. La même fonction sert au codex
// (dessin direct) et à la génération des textures du jeu (HUD / case d'apparition).

import { Tool } from '../core/tools';

/** Outils dessinés ici (pas dans la planche). Ordre = pour la génération de textures. */
export const DRAWN_TOOLS: Tool[] = [Tool.MAGNET, Tool.PANIC, Tool.PHASE, Tool.BOMB, Tool.LIGHTNING, Tool.WARP];

/** Dessine l'icône d'un outil ajouté dans un carré (x, y, s). */
export function drawToolIcon(ctx: CanvasRenderingContext2D, tool: Tool, x: number, y: number, s: number): void {
  const cx = x + s / 2;
  const cy = y + s / 2;
  ctx.save();
  ctx.beginPath();
  ctx.roundRect(x + 2, y + 2, s - 4, s - 4, s * 0.18);
  ctx.fillStyle = '#161a2e';
  ctx.fill();
  ctx.strokeStyle = '#2c3766';
  ctx.lineWidth = 1.5;
  ctx.stroke();
  ctx.clip();
  ctx.lineJoin = 'round';
  ctx.lineCap = 'round';

  switch (tool) {
    case Tool.MAGNET:
      magnet(ctx, cx, cy, s);
      break;
    case Tool.PANIC:
      panic(ctx, cx, cy, s);
      break;
    case Tool.PHASE:
      phase(ctx, cx, cy, s);
      break;
    case Tool.BOMB:
      bomb(ctx, cx, cy, s);
      break;
    case Tool.LIGHTNING:
      lightning(ctx, cx, cy, s);
      break;
    case Tool.WARP:
      warp(ctx, cx, cy, s);
      break;
  }
  ctx.restore();
}

function magnet(ctx: CanvasRenderingContext2D, cx: number, cy: number, s: number): void {
  const r = s * 0.24;
  const w = s * 0.16;
  ctx.lineWidth = w;
  ctx.strokeStyle = '#e23b3b';
  ctx.beginPath();
  ctx.arc(cx, cy - s * 0.04, r, Math.PI, 0); // demi-anneau (haut du fer à cheval)
  ctx.stroke();
  // Deux branches descendantes.
  for (const sx of [-1, 1]) {
    ctx.beginPath();
    ctx.moveTo(cx + sx * r, cy - s * 0.04);
    ctx.lineTo(cx + sx * r, cy + s * 0.22);
    ctx.stroke();
  }
  // Pôles (embouts clairs).
  ctx.fillStyle = '#e7ecf5';
  for (const sx of [-1, 1]) ctx.fillRect(cx + sx * r - w / 2, cy + s * 0.18, w, s * 0.08);
}

function panic(ctx: CanvasRenderingContext2D, cx: number, cy: number, s: number): void {
  // Point d'exclamation dans un triangle d'alerte.
  ctx.fillStyle = '#ffcf33';
  ctx.beginPath();
  ctx.moveTo(cx, cy - s * 0.3);
  ctx.lineTo(cx + s * 0.32, cy + s * 0.26);
  ctx.lineTo(cx - s * 0.32, cy + s * 0.26);
  ctx.closePath();
  ctx.fill();
  ctx.fillStyle = '#161a2e';
  ctx.fillRect(cx - s * 0.04, cy - s * 0.12, s * 0.08, s * 0.22);
  ctx.beginPath();
  ctx.arc(cx, cy + s * 0.17, s * 0.05, 0, Math.PI * 2);
  ctx.fill();
}

function phase(ctx: CanvasRenderingContext2D, cx: number, cy: number, s: number): void {
  // Pacman « fantôme » : disque jaune semi-transparent + contour pointillé + bouche.
  const r = s * 0.3;
  ctx.globalAlpha = 0.55;
  ctx.fillStyle = '#ffe000';
  ctx.beginPath();
  ctx.moveTo(cx, cy);
  ctx.arc(cx, cy, r, Math.PI * 0.22, Math.PI * 1.78);
  ctx.closePath();
  ctx.fill();
  ctx.globalAlpha = 1;
  ctx.setLineDash([s * 0.08, s * 0.06]);
  ctx.lineWidth = 2;
  ctx.strokeStyle = '#fff6a0';
  ctx.beginPath();
  ctx.arc(cx, cy, r, 0, Math.PI * 2);
  ctx.stroke();
  ctx.setLineDash([]);
}

function bomb(ctx: CanvasRenderingContext2D, cx: number, cy: number, s: number): void {
  const r = s * 0.26;
  ctx.fillStyle = '#1c1c22';
  ctx.beginPath();
  ctx.arc(cx, cy + s * 0.06, r, 0, Math.PI * 2);
  ctx.fill();
  ctx.strokeStyle = '#3a3a48';
  ctx.lineWidth = 1.5;
  ctx.stroke();
  // Reflet.
  ctx.fillStyle = '#4a4a5a';
  ctx.beginPath();
  ctx.arc(cx - r * 0.35, cy - r * 0.1, r * 0.22, 0, Math.PI * 2);
  ctx.fill();
  // Mèche + étincelle.
  ctx.strokeStyle = '#caa15a';
  ctx.lineWidth = s * 0.05;
  ctx.beginPath();
  ctx.moveTo(cx + r * 0.5, cy - r * 0.6);
  ctx.quadraticCurveTo(cx + r * 1.1, cy - r * 1.1, cx + r * 0.7, cy - r * 1.4);
  ctx.stroke();
  ctx.fillStyle = '#ff8a00';
  ctx.beginPath();
  ctx.arc(cx + r * 0.7, cy - r * 1.5, s * 0.07, 0, Math.PI * 2);
  ctx.fill();
  ctx.fillStyle = '#ffe000';
  ctx.beginPath();
  ctx.arc(cx + r * 0.7, cy - r * 1.5, s * 0.035, 0, Math.PI * 2);
  ctx.fill();
}

function lightning(ctx: CanvasRenderingContext2D, cx: number, cy: number, s: number): void {
  ctx.fillStyle = '#ffe11a';
  ctx.beginPath();
  ctx.moveTo(cx + s * 0.06, cy - s * 0.32);
  ctx.lineTo(cx - s * 0.2, cy + s * 0.04);
  ctx.lineTo(cx - s * 0.02, cy + s * 0.04);
  ctx.lineTo(cx - s * 0.12, cy + s * 0.32);
  ctx.lineTo(cx + s * 0.22, cy - s * 0.06);
  ctx.lineTo(cx + s * 0.02, cy - s * 0.06);
  ctx.closePath();
  ctx.fill();
}

function warp(ctx: CanvasRenderingContext2D, cx: number, cy: number, s: number): void {
  // Spirale / vortex.
  ctx.strokeStyle = '#00e0ff';
  ctx.lineWidth = s * 0.07;
  ctx.beginPath();
  const turns = 2.4;
  const steps = 60;
  for (let i = 0; i <= steps; i++) {
    const tt = i / steps;
    const ang = tt * turns * Math.PI * 2;
    const rad = tt * s * 0.32;
    const px = cx + Math.cos(ang) * rad;
    const py = cy + Math.sin(ang) * rad;
    if (i === 0) ctx.moveTo(px, py);
    else ctx.lineTo(px, py);
  }
  ctx.stroke();
  ctx.fillStyle = '#c77bff';
  ctx.beginPath();
  ctx.arc(cx, cy, s * 0.06, 0, Math.PI * 2);
  ctx.fill();
}
