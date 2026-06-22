//
// Hiscore table:
//
//    Various functions for handling the hiscore screen, name input etc.
//

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <ctype.h>      // toupper()
#include "dp2_hiscore.h"
#include "dp2_main.h"
#include "a5_error.h"

#define MAX_NAME_LEN       625
#define NUM_SCORES         5
#define TIME_LEN           80
#define HI_MAGIC           0x01201965

typedef struct HISCORES {
   ALLEGRO_USTR *name;
   unsigned long  score;
   int            level;
   char           time[TIME_LEN];   // Adjust the size to match characters needed for storage
} HISCORES;

static HISCORES hiscore[DIFFICULTY][NUM_SCORES];

static ALLEGRO_USTR *yourname;

ALLEGRO_FILE *hiscore_file[DIFFICULTY];


// Initialises the hiscore system
bool init_hiscore(void)
{
   ALLEGRO_STATE state;
   char filename[TEXT_BUFFER] = "";
   yourname = al_ustr_new("");
   char *cstrname;
   int cstrsize;

   al_store_state(&state, ALLEGRO_STATE_NEW_FILE_INTERFACE);
   al_set_standard_file_interface();

   for(int i = 0; i < DIFFICULTY; i++) {
      //snprintf(filename, TEXT_BUFFER, "%s\\Deluxe Pacman\\Deluxe Pacman 2.hi%i", getenv("appdata"), i);
      snprintf(filename, TEXT_BUFFER, "Deluxe Pacman 2.hi%i", i);

      // initialize ustr name variable
      for(int j = 0; j < NUM_SCORES; j++) hiscore[i][j].name = al_ustr_new("");


      // hiscore[0] = easy, hiscore[1] = medium, hiscore[2] = hard
      hiscore_file[i] = NULL;
      hiscore_file[i] = al_fopen(filename, "rb");
      if(hiscore_file[i]) {  // opened hiscore, read in the names and scores
         if(al_fread32le(hiscore_file[i]) == HI_MAGIC) {
            // read in the top scores for this difficulty
            for(int j = 0; j < NUM_SCORES; j++) {
               // read the name and size in
               cstrsize = al_fread32le(hiscore_file[i]);
               cstrname = (char *)malloc(cstrsize);  // allocate memory for name
               if(cstrname == NULL) {
                  al_fclose(hiscore_file[i]);
                  printf("%s(%d): memory allocation failed.\n", __FILE__, __LINE__);
                  return true;
               }
               al_fread(hiscore_file[i], cstrname, cstrsize);
               // assign the c string to the ustr variable from buffer
               hiscore[i][j].name = al_ustr_new_from_buffer(cstrname, cstrsize);
               free(cstrname);
               // read in score as an unsigned long
               al_fread(hiscore_file[i], &hiscore[i][j].score, sizeof(unsigned long));
               // read in the level as an int
               al_fread(hiscore_file[i], &hiscore[i][j].level, sizeof(int));
               // read in time
               cstrsize = al_fread32le(hiscore_file[i]);
               al_fread(hiscore_file[i], hiscore[i][j].time, cstrsize);
            }
         }
         else clear_hiscores(i);
         al_fclose(hiscore_file[i]);
      }
      else clear_hiscores(i);
   }

   al_restore_state(&state);

   return false;
}


// Clears the hiscore table.
void clear_hiscores(int dif)
{
   char timestr[TIME_LEN] = "JAN 20, 1965";
   for(int i = 0; i < NUM_SCORES; i++) {
      al_ustr_assign_cstr(hiscore[dif][i].name, "empty");
      hiscore[dif][i].score = 0;
      hiscore[dif][i].level = 0;
      snprintf(hiscore[dif][i].time, TIME_LEN, timestr);
   }
}


// Shuts down the hiscore system.
void shutdown_hiscore(void)
{
   // convert ustr to c strings with al_cstr()
   ALLEGRO_STATE state;
   char filename[TEXT_BUFFER] = "";
   int cstrsize;

   al_store_state(&state, ALLEGRO_STATE_NEW_FILE_INTERFACE);
   al_set_standard_file_interface();

   for(int i = 0; i < DIFFICULTY; i++) {
      //snprintf(filename, TEXT_BUFFER, "%s\\Deluxe Pacman\\Deluxe Pacman 2.hi%i", getenv("appdata"), i);
      snprintf(filename, TEXT_BUFFER, "Deluxe Pacman 2.hi%i", i);

      // hiscore[0] = easy, hiscore[1] = medium, hiscore[2] = hard
      hiscore_file[i] = al_fopen(filename, "wb");
      if(hiscore_file[i]) {  // opened hiscore, write the names and scores
         al_fwrite32le(hiscore_file[i], HI_MAGIC);
         // write the top scores for this difficulty
         for(int j = 0; j < NUM_SCORES; j++) {
            // write the name as a c string
            cstrsize = al_ustr_size(hiscore[i][j].name);
            al_fwrite32le(hiscore_file[i], cstrsize);
            al_fwrite(hiscore_file[i], al_cstr(hiscore[i][j].name), cstrsize);
            // write score
            al_fwrite(hiscore_file[i], &hiscore[i][j].score, sizeof(unsigned long));
            // write the level
            al_fwrite(hiscore_file[i], &hiscore[i][j].level, sizeof(int));
            // write time
            cstrsize = strlen(hiscore[i][j].time);
            al_fwrite32le(hiscore_file[i], cstrsize);
            al_fwrite(hiscore_file[i], hiscore[i][j].time, cstrsize);
         }
         al_fclose(hiscore_file[i]);
      }
      else {
         printf("%s(%d): Error saving hiscore file.\n", __FILE__, __LINE__);
      }
      for(int j = 0; j < NUM_SCORES; j++) al_ustr_free(hiscore[i][j].name);
   }

   al_ustr_free(yourname);

   al_restore_state(&state);
}


// Displays the text (name) entry box.
void draw_entry_box(int which)
{
   ALLEGRO_BITMAP *b = al_create_bitmap(623, 23);
   int textHeight = al_get_font_ascent(font_verdana);

   al_set_target_bitmap(b);
   al_clear_to_color(al_map_rgb(0, 96, 0));
   al_draw_line(0.5f, HEIGHT - 1.5f, WIDTH + 0.5f, HEIGHT - 1.5f, al_map_rgb(0, 32, 0), 1.0f);

   al_draw_ustr(font_verdana, al_map_rgb_f(1, 1, 1), 1, 1, ALLEGRO_ALIGN_LEFT, yourname);

   if((int)al_get_time() & 2) {
      int x = al_get_text_width(font_verdana, al_cstr(yourname)) + 1;
      al_draw_filled_rectangle(x, 1, x + 2, textHeight + 4, al_map_rgb(0, 0, 0));
   }

   al_set_target_backbuffer(setting.screen);

   int y = HEIGHT / 2 + (which - NUM_SCORES / 2) * (textHeight * 3) - 5;

   al_draw_bitmap(b, 88, y, ALLEGRO_ALIGN_LEFT);

   al_destroy_bitmap(b);
}


// Displays the hiscore table.
// Returns true if close button is clicked
bool score_table(unsigned long score, int player, int level)
{
   int i, j;
   ALLEGRO_BITMAP *hiscore_back = NULL, *buffer = NULL, *flare = NULL, *hiscore_title = NULL, *tbmp = NULL;
   char player_text[80] = "", difficulty_text[80] = "";
   int myscore = -1;
   ALLEGRO_COLOR fontcolor;
   int difficulty = setting.difficulty;
   time_t rawtime;
   struct tm *timeinfo;
   char timestr[80] = "";
   static float angle1 = -1.0;
   static float angle2 = -1.0;
   bool close_button = false;

   if(angle1 == -1) angle1 = (float)(rand() % 628) / 100.0f;
   if(angle2 == -1) angle2 = (float)(rand() % 628) / 100.0f;

   time(&rawtime);
   timeinfo = localtime(&rawtime);
   strftime(timestr, 80, "%b %d, %Y", timeinfo);

   i = 0;
   while(timestr[i]) {
      timestr[i] = toupper(timestr[i]);
      i++;
   }

   buffer = al_create_bitmap(WIDTH, HEIGHT);
   if(!buffer) {
      a5_error(AT, setting.screen, "Error creating hiscore buffer.");
      shut_down();
      exit(1);
   }

   hiscore_back = al_load_bitmap("Graphics/Trophy.png");
   if(!hiscore_back) {
      a5_error(AT, setting.screen, "Error loading Trophy.png.");
      al_destroy_bitmap(buffer); buffer = NULL;
      shut_down();
      exit(1);
   }
   // 5 pixels added to account for the text not quite being centred.
   float hiscore_x = WIDTH / 2 - al_get_bitmap_width(hiscore_back) / 2 + 5;
   float hiscore_y = HEIGHT / 2 - al_get_bitmap_height(hiscore_back) / 2 + 5;
   al_set_target_bitmap(buffer);
   al_draw_bitmap(hiscore_back, hiscore_x, hiscore_y, 0);
   al_destroy_bitmap(hiscore_back); hiscore_back = NULL;

   hiscore_title = al_load_bitmap("Graphics/Hall_Of_Fame_Title.png");
   if(!hiscore_title) {
      a5_error(AT, setting.screen, "Error loading Hall_Of_Fame_Title.png.");
      al_destroy_bitmap(buffer); buffer = NULL;
      shut_down();
      exit(1);
   }
   // 5 pixels added to account for the text not quite being centred.
   hiscore_x = WIDTH / 2 - al_get_bitmap_width(hiscore_title) / 2 + 5;
   hiscore_y = 0;
   al_draw_bitmap(hiscore_title, hiscore_x, hiscore_y, 0);
   al_destroy_bitmap(hiscore_title); hiscore_title = NULL;

   flare = al_load_bitmap("Graphics/Flare.png");
   if(!flare) {
      a5_error(AT, setting.screen, "Error loading Flare.png.");
      al_destroy_bitmap(buffer); buffer = NULL;
      shut_down();
      exit(1);
   }
   float flare_x = al_get_bitmap_width(flare) / 2;
   float flare_y = al_get_bitmap_height(flare) / 2;
   unsigned char cr = rand() % 200 + 55;
   unsigned char cg = rand() % 200 + 55;
   unsigned char cb = rand() % 200 + 55;

   al_set_target_backbuffer(setting.screen);

   if(player > 0) {
      if(player >= 10) snprintf(player_text, 80, "PLAYER %d - %ld", player - 10, score);
      else snprintf(player_text, 80, "PLAYER %d - %ld", player, score);
   }

   if(player < 10) {
      for(i = 0; i < NUM_SCORES; i++) {
         if(score > hiscore[difficulty][i].score) {
            for(j = NUM_SCORES - 1; j > i; j--) {
               hiscore[difficulty][j].score = hiscore[difficulty][j - 1].score;
               hiscore[difficulty][j].level = hiscore[difficulty][j - 1].level;
               al_ustr_assign(hiscore[difficulty][j].name, hiscore[difficulty][j - 1].name);
               strcpy(hiscore[difficulty][j].time, hiscore[difficulty][j - 1].time);
            }

            hiscore[difficulty][i].score = score;
            hiscore[difficulty][j].level = level;
            al_ustr_assign(hiscore[difficulty][j].name, yourname);
            strcpy(hiscore[difficulty][j].time, timestr);

            myscore = i;
            break;
         }
      }
   }

   switch(difficulty) {
      case(0):
         snprintf(difficulty_text, 80, "~ Easy ~");
         break;
      case(1):
         snprintf(difficulty_text, 80, "~ Medium ~");
         break;
      case(2):
         snprintf(difficulty_text, 80, "~ Hard ~");
         break;
   }

   int textHeight = al_get_font_ascent(font_verdana);

   al_set_target_bitmap(buffer);

   // Shadow
   //al_draw_text(font_amaze, al_map_rgb(0, 0, 0), WIDTH/2+10, 0+SHADOW, ALLEGRO_ALIGN_CENTRE, "Hall Of Fame");
   al_draw_text(font_verdana, al_map_rgb(0, 0, 0), WIDTH / 2 + 10, 85 + SHADOW, ALLEGRO_ALIGN_CENTRE, difficulty_text);
   al_draw_text(font_verdana, al_map_rgb(0, 0, 0), WIDTH / 2 + 10, HEIGHT - 102 + SHADOW, ALLEGRO_ALIGN_CENTRE, player_text);

   for(i = 0; i < NUM_SCORES; i++) {
      int y = HEIGHT / 2 + (i - NUM_SCORES / 2) * (textHeight * 3) - 5;

      fontcolor = al_map_rgb(0, 0, 0);

      al_draw_ustr(font_verdana, fontcolor,
                   WIDTH / 2 + SHADOW, y + SHADOW, ALLEGRO_ALIGN_CENTRE,
                   hiscore[difficulty][i].name);

      if(hiscore[difficulty][i].level) {
         al_draw_textf(font_radio_space_bold, fontcolor,
                       89 + SHADOW, y + textHeight + SHADOW, ALLEGRO_ALIGN_LEFT,
                       "Level %d", hiscore[difficulty][i].level);
      }

      al_draw_textf(font_verdana, fontcolor,
                    WIDTH / 2 + SHADOW, y + textHeight + SHADOW, ALLEGRO_ALIGN_CENTRE,
                    "%ld", hiscore[difficulty][i].score);

      if(hiscore[difficulty][i].level) {
         al_draw_textf(font_radio_space_bold, fontcolor,
                       710 + SHADOW, y + textHeight + SHADOW, ALLEGRO_ALIGN_RIGHT,
                       "%s", hiscore[difficulty][i].time);
      }
   }

   // Colour
   //al_draw_text(font_amaze, al_map_rgb(45, 255, 45), WIDTH/2, 0, ALLEGRO_ALIGN_CENTRE, "Hall Of Fame");
   al_draw_text(font_verdana, al_map_rgb(45, 255, 45), WIDTH / 2, 85, ALLEGRO_ALIGN_CENTRE, difficulty_text);
   al_draw_text(font_verdana, al_map_rgb(45, 255, 45), WIDTH / 2, HEIGHT - 102, ALLEGRO_ALIGN_CENTRE, player_text);

   for(i = 0; i < NUM_SCORES; i++) {
      int y = HEIGHT / 2 + (i - NUM_SCORES / 2) * (textHeight * 3) - 5;

      if(i == myscore) fontcolor = al_map_rgb(255, 0, 0);
      //else fontcolor = al_map_rgb(128, 255-i*32, 128);
      else fontcolor = al_map_rgb(255, 255, 255);

      al_draw_ustr(font_verdana, fontcolor,
                   WIDTH / 2, y, ALLEGRO_ALIGN_CENTRE,
                   hiscore[difficulty][i].name);

      if(hiscore[difficulty][i].level) {
         al_draw_textf(font_radio_space_bold, fontcolor,
                       89, y + textHeight, ALLEGRO_ALIGN_LEFT,
                       "Level %d", hiscore[difficulty][i].level);
      }

      al_draw_textf(font_verdana, fontcolor,
                    WIDTH / 2, y + textHeight, ALLEGRO_ALIGN_CENTRE,
                    "%ld", hiscore[difficulty][i].score);

      if(hiscore[difficulty][i].level) {
         al_draw_textf(font_radio_space_bold, fontcolor,
                       710, y + textHeight, ALLEGRO_ALIGN_RIGHT,
                       "%s", hiscore[difficulty][i].time);
      }
   }

   if(myscore >= 0) draw_entry_box(myscore);

   al_set_target_backbuffer(setting.screen);
   al_clear_to_color(al_map_rgba(0, 0, 0, 255));
   al_draw_tinted_scaled_rotated_bitmap(flare, al_map_rgb(cr, cg, cb), flare_x, flare_y,
                                        WIDTH / 2, HEIGHT / 2, 2, 2, angle1, 0);
   al_draw_tinted_scaled_rotated_bitmap(flare, al_map_rgb(cr, cg, cb), flare_x, flare_y,
                                        WIDTH / 2, HEIGHT / 2, 2, 2, angle2, 0);
   al_draw_bitmap(buffer, 0, 0, 0);
   al_flip_display();

   do al_wait_for_event(event_queue, &event);
   while(event.type == ALLEGRO_EVENT_KEY_DOWN);


   if(myscore >= 0) {
      al_flush_event_queue(event_queue);

      bool done = false;
      bool redraw = true;
      int key;
      int pos = (int)al_ustr_size(yourname);
      int x;

      al_draw_bitmap(buffer, 0, 0, 0);
      draw_entry_box(myscore);
      al_flip_display();

      while(!done) {
         if(pos > 0) al_ustr_assign(hiscore[difficulty][myscore].name, yourname);

         al_wait_for_event(event_queue, &event);
         switch(event.type) {
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
               close_button = true;
               done = true;
               break;
            case ALLEGRO_EVENT_TIMER:
               redraw = true;
               break;
            case ALLEGRO_EVENT_KEY_CHAR:
               if(event.keyboard.unichar >= 32) {
                  if(setting.sound_on) al_play_sample(sfx_type, setting.sound_volume, 0, 1,
                                                         ALLEGRO_PLAYMODE_ONCE, NULL);
                  pos += al_ustr_append_chr(yourname, event.keyboard.unichar);
                  // Get the width of the name in pixels (because TTF varies in width)
                  x = al_get_text_width(font_verdana, al_cstr(yourname));
                  if(x > MAX_NAME_LEN) {  // Check if the width in pixels is greater than the allowed size
                     if(al_ustr_prev(yourname, &pos)) al_ustr_truncate(yourname, pos);
                  }
               }
               else if(event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
                  if(al_ustr_prev(yourname, &pos)) al_ustr_truncate(yourname, pos);
               }
               else if(event.keyboard.keycode == ALLEGRO_KEY_ENTER ||
                       event.keyboard.keycode == ALLEGRO_KEY_PAD_ENTER) {
                  al_ustr_assign(hiscore[difficulty][myscore].name, yourname);
                  done = true;
               }
               else if(event.keyboard.keycode == ALLEGRO_KEY_F12) {
                  if(setting.sound_on)
                     al_play_sample(sfx_camera, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  a5_screenshot("Screenshots/DeluxePacman2");
               }
               break;
         }

         if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;
            al_draw_bitmap(buffer, 0, 0, 0);
            draw_entry_box(myscore);
            al_flip_display();
         }
      }
      // Wait if a key is being held down
      do al_wait_for_event(event_queue, &event);
      while(event.type != ALLEGRO_EVENT_KEY_UP);

      // display score once it is entered, add 10 to player # to flag the function not to check
      // to see if the player topped the high score list, this is just to redisplay the scores.
      score_table(score, player + 10, level);
   }
   else {
      al_flush_event_queue(event_queue);

      // Wait until the specified time passes or a key/mouse is pressed.
      bool done = false;
      bool redraw = true;
      do {
         al_wait_for_event(event_queue, &event);

         if(event.type == ALLEGRO_EVENT_TIMER) {
            redraw = true;
         }
         // Wait for key to be pressed and released to continue
         if(event.type == ALLEGRO_EVENT_KEY_UP) {
            if(event.keyboard.keycode == ALLEGRO_KEY_F12) {
               if(setting.sound_on)
                  al_play_sample(sfx_camera, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               a5_screenshot("Screenshots/DeluxePacman2");
            }
            else if(event.keyboard.keycode == ALLEGRO_KEY_DELETE) {
               clear_hiscores(difficulty);
               redraw = true;
               done = true;
            }
            else done = true;
         }

         // Wait for mouse to be clicked and released to continue
         if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            close_button = true;
            done = true;
         }
         else if(event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            done = true;
         }
         /// JOYSTICK
         else if(event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP) {
            done = true;
         }
         else if(event.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION) {
            al_reconfigure_joysticks();
         }
         if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;
            // Rotate the three flares in the background while we wait for input
            angle1 += PI / 540;
            if(angle1 > PI * 2) angle1 -= PI * 2;
            angle2 -= PI / 720;
            if(angle2 < 0) angle2 += PI * 2;
            al_clear_to_color(al_map_rgba(0, 0, 0, 255));
            al_draw_tinted_scaled_rotated_bitmap(flare, al_map_rgb(cr, cg, cb), flare_x, flare_y,
                                                 WIDTH / 2, HEIGHT / 2, 2, 2, angle1, 0);
            al_draw_tinted_scaled_rotated_bitmap(flare, al_map_rgb(cr, cg, cb), flare_x, flare_y,
                                                 WIDTH / 2, HEIGHT / 2, 2, 2, angle2, 0);
            al_draw_bitmap(buffer, 0, 0, 0);
            al_flip_display();
            al_rest(.01);
         }
      }
      while(!done);
   }

   // We're done with these, make sure to free the memory up before we exit.
   al_destroy_bitmap(flare);  flare = NULL;
   al_destroy_bitmap(buffer); buffer = NULL;

   return close_button;
}


// Returns the best score, for other modules to display.
unsigned long get_hiscore(int dif, int num)
{
   return hiscore[dif][num].score;
}
