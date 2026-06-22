/**********************************************************************
 *                                                                    *
 *   Name.........: Pace 2                                            *
 *   Author.......: Neil Roy                                          *
 *   Start Date...: August 3, 2012 @11:02AM                           *
 *   Last Update..: June 14, 2017 @1:42AM                             *
 *   Requirements.: Allegro v5.2.3, MinGW32 7.1.0                     *
 *   Description..: This is the Deluxe Pacman 2 level editor.         *
 *   Notes........: This is compiled as a C 2011 (-std=gnu11) project *
 *                                                                    *
 **********************************************************************/

#define ALLEGRO_STATICLINK

#include <stdio.h>
#include <physfs.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_physfs.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "resource.h"
#include "a5_error.h"
#include "p2_main.h"
#include "dp2_map.h"
#include "dp2_gui.h"
#include "p2_edit.h"
#include "dp2_config.h"
#include "a5_scale_screen.h"



int main(int argc, char *argv[])
{
   int x, y;
   int teleport_number = 0;   // We'll toggle between 0 and 1 each time one is placed.
   int mouse_x = 0, mouse_y = 0;
   char message[256] = "NO";
   char levelname[4096] = "level001.dp2";
   SETTING setting = {"Pace 2", NULL, NULL, NULL, 0, 0, true, ALLEGRO_WINDOWED};
   EDITOR edit = {0, 0, 0, 0, 0, 0, 0, 0, true, true, LINE};
   EDITOR sedit = {0, 0, 0, 0, 0, 0, 0, 0, true, true, LINE};
   ALLEGRO_EVENT_QUEUE *queue = NULL;
   bool done = false;
   BUTTON b_new = {};   // {} sets all BUTTON fields to zero.
   BUTTON b_load = {};
   BUTTON b_save = {};
   BUTTON b_quit = {};
   BUTTON b_grid = {};
   BUTTON b_validate = {};
   BUTTON b_ll_arrow = {};
   BUTTON b_rl_arrow = {};
   BUTTON b_pfill = {};
   BUTTON s_player = {};
   BUTTON s_symx = {};
   BUTTON s_symy = {};
   BUTTON s_pickup = {};
   BUTTON s_ppill = {};
   BUTTON s_teleport = {};
   BUTTON s_ghost[4] = {};
   ALLEGRO_COLOR ghost_color[4];
   ALLEGRO_FONT *font = NULL;
   ALLEGRO_BITMAP *hand = NULL;
   ALLEGRO_MOUSE_CURSOR *cursor = NULL;
   ALLEGRO_BITMAP *pacman = NULL;
   ALLEGRO_BITMAP *ghost[4] = {NULL};
   ALLEGRO_BITMAP *pickup = NULL;
   ALLEGRO_BITMAP *teleport = NULL;
   ALLEGRO_BITMAP *background[20] = {NULL};
   ALLEGRO_BITMAP *pills[5][8] = {{NULL}};
   ALLEGRO_BITMAP *lines[100][16] = {{NULL}};
   bool error[MAPY][MAPX];  // will be initialized with new level

   ghost_color[0] = al_map_rgb_f(1, 0, 0); //Red
   ghost_color[1] = al_map_rgb_f(0, 1, 0); //Green
   ghost_color[2] = al_map_rgb_f(0, 1, 1); //Cyan
   ghost_color[3] = al_map_rgb_f(1, 0, 1); //Purple

   // redirect stdout to file
   /*
   if(!freopen("log.txt", "w", stdout)) {
      return 1;
   }
   */

   printf("Initializing Allegro.\n");
   al_init();
   al_init_image_addon();
   al_init_font_addon();
   al_init_ttf_addon();

   if(!al_init_primitives_addon()) {
      printf("%s(%d): al_init_primitives_addon() failed.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }

   if(!al_install_keyboard()) {
      printf("%s(%d): al_install_keyboard() failed.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }

   if(!al_install_mouse()) {
      printf("%s(%d): al_install_mouse() failed.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }

   load_config(&setting);

   // Only ALLEGRO_FULLSCREEN_WINDOW supported for editor supported due to dialog problems
   // shrinking down the editor screen to display dialog.
   //if(setting.fullscreen==1) al_set_new_display_flags(ALLEGRO_FULLSCREEN);
   if(setting.fullscreen == 2) al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);

   printf("Creating display.\n");
   al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA);

   al_set_new_bitmap_flags(ALLEGRO_MAG_LINEAR | ALLEGRO_VIDEO_BITMAP);
   al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);

   setting.screen = al_create_display(WIDTH, HEIGHT);
   if(!setting.screen) {
      printf("%s(%d): al_create_display() failed!\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   setting.h = al_get_display_height(setting.screen);
   setting.w = al_get_display_width(setting.screen);

   al_clear_to_color(al_map_rgb_f(0, 0, 0));
   al_flip_display();

   if(setting.fullscreen == 2) a5_scale_screen(WIDTH, HEIGHT, setting.w, setting.h);

   printf("Initializing PHYSYS.\n");
   PHYSFS_init(NULL);
   if(!PHYSFS_mount("Deluxe Pacman 2.pak", "/", 1)) {
      printf("%s(%d): PHSYFS_init() failed.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   al_set_physfs_file_interface();

   printf("Loading icon.\n");
   setting.icon = al_load_bitmap("Graphics/Deluxe_Pacman_2_Icon.png");
   if(!setting.icon) {
      printf("%s(%d): Error loading Deluxe_Pacman_2_Icon.png\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   al_set_display_icon(setting.screen, setting.icon);
   al_set_window_title(setting.screen, setting.name);

   setting.redraw_timer = al_create_timer(1.0 / 60);
   queue = al_create_event_queue();
   if(!queue) {
      printf("%s(%d): Error creating event queue.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }

   printf("Loading mouse pointer.\n");
   hand = al_load_bitmap("Graphics/Hand.png");
   if(!hand) {
      printf("%s(%d): Error loading Hand.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }

   cursor = al_create_mouse_cursor(hand, 0, 0);
   if(!cursor) {
      printf("%s(%d): Error setting mouse cursor.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }

   ALLEGRO_BITMAP *tbmp = NULL;
   char s[TEXT_BUFFER] = "";


   printf("Loading line graphics.\n");

   // Here we'll check to see how many line files there are
   int linecount = 0;   // We'll use this to count how many we find
   bool file_found;     // This will be true if we find one, false if we do not
   do {
      snprintf(s, TEXT_BUFFER, "Graphics/Lines/Line%02d.png", linecount);
      file_found = al_filename_exists(s);
      if(file_found) linecount++;   // Increment the number we found so far
   } while(file_found); // As long as we are finding files, keep looking

   if(!linecount) {  // If we found no line files at all, there must be a problem, report it and exit.
      printf("%s(%d): No lines found in pakfile.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }

   printf("Lines found in pak: %d\n", linecount);  // Print to the console how many found

   // We now know exactly how many exist in the pak file, we can load them all in
   for(int i = 0; i < linecount; i++) {
      snprintf(s, TEXT_BUFFER, "Graphics/Lines/Line%02d.png", i);       // Set the filename to check for
      tbmp = al_load_bitmap(s);                                   // Attempt to load it
      if(!tbmp) {                                                 // If it does load, exit game.
         printf("%s(%d): Error loading \"%s\".\n", __FILE__, __LINE__, s);
         goto SHUTDOWN;
      }
      for(int j = 0; j < 16; j++) {                                     // There are 16 tiles per line strip
         lines[i][j] = al_create_bitmap(TILE_SIZE, TILE_SIZE);          // Create a bitmap for each line tile
         al_set_target_bitmap(lines[i][j]);                             // set the target to draw it to the lines array
         al_draw_bitmap_region(tbmp, j * 32, 0, 32, 32, 0, 0, 0); // draw it
      }
      al_destroy_bitmap(tbmp); tbmp = NULL;                 // We no longer need the temporary bitmap
   }

   int max_linesets = linecount; // Set the maximum line sets we found so far.

   // Check for custom line sets from 0 to 99 (100 total)
   al_set_standard_file_interface();
   int custom = 0;
   for(int i = 0; i < 100; i++) {
      snprintf(s, TEXT_BUFFER, "Custom\\Line%02d.png", i);  // We'll look in ths Custom folder
      tbmp = al_load_bitmap(s);
      if(!tbmp) { // Nothing was loaded, so we reached the last custom lineset (if any) found.
         if(i >= linecount) { // Only stop if i is greater in number than what was found in the pak file
            printf("Custom line sets found: %d\n", custom);
            break;   // Break out of the for loop
         }
      }
      else {   // We found something...
         // First destroy any existing lines this may be replacing...
         for(int j = 0; j < 16; j++) {
            if(lines[i][j]) {
               al_destroy_bitmap(lines[i][j]);
               lines[i][j] = NULL;
            }
         }
         for(int j = 0; j < 16; j++) {
            lines[i][j] = al_create_bitmap(TILE_SIZE, TILE_SIZE);
            al_set_target_bitmap(lines[i][j]);
            al_draw_bitmap_region(tbmp, j * 32, 0, 32, 32, 0, 0, 0);
         }
         al_destroy_bitmap(tbmp); tbmp = NULL;
         if(i >= linecount) max_linesets++;
         custom++;
      }
   }
   al_set_physfs_file_interface();

   printf("Maximum lineset# = %d (some custom sets may replace existing sets)\n", max_linesets-1);


   // Load Pills in...
   printf("Loading pill graphics.\n");
   tbmp = al_load_bitmap("Graphics/Pill_Sheet.png");
   if(!tbmp) {
      printf("%s(%d): Error loading Pill_Sheet.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   for(int y = 0; y < 5; y++) {
      for(int x = 0; x < 8; x++) {
         pills[y][x] = al_create_bitmap(32, 32);
         al_set_target_bitmap(pills[y][x]);
         al_draw_bitmap_region(tbmp, x * 32, y * 32, 32, 32, 0, 0, 0);
      }
   }
   al_destroy_bitmap(tbmp); tbmp = NULL;

   printf("Loading pacman graphics.\n");
   tbmp = al_load_bitmap("Graphics/Pacman.png");
   if(!tbmp) {
      printf("%s(%d): Error loading Pacman.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   pacman = al_create_bitmap(50, 50);
   al_set_target_bitmap(pacman);
   al_draw_bitmap_region(tbmp, 50, 0, 50, 50, 0, 0, 0);
   al_destroy_bitmap(tbmp); tbmp = NULL;

   printf("Loading ghost graphics.\n");
   tbmp = al_load_bitmap("Graphics/Ghosts.png");
   if(!tbmp) {
      printf("%s(%d): Error loading Ghosts.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   for(int i = 0; i < 4; i++) {
      ghost[i] = al_create_bitmap(50, 50);
      al_set_target_bitmap(ghost[i]);
      al_draw_bitmap_region(tbmp, 200, i * 50, 50, 50, 0, 0, 0);
   }
   al_destroy_bitmap(tbmp); tbmp = NULL;

   printf("Loading pickup graphic.\n");
   tbmp = al_load_bitmap("Graphics/Pickups.png");
   if(!tbmp) {
      printf("%s(%d): Error loading Pickups.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   pickup = al_create_bitmap(50, 50);
   al_set_target_bitmap(pickup);
   al_draw_bitmap_region(tbmp, 0, 0, 50, 50, 0, 0, 0);
   al_destroy_bitmap(tbmp); tbmp = NULL;

   printf("Loading teleport graphic.\n");
   tbmp = al_load_bitmap("Graphics/Teleport.png");
   if(!tbmp) {
      printf("%s(%d): Error loading Teleport.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   teleport = al_create_bitmap(50, 50);
   al_set_target_bitmap(teleport);
   al_draw_bitmap_region(tbmp, 0, 0, 50, 50, 0, 0, 0);
   al_destroy_bitmap(tbmp); tbmp = NULL;

   printf("Loading background graphics.\n");
   tbmp = al_load_bitmap("Graphics/Backgrounds.png");
   if(!tbmp) {
      printf("%s(%d): Error loading Backgrounds.png\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   for(int i = 0; i < 20; i++) {
      background[i] = al_create_bitmap(TILE_SIZE, TILE_SIZE);
      al_set_target_bitmap(background[i]);
      al_draw_bitmap_region(tbmp, i * 32, 0, 32, 32, 0, 0, 0);
   }
   al_destroy_bitmap(tbmp); tbmp = NULL;

   printf("Loading truetype font.\n");
   font = al_load_ttf_font("Fonts/Verdana_Bold.ttf", -28, 0);
   if(!font) {
      printf("%s(%d): Error loading font.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }

   printf("Initializing buttons.\n");
   if(!new_button(&b_new, false, "Graphics/Buttons/New.png", 759, 48)) { // We load in the button graphics, location etc. (see dp2_gui.h).
      printf("%s(%d): Error loading New.png.\n", __FILE__, __LINE__);    // This error message will only show in debug mode or if output is
      goto SHUTDOWN;                                                     // redirected to a file, like "pace.exe > log.txt".
   }
   if(!new_button(&b_load, false, "Graphics/Buttons/Load.png", 759, 120)) {
      printf("%s(%d): Error loading Load.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   if(!new_button(&b_save, false, "Graphics/Buttons/Save.png", 759, 192)) {
      printf("%s(%d): Error loading Save.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   if(!new_button(&b_quit, false, "Graphics/Buttons/Quit.png", 759, 264)) {
      printf("%s(%d): Error loading Quit.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   if(!new_button(&b_grid, false, "Graphics/Buttons/Grid.png", 759, 336)) {
      printf("%s(%d): Error loading Grid.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   if(!new_button(&b_validate, false, "Graphics/Buttons/Validate.png", 759, 408)) {
      printf("%s(%d): Error loading Validate.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   if(!new_button(&b_pfill, false, "Graphics/Buttons/Pill_Fill.png", 591, 712)) {
      printf("%s(%d): Error loading Pill_Fill.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   if(!new_button(&s_player, true, "Graphics/Buttons/Button.png", 12, 612)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   for(int i = 0; i < 4; i++) {
      if(!new_button(&s_ghost[i], true, "Graphics/Buttons/Button.png", 168 + 156 * i, 612)) {
         printf("%s(%d): Error loading Button.png for s_ghost[%d].\n", __FILE__, __LINE__, i);
         goto SHUTDOWN;
      }
   }
   if(!new_button(&s_pickup, true, "Graphics/Buttons/Button.png", 12, 662)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   if(!new_button(&s_ppill, true, "Graphics/Buttons/Button.png", 168, 662)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   if(!new_button(&s_teleport, true, "Graphics/Buttons/Button.png", 324, 662)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   if(!new_button(&s_symx, true, "Graphics/Buttons/Button.png", 480, 662)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   if(!new_button(&s_symy, true, "Graphics/Buttons/Button.png", 636, 662)) {
      printf("%s(%d): Error loading Button.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   if(!new_button(&b_ll_arrow, false, "Graphics/Buttons/Left_Arrow.png", 6, 712)) {
      printf("%s(%d): Error loading Left_Arrow.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }
   if(!new_button(&b_rl_arrow, false, "Graphics/Buttons/Right_Arrow.png", 552, 712)) {
      printf("%s(%d): Error loading Right_Arrow.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }

   printf("Initializing new level.\n");
   LEVEL level;
   initialize_map(&level);
   memset(error, 0, MAPY * MAPX);

   // set line_set and background separately because when
   // we use NEW button, we don't want to change them.
   level.line_set = 0;
   level.background = 0;

   printf("Registering events.\n");
   al_register_event_source(queue, al_get_mouse_event_source());
   al_register_event_source(queue, al_get_keyboard_event_source());
   al_register_event_source(queue, al_get_display_event_source(setting.screen));
   al_register_event_source(queue, al_get_timer_event_source(setting.redraw_timer));
   al_start_timer(setting.redraw_timer);

   ALLEGRO_KEYBOARD_STATE keys;
   ALLEGRO_MOUSE_STATE mouse;

   printf("Starting main loop.\n");
   while(!done) {
      ALLEGRO_EVENT event;
      al_wait_for_event(queue, &event);

      if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
         if(!edit.saved) {
            printf("Close button detected.\n");
            int selection = al_show_native_message_box(setting.screen, "Warning", "CHANGES NOT SAVED YET",
                            "Are you sure?", NULL, ALLEGRO_MESSAGEBOX_OK_CANCEL | ALLEGRO_MESSAGEBOX_WARN);
            if(selection == 1) {
               done = true;
            }
         }
         else {
            done = true;
         }
      }

      if(event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
         al_get_mouse_state(&mouse);

         // Convert mouse position to proper position on scaled screen.
         mouse_x = (mouse.x - offset_x) / scale_x;
         mouse_y = (mouse.y - offset_y) / scale_y;

         // New button
         if(check_button(&b_new, &mouse)) {
            setting.redraw |= 1;
            if(b_new.state == 2) {
               initialize_map(&level);
               memset(error, 0, MAPY * MAPX);
               edit.saved = true;
               level.validated = false;
               sprintf(message, "NO");
            }
         }

         // Load button
         if(check_button(&b_load, &mouse)) setting.redraw |= 1;

         // Save button
         if(check_button(&b_save, &mouse)) setting.redraw |= 1;

         // Quit button
         if(check_button(&b_quit, &mouse)) setting.redraw |= 1;

         // Grid button
         if(check_button(&b_grid, &mouse)) {
            setting.redraw |= 1;
            if(b_grid.state == 2) edit.grid = !edit.grid;
         }

         // Validate button
         if(check_button(&b_validate, &mouse)) {
            setting.redraw |= 1;
            if(b_validate.state == 2) level.validated = validate(message, error, &level);
         }

         // Pill Fill button
         if(check_button(&b_pfill, &mouse)) {
            setting.redraw |= 1;
            if(b_pfill.state == 2) {
               pill_fill(&level, &edit);
               edit.saved = false;  // changes not saved yet
               level.validated = false; // level needs to be re-validated
               sprintf(message, "NO");
            }
         }

         // Left Arrow button
         if(check_button(&b_ll_arrow, &mouse)) {
            setting.redraw |= 1;
            if(b_ll_arrow.state == 2) {
               if(level.line_set) level.line_set--;
               else level.line_set = max_linesets-1;
            }
         }

         // Right Arrow button
         if(check_button(&b_rl_arrow, &mouse)) {
            setting.redraw |= 1;
            if(b_rl_arrow.state == 2) {
               level.line_set++;
               if(level.line_set == max_linesets) level.line_set = 0;
            }
         }

         // Player button
         if(check_button(&s_player, &mouse)) {
            setting.redraw |= 1;
            if(s_player.state == 2) {
               edit.mode = PLAYER;
               s_pickup.state = 0;
               s_ppill.state = 0;
               s_teleport.state = 0;
               for(int i = 0; i < 4; i++) s_ghost[i].state = 0;
            }
            else if(s_player.old_state == 2) edit.mode = LINE;
         }

         // SymX button
         if(check_button(&s_symx, &mouse)) setting.redraw |= 1;

         // SymY button
         if(check_button(&s_symy, &mouse)) setting.redraw |= 1;

         // Ghost buttons
         for(int i = 0; i < 4; i++) {
            if(check_button(&s_ghost[i], &mouse)) {
               setting.redraw |= 1;
               if(s_ghost[i].state == 2) {
                  s_player.state = 0;
                  s_pickup.state = 0;
                  s_ppill.state = 0;
                  s_teleport.state = 0;
                  for(int j = 0; j < 4; j++) {
                     if(i != j) {
                        s_ghost[j].state = 0;
                        s_ghost[j].old_state = 0;
                     }
                  }
                  edit.mode = GHOST0 + i;
               }
               else if(s_ghost[i].old_state == 2) {
                  edit.mode = LINE;
               }
            }
         }

         // Pickup button
         if(check_button(&s_pickup, &mouse)) {
            setting.redraw |= 1;
            if(s_pickup.state == 2) {
               s_ppill.state = 0;
               s_player.state = 0;
               s_teleport.state = 0;
               for(int i = 0; i < 4; i++) {
                  s_ghost[i].state = 0;
                  s_ghost[i].old_state = 0;
               }
               edit.mode = PICKUP;
            }
            else if(s_pickup.old_state == 2) {
               edit.mode = LINE;
            }
         }

         // Powerpill button
         if(check_button(&s_ppill, &mouse)) {
            setting.redraw |= 1;
            if(s_ppill.state == 2) {
               s_player.state = 0;
               s_pickup.state = 0;
               s_teleport.state = 0;
               for(int i = 0; i < 4; i++) {
                  s_ghost[i].state = 0;
                  s_ghost[i].old_state = 0;
               }
               edit.mode = POWERPILL;
            }
            else if(s_ppill.old_state == 2) {
               edit.mode = LINE;
            }
         }

         // Teleport button
         if(check_button(&s_teleport, &mouse)) {
            setting.redraw |= 1;
            if(s_teleport.state == 2) {
               s_player.state = 0;
               s_pickup.state = 0;
               s_ppill.state = 0;
               for(int i = 0; i < 4; i++) {
                  s_ghost[i].state = 0;
                  s_ghost[i].old_state = 0;
               }
               edit.mode = TELEPORT;
            }
            else if(s_teleport.old_state == 2) {
               edit.mode = LINE;
            }
         }

         // Middle mouse button ANYWHERE toggles between line and pill mode
         if(mouse.buttons & 4) {
            edit.mode = (edit.mode == LINE) ? PILL : LINE;
            setting.redraw |= 1;
         }

         // Main Map Screen
         if(((mouse_x > 11) && (mouse_x < 748)) && ((mouse_y > 47) && (mouse_y < 592))) {
            edit.x = (mouse_x - 11) / 32;
            edit.y = (mouse_y - 47) / 32;

            edit.oldx = edit.x;
            edit.oldy = edit.y;

            if(s_symx.state == 2) sedit.x = MAPX - 1 - edit.x;
            else sedit.x = edit.x;

            if(s_symy.state == 2) sedit.y = MAPY - 1 - edit.y;
            else sedit.y = edit.y;

            sedit.oldx = sedit.x;
            sedit.oldy = sedit.y;

            if(mouse.buttons & 1) {       // Left Mouse Button
               switch(edit.mode) {
                  case GHOST0:
                  case GHOST1:
                  case GHOST2:
                  case GHOST3:
                     remove_tile(&level, &edit);   // remove tile BEFORE setting a new one
                     if(level.ghost[edit.mode].x < 99) // reset old position if it was previously set.
                        level.map[level.ghost[edit.mode].y][level.ghost[edit.mode].x].is_protected = false;
                     level.ghost[edit.mode].x = edit.x;
                     level.ghost[edit.mode].y = edit.y;
                     level.map[edit.y][edit.x].is_protected = true;
                     break;
                  case LINE:
                     place_line(&level, &edit);
                     if(s_symx.state == 2 || s_symy.state == 2) place_line(&level, &sedit);
                     break;
                  case PILL:
                     remove_tile(&level, &edit);   // remove tile BEFORE setting a new one
                     level.map[edit.y][edit.x].is_pill = true;
                     level.map[edit.y][edit.x].tile = edit.pill + 1;
                     if(s_symx.state == 2 || s_symy.state == 2) {
                        remove_tile(&level, &sedit);   // remove tile BEFORE setting a new one
                        level.map[sedit.y][sedit.x].is_pill = true;
                        level.map[sedit.y][sedit.x].tile = edit.pill + 1;
                     }
                     break;
                  case POWERPILL:
                     if(level.map[edit.y][edit.x].is_pill) level.map[edit.y][edit.x].is_powerpill = true;
                     break;
                  case TELEPORT:
                     remove_tile(&level, &edit);   // remove tile BEFORE setting a new one
                     if(level.teleport[teleport_number].x < 99) // reset old position if it was previously set.
                        level.map[level.teleport[teleport_number].y][level.teleport[teleport_number].x].is_protected = false;
                     level.teleport[teleport_number].x = edit.x;
                     level.teleport[teleport_number].y = edit.y;
                     level.map[edit.y][edit.x].is_protected = true;
                     teleport_number = !teleport_number;
                     break;
                  case PLAYER:
                     remove_tile(&level, &edit);   // remove tile BEFORE setting a new one
                     if(level.player.x < 99) // reset old position if it was previously set.
                        level.map[level.player.y][level.player.x].is_protected = false;
                     level.player.x = edit.x;
                     level.player.y = edit.y;
                     level.map[edit.y][edit.x].is_protected = true;
                     break;
                  case PICKUP:
                     remove_tile(&level, &edit);
                     if(level.pickup.x < 99) // reset old position if it was previously set.
                        level.map[level.pickup.y][level.pickup.x].is_protected = false;
                     level.pickup.x = edit.x;
                     level.pickup.y = edit.y;
                     level.map[edit.y][edit.x].is_protected = true;
                     break;
                  default:
                     printf("%s(%d): Unknown mode - %d\n", __FILE__, __LINE__, edit.mode);
               }
            }
            else if(mouse.buttons & 2) {  // Right Mouse Button
               remove_tile(&level, &edit);  // clears everything at that tile
               if(s_symx.state == 2 || s_symy.state == 2) remove_tile(&level, &sedit);
            }
            edit.saved = false;  // changes not saved yet
            level.validated = false;
            sprintf(message, "NO");
            setting.redraw = true;
         }

         // Pill selection area 762, 600 - 1019, 761
         if(((mouse_x > 762) && (mouse_x < 1019)) && ((mouse_y > 600) && (mouse_y < 761))) {
            if(mouse.buttons & 1) {
               edit.pillx = (mouse_x - 763) / 32;
               edit.pilly = (mouse_y - 601) / 32;
               edit.pill = edit.pilly * 8 + edit.pillx;
               edit.mode = PILL;
               s_ppill.state = 0;
               s_pickup.state = 0;
               s_teleport.state = 0;
               setting.redraw = true;
            }
         }

         // if Line area is click on, select LINE drawing mode.
         if(((mouse_x > 38) && (mouse_x < 550)) && ((mouse_y > 712) && (mouse_y < 744))) {
            if(mouse.buttons & 1) {
               edit.mode = LINE;
               s_ppill.state = 0;
               s_pickup.state = 0;
               s_teleport.state = 0;
               setting.redraw = true;
            }
         }
      }

      if(event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
         al_get_mouse_state(&mouse);

         // Convert mouse position to proper position on scaled screen.
         mouse_x = (mouse.x - offset_x) / scale_x;
         mouse_y = (mouse.y - offset_y) / scale_y;

         setting.redraw = check_button(&b_new, &mouse);

         // Check LOAD on RELEASE to prevent button getting stuck
         if(check_button(&b_load, &mouse)) {
            setting.redraw |= 1;
            if(b_load.state != 2) {
               loadmap(setting.screen, &level, levelname);
               memset(error, 0, MAPY * MAPX);
               edit.saved = true;
            }
         }

         // Check SAVE on RELEASE to prevent button getting stuck
         if(check_button(&b_save, &mouse)) {
            setting.redraw |= 1;
            if(b_save.state != 2) {
               savemap(setting.screen, &level, levelname);
               edit.saved = true;
            }
         }

         // Check QUIT on RELEASE to prevent button getting stuck
         if(check_button(&b_quit, &mouse)) {
            setting.redraw |= true;
            if(b_quit.state != 2) {
               if(!edit.saved) {
                  int selection = al_show_native_message_box(setting.screen, "Warning",
                                  "CHANGES NOT SAVED YET", "Are you sure?", NULL,
                                  ALLEGRO_MESSAGEBOX_OK_CANCEL | ALLEGRO_MESSAGEBOX_WARN);
                  if(selection == 1) done = true;
                  else mouse.buttons = 0;
               }
               else done = true;
            }
         }

         setting.redraw = check_button(&b_grid, &mouse)     | setting.redraw;
         setting.redraw = check_button(&b_validate, &mouse) | setting.redraw;
         setting.redraw = check_button(&b_ll_arrow, &mouse) | setting.redraw;
         setting.redraw = check_button(&b_rl_arrow, &mouse) | setting.redraw;
         setting.redraw = check_button(&b_pfill, &mouse)    | setting.redraw;

         edit.oldx = edit.x;
         edit.oldy = edit.y;

         sedit.oldx = edit.x;
         sedit.oldy = edit.y;

         // Button released over Level screen area
         if(((mouse_x > 11) && (mouse_x < 748)) && ((mouse_y > 47) && (mouse_y < 592))) {
            switch(edit.mode) {
               case GHOST0:
               case GHOST1:
               case GHOST2:
               case GHOST3:
                  s_ghost[edit.mode].state = 0; // Ghost placed, reset state.
                  setting.redraw = true;
                  edit.mode = LINE;             // Default mode is LINE drawing.
                  break;
               case LINE:
                  break;
               case PILL:
                  break;
               case POWERPILL:
                  break;
               case TELEPORT:
                  s_teleport.state = 0;         // Teleport placed, reset state.
                  setting.redraw = true;
                  edit.mode = LINE;
                  break;
               case PLAYER:
                  s_player.state = 0;           // Player spawn placed, reset state.
                  setting.redraw = true;
                  edit.mode = LINE;
                  break;
               case PICKUP:                     // Pickup spawn placed, reset state.
                  s_pickup.state = 0;
                  setting.redraw = true;
                  edit.mode = LINE;
                  break;
               default:
                  printf("%s(%d): Unknown mode - %d\n", __FILE__, __LINE__, edit.mode);
            }
         }
      }

      // The mouse x or y values changed (mouse moved)
      if(event.type == ALLEGRO_EVENT_MOUSE_AXES) {
         al_get_mouse_state(&mouse);

         // Convert mouse position to proper position on scaled screen.
         mouse_x = (mouse.x - offset_x) / scale_x;
         mouse_y = (mouse.y - offset_y) / scale_y;

         // Check to see if the mouse moved over any buttons.
         // Redraw the screen if it did as they will change their look.
         if(check_button(&b_new, &mouse))      setting.redraw = true;
         if(check_button(&b_load, &mouse))     setting.redraw = true;
         if(check_button(&b_save, &mouse))     setting.redraw = true;
         if(check_button(&b_quit, &mouse))     setting.redraw = true;
         if(check_button(&b_grid, &mouse))     setting.redraw = true;
         if(check_button(&b_validate, &mouse)) setting.redraw = true;
         if(check_button(&b_ll_arrow, &mouse)) setting.redraw = true;
         if(check_button(&b_rl_arrow, &mouse)) setting.redraw = true;
         if(check_button(&b_pfill, &mouse))    setting.redraw = true;

         if(mouse.buttons == 0) { // make sure the left button isn't pressed first
            if(s_player.state != 2 && check_button(&s_player, &mouse))        setting.redraw = true;
            if(s_symx.state != 2   && check_button(&s_symx, &mouse))          setting.redraw = true;
            if(s_symy.state != 2   && check_button(&s_symy, &mouse))          setting.redraw = true;
            if(s_pickup.state != 2 && check_button(&s_pickup, &mouse))        setting.redraw = true;
            if(s_ppill.state != 2  && check_button(&s_ppill, &mouse))         setting.redraw = true;
            if(s_teleport.state != 2  && check_button(&s_teleport, &mouse))   setting.redraw = true;
            for(int i = 0; i < 4; i++)
               if(s_ghost[i].state != 2 && check_button(&s_ghost[i], &mouse)) setting.redraw = true;
         }

         // Mouse movement detected while over main level map area
         if(((mouse_x > 11) && (mouse_x < 748)) && ((mouse_y > 47) && (mouse_y < 592))) {
            if(mouse.buttons & 1) {    // Left Mouse Button (draw/place tile)
               if(edit.mode == LINE || edit.mode == PILL || edit.mode == POWERPILL) {
                  if(edit.oldx != edit.x) {
                     edit.oldx = edit.x;
                     sedit.oldx = sedit.x;
                  }
                  if(edit.oldy != edit.y) {
                     edit.oldy = edit.y;
                     sedit.oldy = sedit.y;
                  }
                  edit.x = (mouse_x - 11) / 32;
                  edit.y = (mouse_y - 47) / 32;
                  if(s_symx.state == 2) sedit.x = MAPX - 1 - edit.x;
                  else sedit.x = edit.x;
                  if(s_symy.state == 2) sedit.y = MAPY - 1 - edit.y;
                  else sedit.y = edit.y;
                  if(edit.x != edit.oldx || edit.y != edit.oldy) {
                     if(edit.mode == PILL) {
                        remove_tile(&level, &edit);   // remove tile BEFORE setting a new one
                        level.map[edit.y][edit.x].is_pill = true;
                        level.map[edit.y][edit.x].tile = edit.pill + 1;
                        if(s_symx.state == 2 || s_symy.state == 2) {
                           remove_tile(&level, &sedit);   // remove tile BEFORE setting a new one
                           level.map[sedit.y][sedit.x].is_pill = true;
                           level.map[sedit.y][sedit.x].tile = edit.pill + 1;
                        }
                     }
                     else if(edit.mode == LINE) {
                        place_line(&level, &edit);
                        if(s_symx.state == 2 || s_symy.state == 2) place_line(&level, &sedit);
                     }
                     else { // POWERPILL
                        if(level.map[edit.y][edit.x].is_pill) level.map[edit.y][edit.x].is_powerpill = true;
                     }
                     edit.saved = false;  // Changes not saved yet.
                     level.validated = false;   // The level changed, so we need to validate it again.
                     sprintf(message, "NO");
                     setting.redraw = true;
                  }
               }
            }
            if(mouse.buttons & 2) {    // Right Mouse Button (erase tile)
               edit.x = (mouse_x - 11) / 32;
               edit.y = (mouse_y - 47) / 32;
               edit.oldx = edit.x;
               edit.oldy = edit.y;
               if(s_symx.state == 2) sedit.x = MAPX - 1 - edit.x;
               else sedit.x = edit.x;
               if(s_symy.state == 2) sedit.y = MAPY - 1 - edit.y;
               else sedit.y = edit.y;
               remove_tile(&level, &edit);
               if(s_symx.state == 2 || s_symy.state == 2) remove_tile(&level, &sedit);
               edit.saved = false;  // changes not saved yet
               level.validated = false;
               sprintf(message, "NO");
               setting.redraw = true;
            }
         }

         if(mouse.z < edit.wheel) {
            edit.wheel = mouse.z;
            level.background--;
            if(level.background == 255) level.background = 19;
            setting.redraw = true;
         }
         else if(mouse.z > edit.wheel) {
            edit.wheel = mouse.z;
            level.background++;
            if(level.background >= 20) level.background = 0;
            setting.redraw = true;
         }
      }

      // check for key input and process it
      if(event.type == ALLEGRO_EVENT_KEY_DOWN) {
         al_get_keyboard_state(&keys);
         TILE temp_row[MAPX];
         switch(event.keyboard.keycode) {
            case ALLEGRO_KEY_ESCAPE:
               if(!edit.saved) {
                  int button = al_show_native_message_box(setting.screen, "Warning",
                                                          "CHANGES NOT SAVED YET", "Are you sure?", NULL,
                                                          ALLEGRO_MESSAGEBOX_OK_CANCEL | ALLEGRO_MESSAGEBOX_WARN);
                  if(button == 1) done = true;
               }
               else done = true;
               break;
            case ALLEGRO_KEY_G:
               edit.grid = !edit.grid;
               setting.redraw = true;
               break;
            case ALLEGRO_KEY_V:
               level.validated = validate(message, error, &level);
               setting.redraw = true;
               break;
            case ALLEGRO_KEY_X:
               if(s_symx.state == 2) s_symx.state = 0;
               else if(s_symx.state == 0) s_symx.state = 2;
               setting.redraw = true;
               break;
            case ALLEGRO_KEY_Y:
               if(s_symy.state == 2) s_symy.state = 0;
               else if(s_symy.state == 0) s_symy.state = 2;
               setting.redraw = true;
               break;
            case ALLEGRO_KEY_LEFT:     // Shift entire map to the left (wrapping it around)
            case ALLEGRO_KEY_PAD_4:
            case ALLEGRO_KEY_A:
               for(int y = 0; y < MAPY; y++) {
                  // copy first map cell into temp storage
                  memcpy(&temp_row[0], &level.map[y][0], sizeof(level.map[y][0]));
                  // copy each cell one to the left (overwriting the first)
                  for(int x = 0; x < MAPX-1; x++)
                     memcpy(&level.map[y][x], &level.map[y][x+1], sizeof(level.map[y][x+1]));
                  // copy the first cell (in temp) to the last cell
                  memcpy(&level.map[y][MAPX-1], &temp_row[0], sizeof(temp_row[0]));
               }

               // if the player start has been set, move it to the left one
               if(level.player.x < 99) {
                  if(level.player.x > 0) level.player.x--;
                  else level.player.x = MAPX-1;       // if the player moves off the left edge, wrap them to the right
               }
               for(int i = 0; i < 4; i++) {
                  // if the ghost start has been set, move it to the left one
                  if(level.ghost[i].x < 99) {
                     if(level.ghost[i].x > 0) level.ghost[i].x--;
                     else level.ghost[i].x = MAPX-1;   // If the start moves off the left edge, wrap it to the right
                  }
               }
               // If the pickup spawn has been set, move it to the left one
               if(level.pickup.x < 99) {
                  if(level.pickup.x > 0) level.pickup.x--;
                  else level.pickup.x = MAPX-1; // If the spawn moves off the left edge, wrap it to the right
               }
               for(int i = 0; i < 2; i++) {
                  // if the teleport has been set, move it to the left one
                  if(level.teleport[i].x < 99) {
                     if(level.teleport[i].x > 0) level.teleport[i].x--;
                     else level.teleport[i].x = MAPX-1;   // If the teleport moves off the left edge, wrap it to the right
                  }
               }
               level.validated = false;   // map has changed, it will need to be validated again
               edit.saved = false;  // changes not saved yet
               setting.redraw = true;     // map has changed, redraw it
               break;
            case ALLEGRO_KEY_RIGHT:    // Shift entire map to the right (wrapping it around)
            case ALLEGRO_KEY_PAD_6:
            case ALLEGRO_KEY_D:
               for(int y = 0; y < MAPY; y++) {
                  // copy the last map cell into temp storage
                  memcpy(&temp_row[0], &level.map[y][MAPX-1], sizeof(level.map[y][MAPX-1]));
                  // copy each cell one to the right (overwriting the last)
                  for(int x = MAPX-1; x > 0; x--)
                     memcpy(&level.map[y][x], &level.map[y][x-1], sizeof(level.map[y][x-1]));
                  // copy the last cell (in temp) to the first cell
                  memcpy(&level.map[y][0], &temp_row[0], sizeof(temp_row[0]));
               }

               // if the player start has been set, move it to the right one
               if(level.player.x < 99) {
                  if(level.player.x < MAPX-1) level.player.x++;
                  else level.player.x = 0;       // If the player moves off the right edge, wrap them to the left.
               }
               for(int i = 0; i < 4; i++) {
                  // If the ghost start has been set, move it to the right one.
                  if(level.ghost[i].x < 99) {
                     if(level.ghost[i].x < MAPX-1) level.ghost[i].x++;
                     else level.ghost[i].x = 0;   // If the start moves off the right edge, wrap it to the left.
                  }
               }
               // If the pickup spawn has been set, move it to the right one.
               if(level.pickup.x < 99) {
                  if(level.pickup.x < MAPX-1) level.pickup.x++;
                  else level.pickup.x = 0; // If the spawn moves off the right edge, wrap it to the left.
               }
               for(int i = 0; i < 2; i++) {
                  // If the teleport has been set, move it to the right one.
                  if(level.teleport[i].x < 99) {
                     if(level.teleport[i].x < MAPX-1) level.teleport[i].x++;
                     else level.teleport[i].x = 0;   // If the teleport moves off the right edge, wrap it to the left.
                  }
               }
               level.validated = false;   // map has changed, it will need to be validated again
               edit.saved = false;  // changes not saved yet
               setting.redraw = true;     // map has changed, redraw it
               break;
            case ALLEGRO_KEY_UP:       // Shift entire map up (wrapping it around)
            case ALLEGRO_KEY_PAD_8:
            case ALLEGRO_KEY_W:
               // copy the top row to temp
               memcpy(temp_row, level.map[0], sizeof(level.map[0]));
               // move each row up one
               for(int y = 0; y < MAPY-1; y++) memcpy(level.map[y], level.map[y+1], sizeof(level.map[y]));
               // copy the top row (in temp) to the bottom
               memcpy(level.map[MAPY-1], temp_row, sizeof(temp_row));

               // if the player start has been set, move it up one
               if(level.player.y < 99) {
                  if(level.player.y > 0) level.player.y--;
                  else level.player.y = MAPY-1;       // if the player moves off the top edge, wrap them to the bottom
               }
               for(int i = 0; i < 4; i++) {
                  // if the ghost start has been set, move it up one
                  if(level.ghost[i].y < 99) {
                     if(level.ghost[i].y > 0) level.ghost[i].y--;
                     else level.ghost[i].y = MAPY-1;   // If the start moves off the top edge, wrap it to the bottom
                  }
               }
               // If the pickup spawn has been set, move it up one
               if(level.pickup.y < 99) {
                  if(level.pickup.y > 0) level.pickup.y--;
                  else level.pickup.y = MAPY-1; // If the spawn moves off the top edge, wrap it to the bottom
               }
               for(int i = 0; i < 2; i++) {
                  // If the teleport has been set, move it up one.
                  if(level.teleport[i].y < 99) {
                     if(level.teleport[i].y > 0) level.teleport[i].y--;
                     else level.teleport[i].y = MAPY-1;   // If the teleport moves off the top edge, wrap it to the bottom.
                  }
               }
               level.validated = false;   // map has changed, it will need to be validated again
               edit.saved = false;  // changes not saved yet
               setting.redraw = true;     // map has changed, redraw it
               break;
            case ALLEGRO_KEY_DOWN:     // Shift entire map down (wrapping it around)
            case ALLEGRO_KEY_PAD_2:
            case ALLEGRO_KEY_S:
               // copy the bottom row to temp
               memcpy(temp_row, level.map[MAPY-1], sizeof(level.map[MAPY-1]));
               // move each row down one
               for(int y = MAPY-1; y > 0; y--) memcpy(level.map[y], level.map[y-1], sizeof(level.map[y]));
               // copy the bottom row (in temp) to the top
               memcpy(level.map[0], temp_row, sizeof(temp_row));

               // If the player start has been set, move it down one.
               if(level.player.y < 99) {
                  if(level.player.y < MAPY-1) level.player.y++;
                  else level.player.y = 0;       // If the player moves off the bottom edge, wrap them to the top.
               }
               for(int i = 0; i < 4; i++) {
                  // If the ghost start has been set, move it down one.
                  if(level.ghost[i].y < 99) {
                     if(level.ghost[i].y < MAPY-1) level.ghost[i].y++;
                     else level.ghost[i].y = 0;   // If the start moves off the bottom edge, wrap it to the top.
                  }
               }
               // If the pickup spawn has been set, move it down one.
               if(level.pickup.y < 99) {
                  if(level.pickup.y < MAPY-1) level.pickup.y++;
                  else level.pickup.y = 0; // If the spawn moves off the bottom edge, wrap it to the top.
               }
               for(int i = 0; i < 2; i++) {
                  // If the teleport has been set, move it down one.
                  if(level.teleport[i].y < 99) {
                     if(level.teleport[i].y < MAPY-1) level.teleport[i].y++;
                     else level.teleport[i].y = 0;   // If the teleport moves off the bottom edge, wrap it to the top.
                  }
               }
               level.validated = false;   // map has changed, it will need to be validated again
               edit.saved = false;  // changes not saved yet
               setting.redraw = true;     // map has changed, redraw it
               break;
         }
      }

      // Redraw the screen, but only after the event queue is empty
      if(setting.redraw && al_is_event_queue_empty(queue)) {
         setting.redraw = false;

         al_set_target_bitmap(al_get_backbuffer(setting.screen));

         // clear to brown background
         al_clear_to_color(al_map_rgb_f(.3, .15, 0));

         for(int y = 0; y < MAPY; y++)
            for(int x = 0; x < MAPX; x++)
               al_draw_bitmap(background[level.background], 12 + x * 32, 48 + y * 32, 0);

         level.pills = 0;
         for(int y = 0; y < MAPY; y++) {
            for(int x = 0; x < MAPX; x++) {
               int xpos = 12 + x * 32;
               int ypos = 48 + y * 32;
               if(!level.map[y][x].is_pill && level.map[y][x].tile) {
                  int l = level.map[y][x].tile - 1;
                  al_draw_tinted_bitmap(lines[level.line_set][l], al_map_rgba_f(0, 0, 0, .5), xpos + 5, ypos + 5, 0);
                  al_draw_bitmap(lines[level.line_set][l], xpos, ypos, 0);
               }
               else if(level.map[y][x].is_pill && level.map[y][x].tile) {
                  int p = level.map[y][x].tile - 1;
                  int py = (int)(p / 8);
                  int px = p - py * 8;
                  al_draw_tinted_bitmap(pills[py][px], al_map_rgba_f(0, 0, 0, .5), xpos + 5, ypos + 5, 0);
                  al_draw_bitmap(pills[py][px], xpos, ypos, 0);
                  level.pills++;
               }
               if(level.map[y][x].is_powerpill) {
                  al_draw_filled_rectangle(xpos + 1, ypos + 1, xpos + 31, ypos + 31, al_map_rgba_f(.5, .5, .5, .5));
                  al_draw_rectangle(xpos + 1, ypos + 1, xpos + 31, ypos + 31, al_map_rgba_f(0, 1, 0, 1), 1);
               }
               if(error[y][x]) {
                  al_draw_filled_rectangle(xpos, ypos, xpos + 32, ypos + 32, al_map_rgba_f(.5, 0, 0, .5));
               }
            }
         }

         if(level.pills > 0)
            al_draw_textf(font, al_map_rgb_f(1, 1, 1), 12, 6, ALLEGRO_ALIGN_LEFT, "Pills: %03d", level.pills);
         else {
            al_draw_text(font, al_map_rgb_f(1, 1, 1), 12, 6, ALLEGRO_ALIGN_LEFT, "Pills: ");
            al_draw_text(font, al_map_rgb_f(1, 0, 0), 83, 6, ALLEGRO_ALIGN_LEFT, "000");
         }

         al_draw_text(font, al_map_rgb_f(1, 1, 1), 172, 6, ALLEGRO_ALIGN_LEFT, "Validated: ");
         if(level.validated)
            al_draw_text(font, al_map_rgb_f(0, 1, 0), 305, 6, ALLEGRO_ALIGN_LEFT, "YES");
         else
            al_draw_textf(font, al_map_rgb_f(1, 0, 0), 305, 6, ALLEGRO_ALIGN_LEFT, "%s", message);

         al_draw_text(font, al_map_rgb_f(1, 1, 1), 623, 6, ALLEGRO_ALIGN_LEFT, "Filename: ");
         al_draw_textf(font, al_map_rgb_f(1, 1, 0), 768, 6, ALLEGRO_ALIGN_LEFT, "%s", levelname);

         if(level.player.x < 99) {
            al_draw_tinted_bitmap(pacman, al_map_rgba_f(0, 0, 0, 0.5),
                                  12 + level.player.x * 32 - 5, 48 + level.player.y * 32 - 5, 0);
            al_draw_bitmap(pacman, 12 + level.player.x * 32 - 9, 48 + level.player.y * 32 - 9, 0);
         }

         for(int i = 0; i < 4; i++) {
            if(level.ghost[i].x < 99) {
               int x = 12 + level.ghost[i].x * 32;
               int y = 48 + level.ghost[i].y * 32;
               al_draw_tinted_bitmap(ghost[i], al_map_rgba_f(0, 0, 0, 0.5), x - 4, y - 4, 0);
               al_draw_bitmap(ghost[i], x - 9, y - 9, 0);
            }
         }

         for(int i = 0; i < 2; i++) {
            if(level.teleport[i].x < 99) {
               int x = 12 + level.teleport[i].x * 32;
               int y = 48 + level.teleport[i].y * 32;
               al_draw_bitmap(teleport, x, y, 0);
            }
         }

         if(level.pickup.x < 99) {
            al_draw_tinted_bitmap(pickup, al_map_rgba_f(0, 0, 0, .5), 12 + level.pickup.x * 32 - 4, 48 + level.pickup.y * 32 - 4, 0);
            al_draw_bitmap(pickup, 12 + level.pickup.x * 32 - 9, 48 + level.pickup.y * 32 - 9, 0);
         }

         al_draw_rectangle(12, 48, 748, 592, al_map_rgb_f(1, 1, 1), 1);

         if(edit.grid) {
            for(int x = 44; x < 748; x += 32) al_draw_line(x, 48, x, 592, al_map_rgb_f(1, 1, 1), 1);
            for(int y = 48; y < 592; y += 32) al_draw_line(12, y, 748, y, al_map_rgb_f(1, 1, 1), 1);
         }

         draw_button(&b_new);
         draw_button(&b_load);
         draw_button(&b_save);
         draw_button(&b_quit);
         draw_button(&b_grid);
         draw_button(&b_validate);
         draw_button(&b_pfill);
         draw_button(&s_player);
         al_draw_text(font, al_map_rgb_f(1, 1, 1), 45, 612, ALLEGRO_ALIGN_LEFT, "Player");
         for(int i = 0; i < 4; i++) {
            draw_button(&s_ghost[i]);
            al_draw_textf(font, ghost_color[i], 202 + i * 156, 612, ALLEGRO_ALIGN_LEFT, "Ghost");
         }
         draw_button(&s_pickup);
         al_draw_text(font, al_map_rgb_f(1, 1, 1), 45, 662, ALLEGRO_ALIGN_LEFT, "Pickup");

         draw_button(&s_ppill);
         al_draw_text(font, al_map_rgb_f(1, 1, 1), 201, 662, ALLEGRO_ALIGN_LEFT, "PPill");

         draw_button(&s_teleport);
         al_draw_text(font, al_map_rgb_f(1, 1, 1), 357, 662, ALLEGRO_ALIGN_LEFT, "Teleport");

         draw_button(&s_symx);
         al_draw_text(font, al_map_rgb_f(1, 1, 1), 513, 661, ALLEGRO_ALIGN_LEFT, "SymX");

         draw_button(&s_symy);
         al_draw_text(font, al_map_rgb_f(1, 1, 1), 669, 661, ALLEGRO_ALIGN_LEFT, "SymY");

         // Line selection bar at bottom (X:38 - 518 (15*32+38), Y:712 - 744)
         draw_button(&b_ll_arrow);
         for(int x = 0; x < 16; x++) al_draw_bitmap(background[level.background], 38 + x * 32, 712, 0);
         for(int x = 0; x < 16; x++) al_draw_bitmap(lines[level.line_set][x], 38 + x * 32, 712, 0);
         if(edit.mode != LINE) al_draw_rectangle(37.5, 711.5, 550.5, 744.5, al_map_rgb_f(1, 0, 0), 1);
         else al_draw_rectangle(37.5, 711.5, 550.5, 744.5, al_map_rgb_f(0, 1, 0), 1);
         //al_draw_rectangle(38.5, 712.5, 69.5, 743.5, al_map_rgb_f(1, 1, 1), 1);
         draw_button(&b_rl_arrow);

         // Draw pill selection area @762x623 (5 rows of 8)
         for(int y = 0; y < 5; y++) {
            for(int x = 0; x < 8; x++) {
               al_draw_bitmap(background[level.background], 762 + x * 32, 600 + y * 32, 0);
               al_draw_bitmap(pills[y][x], 762 + x * 32, 600 + y * 32, 0);
            }
         }
         if(edit.mode != PILL) {
            al_draw_rectangle(762, 600, 1019, 761, al_map_rgb_f(1, 0, 0), 1);
         }
         else {
            al_draw_rectangle(762, 600, 1019, 761, al_map_rgb_f(0, 1, 0), 1);
         }
         // Draw selection box for currently selected pill
         x = 762 + edit.pillx * 32;
         y = 600 + edit.pilly * 32;
         al_draw_rectangle(x, y, x + 33, y + 33, al_map_rgb_f(1, 1, 1), 1);

         al_flip_display();
      }
   }


SHUTDOWN:
   // clean up and exit
   printf("Cleaning up and Exiting.\n");

   fclose(stdout);

   if(queue) {
      al_destroy_event_queue(queue);
      queue = NULL;
   }

   if(setting.icon) {
      al_destroy_bitmap(setting.icon);
      setting.icon = NULL;
   }

   for(int i = 0; i < 20; i++) {
      if(background[i]) {
         al_destroy_bitmap(background[i]);
         background[i] = NULL;
      }
   }

   if(font) {
      al_destroy_font(font);
      font = NULL;
   }

   if(cursor) {
      al_destroy_mouse_cursor(cursor);
      cursor = NULL;
   }

   if(hand) {
      al_destroy_bitmap(hand);
      hand = NULL;
   }

   if(tbmp) {
      al_destroy_bitmap(tbmp);
      tbmp = NULL;
   }

   if(pacman) {
      al_destroy_bitmap(pacman);
      pacman = NULL;
   }

   if(pickup) {
      al_destroy_bitmap(pickup);
      pickup = NULL;
   }

   if(teleport) {
      al_destroy_bitmap(teleport);
      teleport = NULL;
   }

   for(int i = 0; i < max_linesets; i++) {
      for(int j = 0; j < 16; j++) {
         if(lines[i][j]) {
            al_destroy_bitmap(lines[i][j]);
            lines[i][j] = NULL;
         }
      }
   }

   for(int y = 0; y < 5; y++) {
      for(int x = 0; x < 8; x++) {
         if(pills[y][x]) {
            al_destroy_bitmap(pills[y][x]);
            pills[y][x] = NULL;
         }
      }
   }

   destroy_button(&b_new);
   destroy_button(&b_load);
   destroy_button(&b_save);
   destroy_button(&b_quit);
   destroy_button(&b_validate);
   destroy_button(&b_ll_arrow);
   destroy_button(&b_rl_arrow);
   destroy_button(&b_pfill);
   destroy_button(&s_player);
   destroy_button(&s_symx);
   destroy_button(&s_symy);

   for(int i = 0; i < 4; i++) {
      destroy_button(&s_ghost[i]);
      al_destroy_bitmap(ghost[i]);
      ghost[i] = NULL;
   }

   destroy_button(&s_pickup);
   destroy_button(&s_ppill);
   destroy_button(&s_teleport);

   return 0;
}
