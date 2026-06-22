#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_physfs.h>

#include "dp2_map.h"
#include "p2_main.h"

void initialize_map(LEVEL *level);  // initializes all the LEVEL data to default values
bool savemap(ALLEGRO_DISPLAY *display, LEVEL *level, char *levelname);  // returns true if saved properly
bool loadmap(ALLEGRO_DISPLAY *display, LEVEL *level, char *levelname);  // returns true if loaded properly
bool validate(char message[256], bool error[MAPY][MAPX], LEVEL *level);        // checks a level for errors
void pill_fill(LEVEL *level, EDITOR *edit);
void place_line(LEVEL *level, EDITOR *edit);
void remove_tile(LEVEL *level, EDITOR *edit);
