#pragma once
#include <allegro5/allegro.h>

// Note: Editor screen is larger than game
#define WIDTH    1024
#define HEIGHT   768

typedef enum PLACEMODE {GHOST0, GHOST1, GHOST2, GHOST3, LINE, PILL, POWERPILL, TELEPORT, PLAYER, PICKUP} PLACEMODE;

typedef struct EDITOR {
   int x;               // tile x clicked on
   int y;               // tile y clicked on
   int oldx;            // old tile x (used for tracking)
   int oldy;            // old tile y (used for tracking)
   int wheel;           // store old mouse wheel value
   unsigned char pillx;
   unsigned char pilly;
   unsigned char pill;   // pill = pill x + pill y * 8;
   bool saved;          // is map saved yet?
   bool grid;           // display grid?
   PLACEMODE mode;      // what are we placing on the map
} EDITOR;
