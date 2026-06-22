#pragma once
#include <allegro5/allegro.h>

extern float scale_x;
extern float scale_y;
extern int offset_x;
extern int offset_y;

// usage: a5_scale_screen(BUFFER_WIDTH, BUFFER_HEIGHT, display_width, display_height);
void a5_scale_screen(int bw, int bh, int dw, int dh);
