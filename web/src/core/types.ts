// Types du domaine, transposés des structures C (dp2_map.h).

/** Coordonnée de cellule sur la grille (MAP en C). */
export interface MapCoord {
  x: number; // 0..MAPX-1
  y: number; // 0..MAPY-1
}

/** Une cellule de la grille (TILE en C). */
export interface Tile {
  /** index d'image dans le jeu de lignes ou de pills courant. */
  tile: number;
  /** la cellule est-elle une pill ? (sinon c'est une ligne/mur). */
  isPill: boolean;
  /** la cellule est-elle une powerpill ? (implique isPill). */
  isPowerpill: boolean;
  /** cellule protégée du dessin (points de spawn en général). */
  isProtected: boolean;
}

/** Un niveau complet (LEVEL en C). map est indexé [y][x]. */
export interface Level {
  mapId: string; // "Pace2"
  mapVer: number; // toujours normalisé à MAP_VER après chargement
  validated: boolean;
  lineSet: number; // jeu de lignes utilisé (0..19)
  player: MapCoord;
  ghosts: [MapCoord, MapCoord, MapCoord, MapCoord];
  pickup: MapCoord;
  teleport: [MapCoord, MapCoord];
  background: number;
  map: Tile[][]; // [MAPY][MAPX]
  pills: number; // nombre de pills + powerpills
}
