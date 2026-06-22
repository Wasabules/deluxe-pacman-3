#ifndef _a5_sprite_h_
#define _a5_sprite_h_

#include <allegro5/allegro.h>

typedef struct a5_sprite
{
   ALLEGRO_BITMAP *img;
   int x;   // horizontal co-ordinate
   int y;   // vertical co-ordinate
   int w;   // img width
   int h;   // ing height
   short int collision;
} a5_sprite;

#endif
