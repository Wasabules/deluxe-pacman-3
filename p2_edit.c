/****************************************************
 * Contains functions only used in the Pace 2 level *
 * editor for Deluxe Pacman 2.                      *
 ****************************************************/

#include "p2_edit.h"

// initialize all level data to default values
void initialize_map(LEVEL *level)
{
   strcpy(level->map_id, MAP_ID);
   level->map_ver = MAP_VER;
   level->validated = false;
   level->player.x = 99;
   level->player.y = 99;
   for(int i = 0; i < 4; i++) {
      level->ghost[i].x = 99;
      level->ghost[i].y = 99;
   }
   for(int i = 0; i < 2; i++) {
      level->teleport[i].x = 99;
      level->teleport[i].y = 99;
   }
   level->pickup.x = 99;
   level->pickup.y = 99;
   for(unsigned char y = 0; y < MAPY; y++) {
      for(unsigned char x = 0; x < MAPX; x++) {
         level->map[y][x].tile = 0;
         level->map[y][x].is_pill = false;
         level->map[y][x].is_powerpill = false;
         level->map[y][x].is_protected = false; // New to version 3 maps
      }
   }
   level->pills = 0;
}


// returns true if loaded properly
bool loadmap(ALLEGRO_DISPLAY *display, LEVEL *level, char *levelname)
{
   al_set_standard_file_interface();

   ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
   al_append_path_component(path, "Custom"); // Add the Custom folder to the path
   al_set_path_filename(path, "level001.dp2");
   const char *filename = al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP);

   ALLEGRO_FILECHOOSER *file_dialog = al_create_native_file_dialog(filename, "Load Level",
                                        "*.DP2;*.*", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
   if(!file_dialog) {
      al_set_physfs_file_interface();
      return false;
   }

   if(!al_show_native_file_dialog(display, file_dialog)) {
      if(file_dialog) al_destroy_native_file_dialog(file_dialog);
      al_set_physfs_file_interface();
      return false;
   }

   filename = al_get_native_file_dialog_path(file_dialog, 0);

   // convert to lower case
   char lcname[4096] = "";
   for(int i = 0; i < strlen(filename); i++) lcname[i] = tolower(filename[i]);

   ALLEGRO_FILE *file = NULL;

   file = al_fopen(lcname, "rb");
   if(file == NULL) {
      printf("%s(%d): Error opening \"%s\" for reading.\n", __FILE__, __LINE__, filename);
      if(file_dialog) al_destroy_native_file_dialog(file_dialog);
      al_set_physfs_file_interface();
      return 0;
   }

   al_fread(file, &level->map_id, sizeof(char)*strlen(MAP_ID));
   level->map_ver = al_fgetc(file);

   if(strcmp(level->map_id, MAP_ID) != 0) {  // Check the map ID, should be "Pace2"
      printf("%s(%d): Invalid map type...\n   Map ID = \"%s\".\n", __FILE__, __LINE__, level->map_id);
      al_fclose(file);
      if(file_dialog) al_destroy_native_file_dialog(file_dialog);
      al_set_physfs_file_interface();
      return false;
   }
   else if(level->map_ver > MAP_VER) { // Make sure the map version isn't too high.
      printf("%s(%d): Invalid map version...\n   Version expected: %d, level version: %d\n", __FILE__, __LINE__, MAP_VER, level->map_ver);
      al_fclose(file);
      if(file_dialog) al_destroy_native_file_dialog(file_dialog);
      al_set_physfs_file_interface();
      return false;
   }
   printf("Loaded map_ver = %d\n", level->map_ver);
   level->validated = al_fgetc(file);
   level->line_set = al_fgetc(file);
   level->player.x = al_fgetc(file);
   level->player.y = al_fgetc(file);
   for(int i = 0; i < 4; i++) {
      level->ghost[i].x = al_fgetc(file);
      level->ghost[i].y = al_fgetc(file);
   }
   level->pickup.x = al_fgetc(file);
   level->pickup.y = al_fgetc(file);
   if(level->map_ver >= 3) {  // Teleports are in map version 3 or greater.
      for(int i = 0; i < 2; i++) {
         level->teleport[i].x = al_fgetc(file);
         level->teleport[i].y = al_fgetc(file);
      }
   }
   else {   // Old version map, set default teleport values (99 = not set)
      for(int i = 0; i < 2; i++) {
         level->teleport[i].x = 99;
         level->teleport[i].y = 99;
      }
   }
   level->background = al_fgetc(file);
   for(unsigned char y = 0; y < MAPY; y++) {
      for(unsigned char x = 0; x < MAPX; x++) {
         level->map[y][x].tile = al_fgetc(file);
         level->map[y][x].is_pill = al_fgetc(file);
         level->map[y][x].is_powerpill = al_fgetc(file);
         if(level->map_ver >= 3) level->map[y][x].is_protected = al_fgetc(file);
         else {   // Older maps, we'll have to check for various spawns and set this here.
            // Check for player, ghost[4], pickup and teleport[2] spawns...
            level->map[y][x].is_protected = false; // Set to false by default, unless a spawn is detected.
            if(level->player.x == x && level->player.y == y) {
               level->map[y][x].is_protected = true;
               continue;   // No need to check further.
            }
            if(level->pickup.x == x && level->pickup.y == y) {
               level->map[y][x].is_protected = true;
               continue;   // No need to check further.
            }
            for(int j = 0; j < 4; j++) {
               if(level->ghost[j].x == x && level->ghost[j].y == y) {
                  level->map[y][x].is_protected = true;
                  break;
               }
            }
            if(level->map[y][x].is_protected) continue;  // No sense checking further if it is already set.
            for(int j = 0; j < 2; j++) {
               if(level->teleport[j].x == x && level->teleport[j].y == y) {
                  level->map[y][x].is_protected = true;
                  break;
               }
            }
         }
      }
   }
   al_fread(file, &level->pills, sizeof(unsigned short));

   // never forget to close the file
   al_fclose(file);

   if(file_dialog) al_destroy_native_file_dialog(file_dialog);

   al_set_physfs_file_interface();

   strcpy(levelname, al_get_path_filename(al_create_path(lcname)));

   level->map_ver = MAP_VER;  // Set the level to the new file version! (in case an old version was loaded)

   return true;
}


// returns true if saved properly>
bool savemap(ALLEGRO_DISPLAY *display, LEVEL *level, char *levelname)
{
   al_set_standard_file_interface();

   ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
   al_append_path_component(path, "Custom"); // Add the Custom folder to the path
   al_set_path_filename(path, (const char *)levelname);
   const char *filename = al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP);

   /// TODO: Search folder for levels and set the filename to the next available level #
   ALLEGRO_FILECHOOSER *file_dialog = al_create_native_file_dialog(filename, "Save Level",
                                                       "*.DP2", ALLEGRO_FILECHOOSER_SAVE);
   if(!file_dialog) {
      al_set_physfs_file_interface();
      return false;
   }

   if(!al_show_native_file_dialog(display, file_dialog)) {
      if(file_dialog) al_destroy_native_file_dialog(file_dialog);
      al_set_physfs_file_interface();
      return false;
   }

   filename = al_get_native_file_dialog_path(file_dialog, 0);

   // convert to lower case
   char lcname[4096] = "";
   for(int i = 0; i < strlen(filename); i++) lcname[i] = tolower(filename[i]);

   ALLEGRO_FILE *file = NULL;

   file = al_fopen(lcname, "wb");
   if(file == NULL) {
      printf("%s(%d): Error opening \"%s\" for writing.\n", __FILE__, __LINE__, lcname);
      if(file_dialog) al_destroy_native_file_dialog(file_dialog);
      al_set_physfs_file_interface();
      return false;
   }

   al_fwrite(file, level->map_id, strlen(level->map_id));

   al_fputc(file, level->map_ver);
   al_fputc(file, level->validated);
   al_fputc(file, level->line_set);
   al_fputc(file, level->player.x);
   al_fputc(file, level->player.y);
   for(int i = 0; i < 4; i++) {
      al_fputc(file, level->ghost[i].x);
      al_fputc(file, level->ghost[i].y);
   }
   al_fputc(file, level->pickup.x);
   al_fputc(file, level->pickup.y);
   for(int i = 0; i < 2; i++) {  // New to version 3 maps.
      al_fputc(file, level->teleport[i].x);
      al_fputc(file, level->teleport[i].y);
   }
   al_fputc(file, level->background);
   for(unsigned char y = 0; y < MAPY; y++) {
      for(unsigned char x = 0; x < MAPX; x++) {
         al_fputc(file, level->map[y][x].tile);
         al_fputc(file, level->map[y][x].is_pill);
         al_fputc(file, level->map[y][x].is_powerpill);
         al_fputc(file, level->map[y][x].is_protected);  // New to version 3 maps.
      }
   }
   al_fwrite(file, &level->pills, sizeof(unsigned short));

   // never forget to close the file
   al_fclose(file);

   if(file_dialog) al_destroy_native_file_dialog(file_dialog);

   al_set_physfs_file_interface();

   strcpy(levelname, al_get_path_filename(al_create_path(lcname)));

   return true;
}



// returns true if validated
bool validate(char message[256], bool error[MAPY][MAPX], LEVEL *level)
{
   MAP val;
   bool valid = true;
   int path_node[MAPY][MAPX];
   char *colour[4] = {"Red", "Green", "Cyan", "Purple"}; // ghost colours

   memset(error, 0, MAPY * MAPX);

   // path will start from the player spawn
   val.x = level->player.x;
   val.y = level->player.y;

   // check that the player spawn has been set
   if(val.x == 99 || val.y == 99) {
      snprintf(message, 256, "Missing Player");
      return false;
   }
   /*
   // check to make certain spawn isn't on the border of the map
   if(val.x == 0 || val.x == MAPX - 1 || val.y == 0 || val.y == MAPY - 1) {
      snprintf(message, 256, "Pacman spawn on border");
      return false;
   }
   */

   // get a path from the player spawn
   // this will fill the map's movable spaces with numbers
   // that start with zero (at the player spawn) and get
   // higher as they radiate out from that, spaces next to
   // zero will be 1, then 2's next to it and so on outward
   // if any spaces on the map cannot be reaches they will
   // contain -1s, map tiles that have lines will have 999.
   get_path(path_node, level, &val);

   int teleports = 0;
   int pill_error_count = 0;
   for(int y = 0; y < MAPY; y++) {
      for(int x = 0; x < MAPX; x++) {
         // check to make certain that pills are all reachable
         if(path_node[y][x] == -1 && level->map[y][x].is_pill) {
            error[y][x] = true;
            pill_error_count++;
            valid = false;
            if(pill_error_count == 1) snprintf(message, 256, "Isolated Pill");
            else snprintf(message, 256, "Isolated Pills");
         }
         if(level->teleport[0].x == x && level->teleport[0].y == y) teleports++;
         if(level->teleport[1].x == x && level->teleport[1].y == y) teleports++;
      }
   }
   if(!valid) return false;

   // Check for missing or too many teleports. (teleports are not required for validation)
   if(teleports == 1) {
      snprintf(message, 256, "Missing Teleport");
      return false;
   }
   else if(teleports > 2) {
      snprintf(message, 256, "Too Many Teleports");   // It is virtually impossible for this to happen but...
      return false;                             // I actually triggered this when I first tested! :)
   }

   for(int i = 0; i < 2; i++) {
      int x = level->teleport[i].x;
      int y = level->teleport[i].y;

      if(path_node[y][x] == -1) {
         snprintf(message, 256, "Isolated Teleport"); // in isolated area that can't be reached
         error[y][x] = true;
         return false;
      }
   }

   int wrap_error_count = 0;
   for(int y = 0; y < MAPY; y++) {
      for(int x = 0; x < MAPX; x++) {
         // Check to make sure any empty spots that can be moved on to around
         // the border have a matching spot on the opposite side of the map.
         if(x == 0 || x == MAPX - 1) { // on the border?
            if(path_node[y][x] >= 0 && path_node[y][x] < 999) { // movable and not a line?
               if(path_node[y][MAPX - 1 - x] == -1 || path_node[y][MAPX - 1 - x] == 999) { // oppsite side the same?
                  error[y][x] = true;        // flag both sides in the error
                  error[y][MAPX - 1 - x] = true;
                  wrap_error_count++;
                  if(wrap_error_count == 1) snprintf(message, 256, "Wrap Error");
                  else snprintf(message, 256, "Wrap Errors");
                  valid = false;
               }
            }
         }
         if(y == 0 || y == MAPY - 1) {
            if(path_node[y][x] >= 0 && path_node[y][x] < 999) {
               if(path_node[MAPY - 1 - y][x] == -1 || path_node[MAPY - 1 - y][x] == 999) {
                  error[y][x] = true;
                  error[MAPY - 1 - y][x] = true;
                  wrap_error_count++;
                  if(wrap_error_count == 1) snprintf(message, 256, "Wrap Error");
                  else snprintf(message, 256, "Wrap Errors");
                  valid = false;
               }
            }
         }
      }
   }
   if(!valid) return false;

   // check to make certain pickup spawn was placed.
   if(level->pickup.x == 99 || level->pickup.y == 99) {
      snprintf(message, 256, "Missing Pickup");
      return false;
   }
   // check to make certain the pickup spawn isn't in an unreachable location
   if(path_node[level->pickup.y][level->pickup.x] == -1) {
      error[level->pickup.y][level->pickup.x] = true;
      snprintf(message, 256, "Isolated Pickup"); // in isolated area that can't be reached
      return false;
   }
   /*
   // check to make certain spawn isn't on the border of the map
   if(level->pickup.x == 0 || level->pickup.x == MAPX - 1 || level->pickup.y == 0 || level->pickup.y == MAPY - 1) {
      snprintf(message, 256, "Pickup spawn on border");
      return false;
   }
   */

   // check all 4 ghosts
   for(int i = 0; i < 4; i++) {
      int x = level->ghost[i].x;
      int y = level->ghost[i].y;
      // were ghost spawns set?
      if(x == 99 || y == 99) {
         snprintf(message, 256, "Missing %s Ghost", colour[i]);
         return false;
      }
      // is the ghost spawn set in an unreachable location?
      if(path_node[y][x] == -1) {
         snprintf(message, 256, "Isolated %s Ghost", colour[i]);
         error[y][x] = true;
         return false;
      }
      // check to make certain the ghost isn't next to the player spawn
      // we'll use val.x and val.y which were already set to the player
      // location earlier to compare.
      if((abs(val.x - x) <= 1) && (abs(val.y - y) <= 1)) {
         snprintf(message, 256, "%s Ghost too close to Pacman", colour[i]);
         error[y][x] = true;
         return false;
      }
      /*
      // check to make certain spawn isn't on the border of the map
      if(x == 0 || x == MAPX - 1 || y == 0 || y == MAPY - 1) {
         snprintf(message, 256, "%s Ghost spawn on border", colour[i]);
         return false;
      }
      */
   }

   // make certain there is at least 1 pill on the level
   if(level->pills == 0) {
      snprintf(message, 256, "Missing Pills");
      return false;
   }

   return true;
}


// pill_fill() will fill all empty spaces on a level with pills
// it will also replace any existing pills that aren't powerpills
// it will not place a pill on a level, a pacman or ghost spawn.
void pill_fill(LEVEL *level, EDITOR *edit)
{
   MAP val;
   int path_node[MAPY][MAPX];
   bool checkpath = false; // used to check the path if the player spawn is set

   // Here we'll check the map for where the player can move, but only IF the player
   // has been placed on the map.  This way we can place the pills ONLY on the spaces
   // that the player can move to.  If the player spawn hasn't been placed yet than
   // this will simply fill all empty spaces with pills.
   val.x = level->player.x;
   val.y = level->player.y;
   if(val.x != 99) { // 99 is the value this is set to when player.x has not been placed yet
      get_path(path_node, level, &val);
      checkpath = true; // player spawn is set, so check the path as well
   }

   for(int y = 0; y < MAPY; y++) {
      for(int x = 0; x < MAPX; x++) {
         if(!level->map[y][x].tile || (level->map[y][x].is_pill && !level->map[y][x].is_powerpill)) {
            if(!level->map[y][x].is_protected) {   // Spawns are protected from being overwritten by pillfill or line drawing.
               if(checkpath && path_node[y][x] == -1) continue;   // Isolated tile, do not put anything here.
               level->map[y][x].is_pill = true;
               level->map[y][x].is_powerpill = false;
               level->map[y][x].tile = edit->pill + 1;
            }
         }
      }
   }
}



void place_line(LEVEL *level, EDITOR *edit)
{
   // Some short variables to make things a little easier to read
   int x = edit->x;
   int y = edit->y;
   int ox = edit->oldx;
   int oy = edit->oldy;

   if(x < 0 || x >= MAPX) return;            // Check the horizontal location and make certain it is not off the map.
   if(y < 0 || y >= MAPY) return;            // Check the vertical location and make certain it is not off the map.
   if(ox < 0 || ox >= MAPX) edit->oldx = x;  // When drawing we want to make certain the old location (used when connecting lines)
   if(oy < 0 || oy >= MAPY) edit->oldy = y;  // was on the map, and if not, set it to the current location to avoid errors.

   // Do not place a line over top of a protected tile (usually a spawn).
   if(level->map[y][x].is_protected) return;

   // If the last location we used was a protected tile, reset it to the current one (so we don't try and connect a line to it).
   if(level->map[oy][ox].is_protected) {
      edit->oldx = x;
      edit->oldy = y;
   }

   // update ox and oy in case they changed
   ox = edit->oldx;
   oy = edit->oldy;
   int line_tile = level->map[y][x].tile;

   // If our line overwrote a pill/powerpill, reset the flags for them.
   if(level->map[y][x].is_pill || line_tile == 0) {
      level->map[y][x].is_pill = false;
      level->map[y][x].is_powerpill = false;
      level->map[y][x].tile = 1;
      line_tile = 0;
   }
   else if(line_tile) line_tile--;

   /// ORing connects like this...
   ///     1
   ///     |
   /// 8 - 0 - 2
   ///     |
   ///     4
   /// The lines are organized so that ORing the current value with one of these numbers
   /// creates a line graphic that connects all the directions indicated.  So if we have
   /// a connection to the right, that line number graphic is 2.  If we OR it with 1 so we
   /// also have a connection going up, plus the one to the right, 2 OR 1 = 3, so the graphic
   /// we want is #3, that will have a line up and to the right (a corner).  Incidentally,
   /// Graphic line #0 is the DOT line with no connections, which it defaults to when you
   /// start to draw.

   // If the current tile is different from the last one connect and draw it.
   if(oy != y || ox != x) {
      if(oy == y) {  // We moved horizontally.
         if(x - ox == 1) { // If we only moved one tile to the right, we can connect them up.
            line_tile |= 8;   // ORing the current tile with 8, makes a line that connects left (to the old tile from the new).
            level->map[y][x].tile = line_tile + 1;
            line_tile = level->map[oy][ox].tile;
            if(line_tile) line_tile--;
            line_tile |= 2;   // ORing the current tile with 2, makes a line that connects right (to the new tile from the old).
            level->map[oy][ox].tile = line_tile + 1;
         }
         else if(x - ox == -1) { // If we only moved one tile to the left, connect them up.
            line_tile |= 2;   // ORing the current tile with 2, makes a line that connects right (to the old tile from the new).
            level->map[y][x].tile = line_tile + 1;
            line_tile = level->map[oy][ox].tile;
            if(line_tile) line_tile--;
            line_tile |= 8;   // ORing the current tile with 8, makes a line that connects left (to the new tile from the old).
            level->map[oy][ox].tile = line_tile + 1;
         }
      }
      else if(ox == x) {
         if(y - oy == 1) { // If we only moved one tile down, connect them up.
            line_tile |= 1;   // ORing the current tile with 1, makes a line that connects up (to the old tile, from the new).
            level->map[y][x].tile = line_tile + 1;
            line_tile = level->map[oy][ox].tile;
            if(line_tile) line_tile--;
            line_tile |= 4;   // ORing the current tile with 4, makes a line that connects down (to the new tile, from the old).
            level->map[oy][ox].tile = line_tile + 1;
         }
         else if(y - oy == -1) { // If we only moved one tile up, connect them up.
            line_tile |= 4;   // ORing the current tile with 4, makes a line that connects down (to the old tile from the new).
            level->map[y][x].tile = line_tile + 1;
            line_tile = level->map[oy][ox].tile;
            if(line_tile) line_tile--;
            line_tile |= 1;   // ORing the current tile with 1, makes a line that connects up (to the new tile from the old).
            level->map[oy][ox].tile = line_tile + 1;
         }
      }
   }
}



void remove_tile(LEVEL *level, EDITOR *edit)
{
   int line_tile;
   int x = edit->x;
   int y = edit->y;

   // Make sure we're on the map
   if(x < 0 || x >= MAPX) return;
   if(y < 0 || y >= MAPY) return;

   if(level->player.x == x && level->player.y == y) {
      level->player.x = 99;   // Setting these to 99 = not set yet
      level->player.y = 99;
   }

   for(int i = 0; i < 4; i++) {
      if(level->ghost[i].x == x && level->ghost[i].y == y) {
         level->ghost[i].x = 99;
         level->ghost[i].y = 99;
      }
   }

   for(int i = 0; i < 2; i++) {
      if(level->teleport[i].x == x && level->teleport[i].y == y) {
         level->teleport[i].x = 99;
         level->teleport[i].y = 99;
      }
   }

   if(level->pickup.x == x && level->pickup.y == y) {
      level->pickup.x = 99;
      level->pickup.y = 99;
   }

   // If tile is a line, check all lines that connect to this one and sever the links.
   if(!level->map[y][x].is_pill && level->map[y][x].tile) {
      if(x - 1 >= 0) {
         if(!level->map[y][x - 1].is_pill) {
            line_tile = level->map[y][x - 1].tile;
            if(line_tile) {
               line_tile--;
               line_tile &= 13;
               level->map[y][x - 1].tile = line_tile + 1;
            }
         }
      }
      else {   // When we erase a tile on the border, adjust the opposite border
         if(!level->map[y][MAPX - 1].is_pill) {
            line_tile = level->map[y][MAPX - 1].tile;
            if(line_tile) {
               line_tile--;
               line_tile &= 13;
               level->map[y][MAPX - 1].tile = line_tile + 1;
            }
         }
      }

      if(x + 1 < MAPX) {
         if(!level->map[y][x + 1].is_pill) {
            line_tile = level->map[y][x + 1].tile;
            if(line_tile) {
               line_tile--;
               line_tile &= 7;
               level->map[y][x + 1].tile = line_tile + 1;
            }
         }
      }
      else {   // When we erase a tile on the border, adjust the opposite border
         if(!level->map[y][0].is_pill) {
            line_tile = level->map[y][0].tile;
            if(line_tile) {
               line_tile--;
               line_tile &= 7;
               level->map[y][0].tile = line_tile + 1;
            }
         }
      }

      if(y - 1 >= 0) {
         if(!level->map[y - 1][x].is_pill) {
            line_tile = level->map[y - 1][x].tile;
            if(line_tile) {
               line_tile--;
               line_tile &= 11;
               level->map[y - 1][x].tile = line_tile + 1;
            }
         }
      }
      else {   // When we erase a tile on the border, adjust the opposite border
         if(!level->map[MAPY - 1][x].is_pill) {
            line_tile = level->map[MAPY - 1][x].tile;
            if(line_tile) {
               line_tile--;
               line_tile &= 11;
               level->map[MAPY - 1][x].tile = line_tile + 1;
            }
         }
      }

      if(y + 1 < MAPY) {
         if(!level->map[y + 1][x].is_pill) {
            line_tile = level->map[y + 1][x].tile;
            if(line_tile) {
               line_tile--;
               line_tile &= 14;
               level->map[y + 1][x].tile = line_tile + 1;
            }
         }
      }
      else {   // When we erase a tile on the border, adjust the opposite border
         if(!level->map[0][x].is_pill) {
            line_tile = level->map[0][x].tile;
            if(line_tile) {
               line_tile--;
               line_tile &= 14;
               level->map[0][x].tile = line_tile + 1;
            }
         }
      }
   }

   level->map[y][x].tile = 0;  // reset tile to empty
   level->map[y][x].is_pill = false;
   level->map[y][x].is_powerpill = false;
   level->map[y][x].is_protected = false;
}
