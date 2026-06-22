#pragma once
#include "dp2_map.h"

typedef struct PACMAN {
   ALLEGRO_TIMER *timer;
   ALLEGRO_BITMAP *sheet;     // the sprite sheet containing all our pacman images.
   ALLEGRO_BITMAP *bitmap[25];// this will be an array of sub bitmaps taken from sprite sheet.
   int cur_img;               // the current pacman image from above
   int inc;                   // +1 increment and -1 for opening/closing mouth
   int ts;                    // timer speed (ex: 120)
   bool fast;                 // is pacman moving fast
   int size;                  // bitmap width and height (all frames will have the same size)
   MAP map;                   // Current map co-ordinate pacman is in
   int x, y;                  // pacman screen co-ordinates
   int ox, oy;                // old screen co-ordinates
   int dx, dy;                // direction pacman is facing x+/- y+/-
   float rot;                 // pacman rotation
   int flip;                  // pacman flip
   bool dead;                 // is pacman dead
   int img_delay;             // delay between animation images
   int cntr;                  // delay counter (if (cntr%img_delay==0) update anim)
   double r;                  // radius for collision
} PACMAN;
