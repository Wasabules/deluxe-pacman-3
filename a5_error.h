#pragma once
// usage: a5_error(AT, display, "your message here");
#define AT __func__,__LINE__
void a5_error(const char *func, int line, ALLEGRO_DISPLAY *display, const char *message);

