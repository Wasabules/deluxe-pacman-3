#pragma once
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAP_ID "Pace2"  // The map ID, this identifies it as a Deluxe Pacman 2 level
#define MAP_VER 3       // The current version of the map that the game normally supports (1 & 2 = no teleports, 3 = teleports)
#define MAPX 23         // width of the map in cells
#define MAPY 17         // height of the map in cells
#define TILE_SIZE 32    // The size of each cell tile


typedef struct TILE {
   unsigned char tile;  // current tile image #.  This will be an index into the current line set or pill set.
   bool is_pill;        // is the current map tile a pill? true = yes, false = line.
   bool is_powerpill;   // is the current map tile a powerpill? true = yes.  Tile must also be a pill.
   bool is_protected;   // is the current map tile protected from being drawn on with a line (usually spawn points).
} TILE;

typedef struct MAP {
   unsigned char x;  // map cell, 0 to 22
   unsigned char y;  // map cell, 0 to 16
} MAP;

typedef struct LEVEL {
   char map_id[6];                  // currently "Pace2"
   unsigned char map_ver;           // currently 2
   bool validated;                  // is the map validated yet?  Unplayable if false.
   unsigned char line_set;          // The line set used, currently 0 to 19
   MAP player;                      // player map location
   MAP ghost[4];                    // ghost map locations
   MAP pickup;                      // pickup map spawn location
   MAP teleport[2];                 // teleport map locations
   unsigned char background;        // 0 - ? = background tile to use.
   TILE map[MAPY][MAPX];            // map data: an array of tiles
   unsigned short pills;            // the number of pills & powerpills on the map
} LEVEL;

#include "dp2_ghost.h"

// dest_mx & dest_my is the map tile location you're trying to find a path TO.
void get_path(int path_node[MAPY][MAPX], LEVEL *level, MAP *dest);
int dir(LEVEL *level, GHOST *ghost, MAP *dest, bool random_dir);
