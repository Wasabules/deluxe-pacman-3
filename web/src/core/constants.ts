// Constantes reprises du source C original (dp2_main.h et dp2_map.h).
// Le `core` est volontairement sans dépendance navigateur : il reste testable.

// --- Résolution logique fixe (dp2_main.h) ---
export const WIDTH = 800; // largeur de la zone de jeu (labyrinthe + marges internes)
export const HEIGHT = 600;

// Écran 16:9 : la zone de jeu (WIDTH) est centrée, et les deux bords latéraux
// (SIDE_W chacun) accueillent le HUD façon synthwave.
export const SIDE_W = 133;
export const SCREEN_W = WIDTH + 2 * SIDE_W; // 1066 ≈ 16:9 à 600 de haut
export const GAME_OFFSET_X = SIDE_W; // décalage horizontal du jeu centré

// Cadence de simulation cible. Le jeu original fait avancer pacman et les
// fantômes via des timers à TIMER_SPEED (60 Hz). On reproduit cette cadence
// avec une boucle à pas de temps FIXE pour garder un gameplay « à l'identique ».
export const TICK_HZ = 60; // TIMER_SPEED

// Horloge interne du jeu original (dp2_main.h : REDRAW_TIMER), à 500 Hz, qui sert
// de base de temps aux délais d'animation et au minuteur de frayeur des fantômes.
export const REDRAW_TIMER = 500;

// --- Grille de jeu (dp2_map.h) ---
export const MAPX = 23; // largeur en cellules
export const MAPY = 17; // hauteur en cellules
export const TILE_SIZE = 32; // taille d'une cellule en pixels
export const PLAYFIELD_W = MAPX * TILE_SIZE; // 736
export const PLAYFIELD_H = MAPY * TILE_SIZE; // 544
// Décalage de la zone de jeu sur l'écran (le levelmap 736x544 est blitté ici,
// le reste forme le HUD : bandeau haut, marges, barre d'énergie en bas).
export const PLAYFIELD_X = 32;
export const PLAYFIELD_Y = 32;
// Décalage de l'ombre portée des murs (dp2_main.h : SHADOW).
export const SHADOW = 6;

// --- Format de niveau (dp2_map.h) ---
export const MAP_ID = 'Pace2'; // identifie un niveau Deluxe Pacman 2
export const MAP_VER = 3; // 1 & 2 = sans téléports, 3 = téléports + is_protected

// --- Sprites et déplacement (dp2_main.h) ---
export const SPRITE_SIZE = 50;
export const MOVE_PIXELS = 4;

// --- Règles (dp2_main.h) ---
export const MAX_LEVELS = 50;
export const MAX_LIVES = 5;
export const MAX_ENERGY = 500;
export const NEW_LIFE_SCORE = 200000;
export const MAX_BONUS = 4;

// Valeur sentinelle d'un téléport non défini (cf. loadmap pour les vieux niveaux).
export const TELEPORT_UNSET = 99;
