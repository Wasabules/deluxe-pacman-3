#pragma once
#include <allegro5\allegro.h>
#include "dp2_map.h"

typedef struct GHOST {
   ALLEGRO_TIMER *timer;      // for controlling movement speed, only the
   // first ghost[0] will be used for now
   ALLEGRO_BITMAP *bitmap[5]; // images that make up the ghost animation
   // BLUE ghosts will be in a separate variable
   int ts;                    // movement speed for timer
   MAP map;                   // current map cell ghost is in
   int x, y;                  // current screen co-ordinates
   MAP spawn;                 // ghost map spawn location
   int ox, oy;                // old screen co-ordinates
   int dx, dy;                // direction ghost is travelling in
   int cur_img;               // current image
   bool dead;                 // ghost dead flag
   int img_delay;             // delay between animation images
   int cntr;                  // delay counter for animation
   bool scared;               // ghost scared (blue) flag
   int inc;                   // animation direction (+1 or -1)
   int stimer;                // scared timer
   int stime;                 // how long to stay scared
   int scntr;                 // scared counter
   int eyes;                  // direction eyes are pointing
   bool frozen;               // is ghost frozen?  TRUE or FALSE
   int size;                  // bitmap width and height (all frames will have the same size)
   double r;                     // radius for collision
} GHOST;
