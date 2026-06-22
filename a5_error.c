// Allegro 5 error message

#include <stdio.h>
#include <allegro5/allegro_native_dialog.h>
#include "a5_error.h"

void a5_error(const char *func, int line, ALLEGRO_DISPLAY *display, const char *message)
{
   char text[256];

   snprintf(text, 256, "%s(%d): %s", func, line, message);
   al_show_native_message_box(display, "Error", "Error", text,
                              NULL, ALLEGRO_MESSAGEBOX_ERROR);
}
