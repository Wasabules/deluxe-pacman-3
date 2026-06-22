// Outils du jeu, dans le MÊME ordre que l'enum TOOLS du C (dp2_main.h) — cet
// ordre correspond aussi à la disposition de la feuille Tools.png (gauche→droite,
// haut→bas). L'index d'image d'un outil est (valeur - 1) : OFF n'a pas d'image.

export enum Tool {
  OFF,
  RANDOM,
  SHIELDS,
  PRESENT,
  BLUE_DIAMOND,
  PINK_DIAMOND,
  BONUS,
  TIMES2,
  TIMES5,
  TIMES7,
  FREEZE,
  AUTISM,
  PRECIOUS,
  GOLDLEAF,
  DYNAMITE,
  JUMP,
  EXTRA_E,
  EXTRA_X,
  EXTRA_T,
  EXTRA_R,
  EXTRA_A,
  GUN,
  GLUE,
  SPEED,
  TIME,
  SKULL,
  // --- Outils ajoutés (Deluxe Pacman 3) : icônes dessinées en vectoriel ---
  MAGNET, // 26 : attire pills + bonus
  PANIC, // 27 : les fantômes fuient
  PHASE, // 28 : traverse les murs
  BOMB, // 29 : bombe à retardement
  LIGHTNING, // 30 : foudroie le fantôme le plus proche
  WARP, // 31 : téléporte Pacman à une case libre au hasard
}

export const MAX_TOOLS = 31;
export const FIRST_NEW_TOOL = Tool.MAGNET; // outils sans frame dans Tools.png
export const MAX_PICKUPS = 35;

// Durées en frames 60 Hz (converties depuis les ticks 500 Hz du C : ×60/500).
// pickup.wait / tool.wait = REDRAW_TIMER*16 = 16 s entre apparitions.
export const SPAWN_WAIT = 16 * 60; // 960
// screen_time = REDRAW_TIMER*8 = 8 s d'affichage sur la case.
export const SPAWN_SCREEN = 8 * 60; // 480
// TOOL_USE_TIME = REDRAW_TIMER*12 = 12 s d'effet d'un outil persistant.
export const TOOL_USE_FRAMES = 12 * 60; // 720
// Le tool TIME porte la durée d'effet à tool.wait (16 s).
export const TIME_USE_FRAMES = 16 * 60; // 960

// Lettres EXTRA : bit par lettre ; les cinq réunies (31) donnent une vie.
export const EXTRA_ALL = 0b11111;
export function extraBit(tool: Tool): number {
  return 1 << (tool - Tool.EXTRA_E); // E=1, X=2, T=4, R=8, A=16
}

export function isExtraLetter(t: Tool): boolean {
  return t >= Tool.EXTRA_E && t <= Tool.EXTRA_A;
}

/** Outils persistants : restent actifs (tool_inuse) pendant TOOL_USE_FRAMES. */
export const PERSISTENT_TOOLS: ReadonlySet<Tool> = new Set([
  Tool.SHIELDS,
  Tool.PRESENT,
  Tool.BLUE_DIAMOND,
  Tool.PINK_DIAMOND,
  Tool.TIMES2,
  Tool.TIMES5,
  Tool.TIMES7,
  Tool.FREEZE,
  Tool.GUN,
  Tool.GLUE,
  Tool.SPEED,
  Tool.MAGNET,
  Tool.PANIC,
  Tool.PHASE,
]);
