// Entités mobiles. Types purs partagés entre logique et rendu.

import type { MapCoord } from './types';

/** Direction désirée / de déplacement. Ordre identique au C : 0=UP 1=DOWN 2=LEFT 3=RIGHT. */
export type Dir = 0 | 1 | 2 | 3;

export interface Ghost {
  index: number; // 0=rouge 1=vert 2=cyan 3=violet
  map: MapCoord; // cellule courante
  spawn: MapCoord; // cellule de réapparition
  x: number; // coin haut-gauche (map.x*32), repère zone de jeu
  y: number;
  ox: number;
  oy: number;
  dx: number;
  dy: number;
  eyes: number; // 0=bas 1=gauche 2=droite 3=haut 4=tout droit
  curImg: number; // frame d'animation
  inc: number;
  animCntr: number;
  dead: boolean; // mangé → yeux qui rentrent au spawn
  scared: boolean; // bleu, vulnérable
  frozen: boolean; // gelé sur place
  stimer: number; // compteur de frayeur écoulé
  stime: number; // durée de frayeur (frames)
  r: number; // rayon de collision
}

export interface Pacman {
  map: MapCoord; // cellule courante
  x: number; // centre du sprite, repère zone de jeu (0..PLAYFIELD_W)
  y: number;
  ox: number; // position précédente (pour détecter le mouvement)
  oy: number;
  dx: number; // direction effective, -1/0/+1
  dy: number;
  rot: number; // rotation du sprite (radians)
  flip: boolean; // miroir horizontal
  curImg: number; // frame d'animation de la bouche (0..3)
  inc: number; // sens d'animation (+1 / -1)
  animCntr: number; // compteur d'animation
  fast: boolean; // mode rapide (CTRL) actif
  dead: boolean;
}
