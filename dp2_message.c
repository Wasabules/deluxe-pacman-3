#include "dp2_message.h"

// Creates a message box at the center of the screen with a dark transparent filled box
// and a light solid border and text of the same specified colour.
// Takes printf() style formatting arguements
void dp2_message(ALLEGRO_FONT *font, ALLEGRO_COLOR colour, const char *message, ...)
{
   va_list args;
   char buffer[4096];

   va_start(args, message);
   vsprintf(buffer, message, args);
   va_end(args);

   int text_w = al_get_text_width(font, buffer);
   int text_h = al_get_font_ascent(font);

   int x0 = WIDTH / 2 - text_w / 2 - 12;
   int x1 = WIDTH / 2 + text_w / 2 + 12;
   int y0 = HEIGHT / 2 - text_h / 2 - 7;
   int y1 = HEIGHT / 2 + text_h / 2 + 15;

   al_draw_filled_rounded_rectangle(x0 + 10, y0 + 10, x1 + 10, y1 + 10, 15, 15, al_map_rgba_f(0, 0, 0, .25));
   al_draw_filled_rounded_rectangle(x0, y0, x1, y1, 15, 15, al_map_rgba_f(0, 0, 0, .6));
   al_draw_rounded_rectangle(x0, y0, x1, y1, 15, 15, colour, 2);
   al_draw_textf(font, colour, WIDTH / 2, HEIGHT / 2 - text_h / 2, ALLEGRO_ALIGN_CENTRE, buffer);
}
