#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include "a5_error.h"
#include "a5_scale_screen.h"

// Initialize new button variables with: {0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL}
typedef struct BUTTON {
   int x;                  // upper left corner horizontal position
   int y;                  // upper left corner vertical position
   int w;                  // width of button
   int h;                  // height of button
   int state;              // Button state: 0 up, 1 over, 2 down
   int old_state;          // store the old state to see if it changed
   bool is_switch;         // is this a switch (true) or button (false)?
   ALLEGRO_BITMAP *bmp;    // main bitmap with button images
   ALLEGRO_BITMAP *up;     // subbitmap for button up graphic (when it isn't active)
   ALLEGRO_BITMAP *over;   // subbitmap for button that is being hovered over by mouse
   ALLEGRO_BITMAP *down;   // subbitmap for button after it has been clicked
} BUTTON;

bool new_button(BUTTON *b, bool is_switch, const char *file, int x, int y);
bool check_button(BUTTON *b, ALLEGRO_MOUSE_STATE *mouse);
void draw_button(BUTTON *b);
void destroy_button(BUTTON *b);
