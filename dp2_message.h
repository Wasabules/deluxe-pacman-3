#ifndef _dp2_message_h_
#define _dp2_message_h_

#include <stdio.h>
#include <stdarg.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include "dp2_main.h"

static void dp2_message(ALLEGRO_FONT *font, ALLEGRO_COLOR colour, const char *message, ...);

#endif //_dp2_message_h_
