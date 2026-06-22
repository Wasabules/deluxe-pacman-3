#pragma once
#include <allegro5/allegro.h>

#define FULLSCREEN ALLEGRO_FULLSCREEN_WINDOW

typedef struct DISPLAY {
   const char *name;
   ALLEGRO_DISPLAY *screen;
   ALLEGRO_TIMER *redraw_timer;
   ALLEGRO_BITMAP *icon;
   int w;
   int h;
   bool redraw;
   int vsync;
   int fullscreen;
   int frequency;
} DISPLAY;

extern DISPLAY display;
