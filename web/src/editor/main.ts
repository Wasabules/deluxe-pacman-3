import './editor.css';
import { loadGameSprites, type GameSprites } from '../platform/assets';
import { decodeLevel, encodeLevel } from '../core/level';
import { MAPX, MAPY, TILE_SIZE, TELEPORT_UNSET } from '../core/constants';
import type { Level } from '../core/types';
import {
  initEmptyLevel,
  placeLine,
  removeTile,
  placePill,
  placeSpawn,
  pillFill,
  syncPills,
  validate,
  recomputeProtected,
  type SpawnKind,
  type ValidationResult,
} from './editorState';

type Tool =
  | { kind: 'wall' }
  | { kind: 'erase' }
  | { kind: 'pill'; power: boolean }
  | { kind: 'spawn'; spawn: SpawnKind };

const GHOST_TINT = ['#ff8080', '#80ff80', '#80ffff', '#ff80ff'];

void main();

async function main(): Promise<void> {
  const canvas = document.getElementById('editor') as HTMLCanvasElement;
  const ctx = canvas.getContext('2d')!;
  ctx.imageSmoothingEnabled = false;
  const statusEl = document.getElementById('status')!;
  const toolbar = document.getElementById('toolbar')!;

  const sprites = await loadGameSprites();

  let level = initEmptyLevel();
  let tool: Tool = { kind: 'wall' };
  let pillIndex = 0; // index du sprite de pill (0..39)
  let drawing = false;
  let lastX = -1;
  let lastY = -1;
  let validation: ValidationResult | null = null;

  // --- Barre d'outils ---
  const buttons: { el: HTMLButtonElement; isActive: () => boolean }[] = [];
  function toolBtn(label: string, set: () => void, isActive: () => boolean): void {
    const el = document.createElement('button');
    el.textContent = label;
    el.onclick = () => {
      set();
      refreshButtons();
    };
    toolbar.appendChild(el);
    buttons.push({ el, isActive });
  }
  function actionBtn(label: string, fn: () => void): void {
    const el = document.createElement('button');
    el.textContent = label;
    el.onclick = fn;
    toolbar.appendChild(el);
  }
  function sep(): void {
    const s = document.createElement('div');
    s.className = 'sep';
    toolbar.appendChild(s);
  }
  function refreshButtons(): void {
    for (const b of buttons) b.el.classList.toggle('active', b.isActive());
  }

  toolBtn('Mur', () => (tool = { kind: 'wall' }), () => tool.kind === 'wall');
  toolBtn('Gomme', () => (tool = { kind: 'erase' }), () => tool.kind === 'erase');
  toolBtn('Pill', () => (tool = { kind: 'pill', power: false }), () => tool.kind === 'pill' && !tool.power);
  toolBtn('Powerpill', () => (tool = { kind: 'pill', power: true }), () => tool.kind === 'pill' && tool.power);
  sep();
  toolBtn('Pacman', () => (tool = { kind: 'spawn', spawn: 'player' }), () => tool.kind === 'spawn' && tool.spawn === 'player');
  for (let i = 0; i < 4; i++) {
    const sp = `ghost${i}` as SpawnKind;
    toolBtn(`Fantôme ${i + 1}`, () => (tool = { kind: 'spawn', spawn: sp }), () => tool.kind === 'spawn' && tool.spawn === sp);
  }
  toolBtn('Pickup', () => (tool = { kind: 'spawn', spawn: 'pickup' }), () => tool.kind === 'spawn' && tool.spawn === 'pickup');
  toolBtn('Téléport', () => (tool = { kind: 'spawn', spawn: 'teleport' }), () => tool.kind === 'spawn' && tool.spawn === 'teleport');
  sep();
  actionBtn('Remplir pills', () => {
    pillFill(level, pillIndex);
    syncPills(level);
  });
  actionBtn('Valider', () => {
    validation = validate(level);
    setStatus(validation.message, validation.valid);
  });
  actionBtn('Nouveau', () => {
    if (confirm('Effacer le niveau courant ?')) {
      level = initEmptyLevel();
      validation = null;
      setStatus('Nouveau niveau');
    }
  });
  sep();
  actionBtn('Décor ◀', () => (level.background = (level.background + 19) % 20));
  actionBtn('Décor ▶', () => (level.background = (level.background + 1) % 20));
  actionBtn('Lignes ◀', () => (level.lineSet = (level.lineSet + sprites.linesets.length - 1) % sprites.linesets.length));
  actionBtn('Lignes ▶', () => (level.lineSet = (level.lineSet + 1) % sprites.linesets.length));
  sep();
  actionBtn('Importer', () => fileInput.click());
  actionBtn('Exporter .dp2', () => exportDp2());
  actionBtn('Exporter JSON', () => exportJson());
  refreshButtons();

  // Import de fichier.
  const fileInput = document.createElement('input');
  fileInput.type = 'file';
  fileInput.accept = '.dp2,.json';
  fileInput.style.display = 'none';
  fileInput.onchange = () => void importFile();
  document.body.appendChild(fileInput);

  async function importFile(): Promise<void> {
    const f = fileInput.files?.[0];
    if (!f) return;
    try {
      if (f.name.endsWith('.json')) {
        level = JSON.parse(await f.text()) as Level;
      } else {
        level = decodeLevel(await f.arrayBuffer());
      }
      recomputeProtected(level);
      validation = null;
      setStatus(`Importé : ${f.name}`);
    } catch (e) {
      setStatus(`Échec import : ${(e as Error).message}`, false);
    }
    fileInput.value = '';
  }

  function download(blob: Blob, name: string): void {
    const a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = name;
    a.click();
    URL.revokeObjectURL(a.href);
  }
  function exportDp2(): void {
    syncPills(level);
    const bytes = encodeLevel(level);
    download(new Blob([bytes.buffer as ArrayBuffer], { type: 'application/octet-stream' }), 'level.dp2');
  }
  function exportJson(): void {
    syncPills(level);
    download(new Blob([JSON.stringify(level, null, 2)], { type: 'application/json' }), 'level.json');
  }

  // --- Souris ---
  function cellAt(e: PointerEvent): { x: number; y: number } {
    const r = canvas.getBoundingClientRect();
    const x = Math.floor(((e.clientX - r.left) / r.width) * MAPX);
    const y = Math.floor(((e.clientY - r.top) / r.height) * MAPY);
    return { x: Math.max(0, Math.min(MAPX - 1, x)), y: Math.max(0, Math.min(MAPY - 1, y)) };
  }
  function apply(x: number, y: number, first: boolean): void {
    switch (tool.kind) {
      case 'wall':
        placeLine(level, x, y, first ? -1 : lastX, first ? -1 : lastY);
        break;
      case 'erase':
        removeTile(level, x, y);
        break;
      case 'pill':
        placePill(level, x, y, pillIndex, tool.power);
        break;
      case 'spawn':
        placeSpawn(level, tool.spawn, x, y);
        break;
    }
    syncPills(level);
  }
  canvas.addEventListener('pointerdown', (e) => {
    drawing = true;
    canvas.setPointerCapture(e.pointerId);
    const { x, y } = cellAt(e);
    apply(x, y, true);
    lastX = x;
    lastY = y;
  });
  canvas.addEventListener('pointermove', (e) => {
    if (!drawing) return;
    const { x, y } = cellAt(e);
    if (x === lastX && y === lastY) return;
    apply(x, y, false);
    lastX = x;
    lastY = y;
  });
  const stop = () => {
    drawing = false;
    lastX = lastY = -1;
  };
  canvas.addEventListener('pointerup', stop);
  canvas.addEventListener('pointercancel', stop);

  function setStatus(msg: string, ok?: boolean): void {
    statusEl.textContent = msg;
    statusEl.className = ok === undefined ? '' : ok ? 'ok' : 'err';
  }
  setStatus('Outil : Mur — dessinez les murs au clic-glisser');

  // Hook de debug : charger un niveau existant (console / tests headless).
  (window as unknown as Record<string, unknown>).__editor = {
    async importUrl(url: string) {
      const buf = await (await fetch(url)).arrayBuffer();
      level = decodeLevel(buf);
      recomputeProtected(level);
      validation = validate(level);
      setStatus(validation.message, validation.valid);
    },
    get level() {
      return level;
    },
  };

  // --- Rendu ---
  requestAnimationFrame(function frame() {
    render(ctx, level, sprites, validation);
    setStatusLine();
    requestAnimationFrame(frame);
  });

  function setStatusLine(): void {
    if (validation) return; // garde le message de validation
    const t = tool.kind === 'spawn' ? tool.spawn : tool.kind === 'pill' ? (tool.power ? 'powerpill' : 'pill') : tool.kind;
    statusEl.textContent = `Outil : ${t} · lignes ${level.lineSet} · décor ${level.background} · pills ${level.pills}`;
  }
}

function render(ctx: CanvasRenderingContext2D, level: Level, sprites: GameSprites, validation: ValidationResult | null): void {
  // Fond tuilé.
  for (let y = 0; y < MAPY; y++) {
    for (let x = 0; x < MAPX; x++) sprites.backgrounds.draw(ctx, level.background, x * TILE_SIZE, y * TILE_SIZE);
  }

  // Murs et pills.
  const ls = sprites.linesets[level.lineSet] ?? sprites.linesets[0]!;
  for (let y = 0; y < MAPY; y++) {
    for (let x = 0; x < MAPX; x++) {
      const c = level.map[y][x]!;
      if (!c.tile) continue;
      if (c.isPill) {
        sprites.pills.draw(ctx, c.tile - 1, x * TILE_SIZE, y * TILE_SIZE);
        if (c.isPowerpill) {
          ctx.strokeStyle = '#ff4040';
          ctx.lineWidth = 2;
          ctx.strokeRect(x * TILE_SIZE + 4, y * TILE_SIZE + 4, TILE_SIZE - 8, TILE_SIZE - 8);
        }
      } else {
        ls.draw(ctx, c.tile - 1, x * TILE_SIZE, y * TILE_SIZE);
      }
    }
  }

  // Spawns.
  for (let i = 0; i < 4; i++) {
    const g = level.ghosts[i]!;
    if (g.x < TELEPORT_UNSET) drawTinted(ctx, sprites.ghostSpawn, GHOST_TINT[i]!, g.x * TILE_SIZE, g.y * TILE_SIZE);
  }
  if (level.pickup.x < TELEPORT_UNSET) drawTinted(ctx, sprites.pickupSpawn, '#bfbfbf', level.pickup.x * TILE_SIZE, level.pickup.y * TILE_SIZE);
  for (const t of level.teleport) {
    if (t.x < TELEPORT_UNSET) ctx.drawImage(sprites.teleport, t.x * TILE_SIZE, t.y * TILE_SIZE);
  }
  if (level.player.x < TELEPORT_UNSET) sprites.pacman.draw(ctx, 1, level.player.x * TILE_SIZE - 9, level.player.y * TILE_SIZE - 9);

  // Grille.
  ctx.strokeStyle = 'rgba(255,255,255,0.08)';
  ctx.lineWidth = 1;
  for (let x = 0; x <= MAPX; x++) {
    ctx.beginPath();
    ctx.moveTo(x * TILE_SIZE + 0.5, 0);
    ctx.lineTo(x * TILE_SIZE + 0.5, MAPY * TILE_SIZE);
    ctx.stroke();
  }
  for (let y = 0; y <= MAPY; y++) {
    ctx.beginPath();
    ctx.moveTo(0, y * TILE_SIZE + 0.5);
    ctx.lineTo(MAPX * TILE_SIZE, y * TILE_SIZE + 0.5);
    ctx.stroke();
  }

  // Erreurs de validation.
  if (validation && !validation.valid) {
    ctx.fillStyle = 'rgba(255,0,0,0.4)';
    for (let y = 0; y < MAPY; y++) {
      for (let x = 0; x < MAPX; x++) {
        if (validation.errors[y]?.[x]) ctx.fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
      }
    }
  }
}

function drawTinted(ctx: CanvasRenderingContext2D, img: HTMLImageElement, css: string, dx: number, dy: number): void {
  const w = img.naturalWidth || TILE_SIZE;
  const h = img.naturalHeight || TILE_SIZE;
  const c = document.createElement('canvas');
  c.width = w;
  c.height = h;
  const cx = c.getContext('2d')!;
  cx.drawImage(img, 0, 0);
  cx.globalCompositeOperation = 'multiply';
  cx.fillStyle = css;
  cx.fillRect(0, 0, w, h);
  cx.globalCompositeOperation = 'destination-in';
  cx.drawImage(img, 0, 0);
  ctx.drawImage(c, dx, dy);
}
