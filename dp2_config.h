#pragma once
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_physfs.h>
#include "a5_error.h"

#define TEXT_BUFFER  4096

typedef struct SETTING {
   const char *name;                // game name (for window)
   ALLEGRO_DISPLAY *screen;         // main screen
   ALLEGRO_TIMER *redraw_timer;     // timer for updating screen
   ALLEGRO_BITMAP *icon;            // game window icon
   int w;                           // screen height
   int h;                           // screen width
   bool redraw;                     // redraw screen flag
   int vsync;                       // vsync switch (0 = driver, 1 = on, 2 = off)
   int fullscreen;                  // fullscreen mode switch (0 windowed, 1 fswindow, 2 fullscreen)
   int frequency;                   // monitor frequency
   bool sound_on;                   // sound on/off switch
   bool music_on;                   // music on/off switch
   float sound_volume;              // sound volume (0.0 - 1.0, saved as 0 - 10 and converted at load time)
   float music_volume;              // music volume (0.0 - 1.0, saved as 0 - 10 and converted at load time)
   bool cheat_mode;                 // cheat mode (aka: programmer's test mode) on/off switch
   bool shadows;                    // draw shadows?
   float pan;                       // pan (-1.0 to 1.0)
   bool opengl;                     // use OpenGL on/off switch
   int difficulty;                  // difficulty (0 = easy, 1 = medium, 2 = hard)
   int suicide;                     // suicide mode (0 = normal speed, 1 = double speed)
   bool user_levels;                // load user made levels on/off switch
   bool pakman;                     // Set this to true if you plan to alter the .pak file
} SETTING;

extern SETTING setting;

int option(ALLEGRO_CONFIG *config, char const *name, int v);
void load_config(SETTING *setting);
void save_config(SETTING *setting);
