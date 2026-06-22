/**************************************************************************
 * Name         : Deluxe Pacman 2                                         *
 * Author       : Neil Roy                                                *
 * Date started : June 27, 2012  (actually started designing it MANY      *
 *                                years earlier, around 2009 or so)       *
 * Last update  : March 21, 2018 @2:40PM                                  *
 * Notes        : Allegro 5.2.4.1, MinGW 8.1.0, C11 (-std=gnu11)          *
 *                gnu11 is the C 2011 standard with gnu extensions.       *
 * Code syntax  : I use the K&R style unless otherwise stated.            *
 *                Indentations are 3 space (spaces not tabs).             *
 *                All filenames have the first letter of each word in     *
 *                the filename capitalized and use underscores to         *
 *                separate words.  Extensions are in lower case.          *
 *                Braces for commands are placed so the opening brace is  *
 *                on the same line as the command and the final brace is  *
 *                on a line of it's own.                                  *
 *                "else" and "else if" will start on their own line       *
 *                variables start in lower case, using underscores to     *
 *                separate words like this: int my_variable               *
 *                Functions are like variables, all lower case with       *
 *                underscores to separate words, like this: my_function() *
 *                Macros, structs and typedefs are in all uppercase and   *
 *                separated by underscores like this: #define MY_MACRO    *
 **************************************************************************/

#include "dp2_main.h"

const char *VERSION   = "v18.07.09 (1943)";
const char *COPYRIGHT = "Copyright \u00A92018 by Neil Roy";

/// Hack protection, only used if HACK_PROTECTION is defined in dp2_main.h
bool hack_detected = false;   // This will be set to true if a hack is detected and then used to screw up the game :)

/// Global Variables, some people hate them, but I won't jump through hoops just to avoid them.
SETTING setting;
PACMAN pacman;
PLAYER player[4];
GHOST ghost[4];
LEVEL *level = NULL;  // will point to player[cplayer].level
unsigned long hiscore = 0, ghostscore = 0;
FPOINT point[MAX_POINTS];
PICKUP pickup;
TOOL tool;
bool tool_inuse[MAX_TOOLS + 1]; // note: tool_inuse[0] is not used, tools are 1 - MAX_TOOLS
bool game_over = false;
bool exit_game = false;
bool done = false;
bool game_loaded = false;
bool highscore_initialized = false;
double fps = 0;
long frames = 0;
// Used for drawing energy bar. 136 pixels / MAX_ENERGY = what to multiply energy by to fit it inside the bar.
const float EMULT = 136 / (float)MAX_ENERGY;
BULLET bullet = {NULL, NULL, 0, 0, 0, 0, 0, 0.0f, 0, INACTIVE};
ALLEGRO_STATE state;
ALLEGRO_BITMAP *ghost_sheet = NULL;
ALLEGRO_BITMAP *blue_eyes_sheet = NULL;
ALLEGRO_BITMAP *ghost_eyes[4] = {NULL, NULL, NULL, NULL};
ALLEGRO_BITMAP *blue_ghost[4] = {NULL, NULL, NULL, NULL};
ALLEGRO_BITMAP *ice_cube = NULL;
ALLEGRO_BITMAP *heart[2] = {NULL, NULL};
ALLEGRO_BITMAP *ghost_spawn = NULL;
ALLEGRO_BITMAP *teleport = NULL;
ALLEGRO_BITMAP *pickup_spawn = NULL;
ALLEGRO_BITMAP *levelmap = NULL;
ALLEGRO_BITMAP *shield = NULL;
ALLEGRO_FONT *font_amaze = NULL;
ALLEGRO_FONT *font_verdana = NULL;
ALLEGRO_FONT *font_verdana_bmp = NULL;
ALLEGRO_FONT *font_radio_space_bold = NULL;
ALLEGRO_FONT *font_fatfont_grn = NULL;
ALLEGRO_FONT *font_fatfont_red = NULL;
ALLEGRO_FONT *font_digital = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_EVENT event;
ALLEGRO_BITMAP *icon = NULL;
ALLEGRO_BITMAP *background[20];
ALLEGRO_BITMAP *pills[5][8];
ALLEGRO_BITMAP *lines[100][16];
int max_linesets = 100; // Set this to max for now, we can adjust it to a smaller value later
ALLEGRO_SAMPLE_ID sample_id;
bool sample_id_on = false;
ALLEGRO_SAMPLE *sfx_pill = NULL;
ALLEGRO_SAMPLE *sfx_ppill = NULL;
ALLEGRO_SAMPLE *sfx_ghost1 = NULL;
ALLEGRO_SAMPLE *sfx_ghost2 = NULL;
ALLEGRO_SAMPLE *sfx_ghost3 = NULL;
ALLEGRO_SAMPLE *sfx_new_life = NULL;
ALLEGRO_SAMPLE *sfx_death = NULL;
ALLEGRO_SAMPLE *sfx_laugh = NULL;
ALLEGRO_SAMPLE *sfx_end_of_level1 = NULL;
ALLEGRO_SAMPLE *sfx_end_of_level2 = NULL;
ALLEGRO_SAMPLE *sfx_pickup = NULL;
ALLEGRO_SAMPLE *sfx_tool_pickup = NULL;
ALLEGRO_SAMPLE *sfx_eat_pickup = NULL;
ALLEGRO_SAMPLE *sfx_type = NULL;
ALLEGRO_SAMPLE *sfx_background1 = NULL;
ALLEGRO_SAMPLE *sfx_background2 = NULL;
ALLEGRO_SAMPLE *sfx_heaven = NULL;
ALLEGRO_SAMPLE *sfx_anthem = NULL;
ALLEGRO_SAMPLE *sfx_diamond = NULL;
ALLEGRO_SAMPLE *sfx_boom = NULL;
ALLEGRO_SAMPLE *sfx_freeze = NULL;
ALLEGRO_SAMPLE *sfx_precious = NULL;
ALLEGRO_SAMPLE *sfx_camera = NULL;
ALLEGRO_SAMPLE *sfx_teleport = NULL;
ALLEGRO_SAMPLE *music_main = NULL;
ALLEGRO_SAMPLE_ID music_id;


/* shutdown_display:
 *    Destroys objects to do with the display and frees up the memory they use.
 *    This function can be called when the display is reset via options so it
 *    needs to be separate from the normal shutdown.
 */
void shutdown_display(void)
{
   if(font_amaze) {
      al_destroy_font(font_amaze);
      font_amaze = NULL;
   }

   if(font_verdana) {
      al_destroy_font(font_verdana);
      font_verdana = NULL;
   }
   if(font_verdana_bmp) {
      al_destroy_font(font_verdana_bmp);
      font_verdana_bmp = NULL;
   }
   if(font_radio_space_bold) {
      al_destroy_font(font_radio_space_bold);
      font_radio_space_bold = NULL;
   }
   if(font_fatfont_grn) {
      al_destroy_font(font_fatfont_grn);
      font_fatfont_grn = NULL;
   }
   if(font_fatfont_red) {
      al_destroy_font(font_fatfont_red);
      font_fatfont_red = NULL;
   }
   if(font_digital) {
      al_destroy_font(font_digital);
      font_digital = NULL;
   }

   if(icon) {
      al_destroy_bitmap(icon);
      icon = NULL;
   }

   for(int i = 0; i < 2; i++) {
      if(heart[i]) {
         al_unlock_bitmap(heart[i]);
         al_destroy_bitmap(heart[i]);
         heart[i] = NULL;
      }
   }

   if(ghost_spawn) {
      al_unlock_bitmap(ghost_spawn);
      al_destroy_bitmap(ghost_spawn);
      ghost_spawn = NULL;
   }

   if(teleport) {
      al_unlock_bitmap(teleport);
      al_destroy_bitmap(teleport);
      teleport = NULL;
   }

   if(pickup_spawn) {
      al_unlock_bitmap(pickup_spawn);
      al_destroy_bitmap(pickup_spawn);
      pickup_spawn = NULL;
   }

   if(ice_cube) {
      al_destroy_bitmap(ice_cube);
      ice_cube = NULL;
   }

   if(levelmap) {
      al_destroy_bitmap(levelmap);
      levelmap = NULL;
   }

   if(pacman.sheet) {
      al_unlock_bitmap(pacman.sheet);
      for(int x = 0; x < 25; x++) {
         if(pacman.bitmap[x]) {
            al_destroy_bitmap(pacman.bitmap[x]);
            pacman.bitmap[x] = NULL;
         }
      }
      al_destroy_bitmap(pacman.sheet);
      pacman.sheet = NULL;
   }

   if(pickup.sheet) {
      al_unlock_bitmap(pickup.sheet);
      for(int x = 0; x < MAX_PICKUPS; x++) {
         if(pickup.bitmap[x]) {
            al_destroy_bitmap(pickup.bitmap[x]);
            pickup.bitmap[x] = NULL;
         }
      }
      al_destroy_bitmap(pickup.sheet);
      pickup.sheet = NULL;
   }

   if(tool.sheet) {
      al_unlock_bitmap(tool.sheet);
      for(int x = 0; x < MAX_TOOLS; x++) {
         if(tool.bitmap[x]) {
            al_destroy_bitmap(tool.bitmap[x]);
            tool.bitmap[x] = NULL;
         }
      }
      al_destroy_bitmap(tool.sheet);
      tool.sheet = NULL;
   }

   if(ghost_sheet) {
      al_unlock_bitmap(ghost_sheet);
      for(int y = 0; y < 4; y++) {     // 4 rows
         for(int x = 0; x < 5; x++) {  // 5 columns
            if(ghost[y].bitmap[x]) {
               al_destroy_bitmap(ghost[y].bitmap[x]);
               ghost[y].bitmap[x] = NULL;
            }
         }
      }
      al_destroy_bitmap(ghost_sheet);
      ghost_sheet = NULL;
   }

   if(blue_eyes_sheet) {
      al_unlock_bitmap(blue_eyes_sheet);
      for(int y = 0; y < 2; y++) {     // 2 rows
         for(int x = 0; x < 4; x++) {  // 4 columns
            if(y == 0 && ghost_eyes[x]) {
               al_destroy_bitmap(ghost_eyes[x]);
               ghost_eyes[x] = NULL;
            }
            else if(y == 1 && blue_ghost[x]) {
               al_destroy_bitmap(blue_ghost[x]);
               blue_ghost[x] = NULL;
            }
         }
      }
      al_destroy_bitmap(blue_eyes_sheet);
      blue_eyes_sheet = NULL;
   }

   for(int i = 0; i < 20; i++) {
      if(background[i]) {
         al_unlock_bitmap(background[i]);
         al_destroy_bitmap(background[i]);
         background[i] = NULL;
      }
   }

   for(int y = 0; y < 5; y++) {
      for(int x = 0; x < 8; x++) {
         if(pills[y][x]) {
            al_unlock_bitmap(pills[y][x]);
            al_destroy_bitmap(pills[y][x]);
            pills[y][x] = NULL;
         }
      }
   }

   for(int i = 0; i < max_linesets; i++) {
      for(int j = 0; j < 16; j++) {
         if(lines[i][j]) {
            al_unlock_bitmap(lines[i][j]);
            al_destroy_bitmap(lines[i][j]);
            lines[i][j] = NULL;
         }
      }
   }

   if(shield) {
      al_destroy_bitmap(shield);
      shield = NULL;
   }

   if(bullet.bitmap) {
      al_destroy_bitmap(bullet.bitmap);
      bullet.bitmap = NULL;
   }

   al_unregister_event_source(event_queue, al_get_display_event_source(setting.screen));

   if(setting.screen) {
      al_destroy_display(setting.screen);
      setting.screen = NULL;
   }
}


/* shut_down:
 *    destroys anything that needs destroying before shutdown
 *    in order to free up system memory.
 */
void shut_down(void)
{
#ifdef HACK_PROTECTION
   /// *** HACK CHECK ***
   if(hack_detected) {  // Lets leak some memory and skip writing the high scores, shall we?
#ifdef DEBUG
      printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
#ifndef DEBUG
      return;
#endif // return
   }
#endif // HACK_PROTECTION

   shutdown_display();

   if(setting.redraw_timer) {
      al_destroy_timer(setting.redraw_timer);
      setting.redraw_timer = NULL;
   }

   if(event_queue) {
      al_destroy_event_queue(event_queue);
      event_queue = NULL;
   }

   if(pacman.timer) {
      al_destroy_timer(pacman.timer);
      pacman.timer = NULL;
   }

   for(int i = 0; i < 4; i++) {
      if(ghost[i].timer) {
         al_destroy_timer(ghost[i].timer);
         ghost[i].timer = NULL;
      }
   }

   // Unload sound samples
   if(setting.music_on || setting.sound_on) al_stop_samples();

   al_destroy_sample(sfx_pill);           sfx_pill = NULL;
   al_destroy_sample(sfx_ppill);          sfx_ppill = NULL;
   al_destroy_sample(sfx_ghost1);         sfx_ghost1 = NULL;
   al_destroy_sample(sfx_ghost2);         sfx_ghost2 = NULL;
   al_destroy_sample(sfx_ghost3);         sfx_ghost3 = NULL;
   al_destroy_sample(sfx_new_life);       sfx_new_life = NULL;
   al_destroy_sample(sfx_death);          sfx_death = NULL;
   al_destroy_sample(sfx_laugh);          sfx_laugh = NULL;
   al_destroy_sample(sfx_end_of_level1);  sfx_end_of_level1 = NULL;
   al_destroy_sample(sfx_end_of_level2);  sfx_end_of_level2 = NULL;
   al_destroy_sample(sfx_pickup);         sfx_pickup = NULL;
   al_destroy_sample(sfx_tool_pickup);    sfx_tool_pickup = NULL;
   al_destroy_sample(sfx_eat_pickup);     sfx_eat_pickup = NULL;
   al_destroy_sample(sfx_type);           sfx_type = NULL;
   al_destroy_sample(sfx_background1);    sfx_background1 = NULL;
   al_destroy_sample(sfx_background2);    sfx_background2 = NULL;
   al_destroy_sample(sfx_anthem);         sfx_anthem = NULL;
   al_destroy_sample(sfx_diamond);        sfx_diamond = NULL;
   al_destroy_sample(sfx_boom);           sfx_boom = NULL;
   al_destroy_sample(sfx_freeze);         sfx_freeze = NULL;
   al_destroy_sample(sfx_precious);       sfx_precious = NULL;
   al_destroy_sample(sfx_camera);         sfx_camera = NULL;
   al_destroy_sample(sfx_teleport);       sfx_teleport = NULL;
   al_destroy_sample(bullet.sound);       bullet.sound = NULL;
   al_destroy_sample(music_main);         music_main = NULL;

   al_uninstall_audio();

   if(highscore_initialized) shutdown_hiscore();
}


// display the opening message at the start of each level
// the message contains the player and level number which
// are passed to this function.
void level_message(int levelnum, int cplayer, bool bonus_level)
{
   ALLEGRO_FONT *font = font_verdana_bmp;
   char buffer[TEXT_BUFFER];
   const int shadow = 13;

   // Maximum width of the window is determined by the longest sentence
   // in the message which is the "GET READY,,," message.
   snprintf(buffer, TEXT_BUFFER, "GET READY PLAYER %d", cplayer);

   int textWidth = al_get_text_width(font, buffer);
   int textHeight = al_get_font_ascent(font);

   int leftSide = WIDTH / 2 - textWidth / 2 - 12;
   int rightSide = WIDTH / 2 + textWidth / 2 + 12;
   int topSide = HEIGHT / 2 - textHeight - 7;
   int bottomSide = HEIGHT / 2 + textHeight * 2 + 15;

   // Draw the shadows first (if shadows are enabled!)
   if(setting.shadows) {
      al_draw_filled_rounded_rectangle(leftSide + shadow, topSide + shadow, rightSide + shadow,
                                       bottomSide + shadow, 15, 15, al_map_rgba_f(0, 0, 0, .4));
      al_draw_rounded_rectangle(leftSide + shadow, topSide + shadow, rightSide + shadow,
                                bottomSide + shadow, 15, 15, al_map_rgba_f(0, 0, 0, .2), 4);

      if(bonus_level) {
         al_draw_textf(font, al_map_rgba_f(0, 0, 0, .5),
                       WIDTH / 2 + 10, HEIGHT / 2 - textHeight + 10, ALLEGRO_ALIGN_CENTER,
                       "BONUS LEVEL!");
      }
      else {
         al_draw_textf(font, al_map_rgba_f(0, 0, 0, .5),
                       WIDTH / 2 + 10, HEIGHT / 2 - textHeight + 10, ALLEGRO_ALIGN_CENTER,
                       "LEVEL %d", levelnum);
      }
      al_draw_textf(font, al_map_rgba_f(0, 0, 0, .5),
                    WIDTH / 2 + 10, HEIGHT / 2 + 10, ALLEGRO_ALIGN_CENTRE,
                    "GET READY PLAYER %d", cplayer);
      al_draw_textf(font, al_map_rgba_f(0, 0, 0, .5),
                    WIDTH / 2 + 10, HEIGHT / 2 + textHeight + 10, ALLEGRO_ALIGN_CENTRE,
                    "PRESS ANY KEY");
   }

   // Shadows are drawn, so now we'll draw the actual box on top
   al_draw_filled_rounded_rectangle(leftSide, topSide, rightSide, bottomSide, 15, 15,
                                    al_map_rgba_f(0, 0, .1, .4));
   al_draw_rounded_rectangle(leftSide, topSide, rightSide, bottomSide, 15, 15, al_map_rgb_f(.25, .25, 1), 4);

   if(bonus_level) {
      al_draw_textf(font, al_map_rgb_f(1, 0, 0),
                    WIDTH / 2, HEIGHT / 2 - textHeight, ALLEGRO_ALIGN_CENTER,
                    "BONUS LEVEL!");
   }
   else {
      al_draw_textf(font, al_map_rgb_f(1, 0, 0),
                    WIDTH / 2, HEIGHT / 2 - textHeight, ALLEGRO_ALIGN_CENTER,
                    "LEVEL %d", levelnum);
   }
   al_draw_textf(font, al_map_rgb_f(1, 1, 1),
                 WIDTH / 2, HEIGHT / 2, ALLEGRO_ALIGN_CENTRE,
                 "GET READY PLAYER %d", cplayer);
   al_draw_textf(font, al_map_rgb_f(1, 1, 1),
                 WIDTH / 2, HEIGHT / 2 + textHeight, ALLEGRO_ALIGN_CENTRE,
                 "PRESS ANY KEY");
}


void init_pacman(void)
{
   pacman.cur_img = 1;
   pacman.cntr = 0;
   pacman.img_delay = IMAGE_DELAY;
   pacman.inc = 1;
   pacman.ts = TIMER_SPEED + (TIMER_SPEED * setting.suicide);
   pacman.fast = false;
   pacman.dx = 1;    /// TODO: Check area around pacman and set an alternate initial direction if path is blocked
   pacman.dy = 0;
   pacman.rot = 0.0f;
   pacman.flip = 0;
   //pacman.dead = false;
   pacman.size = SPRITE_SIZE;
   pacman.r = 21.0; // Radius of the pacman collision circle
}


void init_ghosts(void)
{
   for(int i = 0; i < 4; i++) {
      ghost[i].cur_img = 0;
      ghost[i].cntr = 0;
      ghost[i].img_delay = IMAGE_DELAY;
      ghost[i].inc = 1;
      ghost[i].ts = TIMER_SPEED + (TIMER_SPEED * setting.suicide);
      ghost[i].y = HEIGHT / 2 + i * SPRITE_SIZE;
      ghost[i].dx = 1;
      ghost[i].dy = 0;
      ghost[i].dead = false;
      ghost[i].scared = false;
      ghost[i].inc = 1;
      ghost[i].stimer = 0;
      ghost[i].stime = (int)(REDRAW_TIMER * 0.75f) >> setting.suicide;
      ghost[i].scntr = 0;
      ghost[i].eyes = 4; // 0 = down, 1 = left, 2 = right, 3 = up, 4 = straight
      ghost[i].frozen = false;
      ghost[i].size = SPRITE_SIZE;
      ghost[i].r = 19.0;  // Radius of the ghost collision circle.
   }

   ghostscore = 500;
}


void init_variables(void)
{
   for(int i = 0; i < MAX_POINTS; i++) {
      point[i].x = 0;
      point[i].y = 0;
      point[i].colour = 1.0f;
      point[i].delay = IMAGE_DELAY * 2;
      point[i].counter = 0;
      point[i].value = 0;
      point[i].move = 16;
      point[i].moved = 0;
   }

   // zero (false) tool_inuse[] array
   memset(tool_inuse, 0, sizeof(tool_inuse));

   for(int i = 0; i < 4; i++) player[i].dead = false;

   frames = 0;
}


// Initialize the current player (new game)
void init_player(void)
{
   for(int i = 0; i < 4; i++) {
      player[i].lives = 0;
      player[i].oldlives = 0;
      player[i].score = 0;          // Current score
      player[i].oldscore = 0;       // used to see how much the players score has changed
      player[i].clevel = 1;         // current level player is on
      player[i].oldlevel = 0;       // the last level the player came from
      player[i].pvalue = 1000;      // the current value of pickups
      player[i].oldenergy = 0;      // oldenergy
      player[i].energy = 0;         // energy used for speed
      player[i].extra = 0;          // What EXTRA letters does the player have, this uses bits to represent each letter.
      player[i].wrap = 0;           // has the player done all the levels? (needs to be an int to keep track of how many wraps
      player[i].god_mode = false;   // god_mode defaults to off
      player[i].extra_time = false; // this is set to true when the player gets the TIME tool
      player[i].bonus_level = 0;    // Set to level # when a player is on a bonus level.
   }
}


void init_pickups(LEVEL *level)
{
   pickup.current = 0;                       // current pickup number (0 to max)
   pickup.screen_time = REDRAW_TIMER * 8;    // max time pickup should stay on screen
   pickup.x = level->pickup.x * TILE_SIZE;   // on screen co-ordinates (map stores cell location)
   pickup.y = level->pickup.y * TILE_SIZE;   // on screen co-ordinates (map stores cell location)
   pickup.map = level->pickup;               // map co-ordinates (stored on level)
   pickup.active = INACTIVE;                 // is pickup currently on screen?
   pickup.cntr = 0;                          // delay counter
   pickup.wait = REDRAW_TIMER * 16;          // time between pickups (1/60th sec)
}


void init_tools(LEVEL *level)
{
   tool.current = 0;                      // current tool number (0 to max)
   tool.screen_time = REDRAW_TIMER * 8;   // max time tool should stay on screen
   tool.x = level->pickup.x * TILE_SIZE;  // on screen co-ordinates (map stores cell location)
   tool.y = level->pickup.y * TILE_SIZE;  // on screen co-ordinates (map stores cell location)
   tool.map = level->pickup;             // map co-ordinates (stored on level)
   tool.active = INACTIVE;                // is tool currently on screen?
   tool.cntr = 0;                         // delay counter
   tool.wait = REDRAW_TIMER * 16;         // time between tools (1/60th sec)
   tool.use_time = (TOOL_USE_TIME >> setting.suicide);         // time tool can be used by player
   tool.force = OFF;                      // If a cheat key is used, this will be set to the
   // tool # to be forced on (use enum word).

   bullet.state = INACTIVE;               // initial state is INACTIVE (0), can be ACTIVE (1).
}


/* init_display:
 *    initializes the display
 *    This function needs to be separate from the normal initialize function
 *    in order to be called if the display is reset via the options menu.
 */
void init_display(bool reset_display)
{
   char s[TEXT_BUFFER] = "";
   ALLEGRO_BITMAP *tempBitmap = NULL;

   if(!reset_display) { // These will already be NULL'd out if this is a reset
      // Initialize pointer arrays to zero before anything else so that if we
      // have to call shutdown() they can be checked without problems
      memset(background, 0, sizeof(background));
      memset(pills, 0, sizeof(pills));
      memset(lines, 0, sizeof(lines));

      setting.screen = NULL;
      pickup.sheet = NULL;
      tool.sheet = NULL;
      shield = NULL;
      bullet.bitmap = NULL;
      pacman.sheet = NULL;
   }

   // Vsync 1 means force on, 2 means forced off.
   if(setting.vsync)
      al_set_new_display_option(ALLEGRO_VSYNC, setting.vsync, ALLEGRO_SUGGEST);

   switch(setting.fullscreen) {
      case 0:  // Windowed
         if(setting.opengl) al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_WINDOWED);
         else al_set_new_display_flags(ALLEGRO_WINDOWED);
         break;
      case  1: // Fullscreen Window (0 = window)
         if(setting.opengl) al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW | ALLEGRO_OPENGL);
         else al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
         if(setting.frequency) al_set_new_display_refresh_rate(setting.frequency);
         break;
      case 2:  // Regular Fullscreen (0 = window)
         if(setting.opengl) al_set_new_display_flags(ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL);
         else al_set_new_display_flags(ALLEGRO_FULLSCREEN);
         if(setting.frequency) al_set_new_display_refresh_rate(setting.frequency);
         break;
      default: // invalid mode, we'll select windowed by default and reset the fullscreen variable
         if(setting.opengl) al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_WINDOWED);
         else al_set_new_display_flags(ALLEGRO_WINDOWED);
         setting.fullscreen = 0;
   }

   al_set_new_bitmap_flags(ALLEGRO_MAG_LINEAR); // | ALLEGRO_NO_PRESERVE_TEXTURE);
   //al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);

   al_set_new_display_option(ALLEGRO_SINGLE_BUFFER, 1, ALLEGRO_REQUIRE);

   // Allegro picks the desktop resolution automatically with ALLEGRO_FULLSCREEN_WINDOW flag set.
   setting.screen = al_create_display(WIDTH, HEIGHT);
   if(!setting.screen) {
      a5_error(AT, setting.screen, "Create display failed.");
      shut_down();
      exit(1);
   }

   setting.w = al_get_display_width(setting.screen);
   setting.h = al_get_display_height(setting.screen);

   if(setting.opengl) printf("OpenGL version: 0x%08X\n", al_get_opengl_version());

   // Call scale screen for all modes to correct mouse movement in certain mode changes
   a5_scale_screen(WIDTH, HEIGHT, setting.w, setting.h);

   al_clear_to_color(al_map_rgb_f(0, 0, 0));
   al_flip_display();

   icon = al_load_bitmap("Graphics/Deluxe_Pacman_2_Icon.png");
   if(!icon) {
      a5_error(AT, setting.screen, "Error loading Deluxe_Pacman_2_Icon.png.");
      shut_down();
      exit(1);
   }
   al_set_display_icon(setting.screen, icon);

   al_set_window_title(setting.screen, "Deluxe Pacman 2");

   /// ****** Load Fonts *******
   font_fatfont_grn = al_load_bitmap_font("Fonts/FatFont_GRN.png");
   if(!font_fatfont_grn) {
      a5_error(AT, setting.screen, "Error loading FatFont_GRN.png");
      shut_down();
      exit(1);
   }

   if(!reset_display) {   // Don't display loading screen on a reset
      al_clear_to_color(al_map_rgb_f(0, 0, 0));
      al_draw_textf(font_fatfont_grn, al_map_rgb_f(1, 1, 1), WIDTH / 2, HEIGHT / 2, ALLEGRO_ALIGN_CENTRE, "Loading...");
      al_flip_display();
   }

   font_fatfont_red = al_load_bitmap_font("Fonts/FatFont_RED.png");
   if(!font_fatfont_red) {
      a5_error(AT, setting.screen, "Error loading FatFont_RED.png");
      shut_down();
      exit(1);
   }

   font_verdana_bmp = al_load_bitmap_font("Fonts/Verdana_Bold_12_A4.png");
   if(!font_verdana_bmp) {
      a5_error(AT, setting.screen, "Error loading Verdana_Bold_12_A4.png");
      shut_down();
      exit(1);
   }

   font_verdana = al_load_ttf_font("Fonts/verdanab.ttf", 18, 0);
   if(!font_verdana) {
      a5_error(AT - 2, setting.screen, "Error loading Verdana_Bold.ttf.");
      shut_down();
      exit(1);
   }

   font_amaze = al_load_ttf_font("Fonts/Amaze_Bold.ttf", 60, 0);
   if(!font_amaze) {
      a5_error(AT, setting.screen, "Error loading Amaze_Bold.ttf.");
      shut_down();
      exit(1);
   }

   font_radio_space_bold = al_load_ttf_font("Fonts/Radio_Space_Bold.ttf", 12, 0);
   if(!font_radio_space_bold) {
      a5_error(AT, setting.screen, "Error loading Radio_Space_Bold.ttf.");
      shut_down();
      exit(1);
   }

   // The Digital fonts will be a space + numbers 0 to 9 ONLY for use with the score.
   // The Digital font itself is White and grey, you need to set the colour you want it to be.
   tempBitmap = al_load_bitmap("Fonts/Digital.png");
   int ranges2[4] = {
      0x0020, 0x0020,  // Space (totally dark number)
      0x0030, 0x0039   // 0 to 9
   };
   font_digital = al_grab_font_from_bitmap(tempBitmap, 2, ranges2);

   al_destroy_bitmap(tempBitmap);
   tempBitmap = NULL;

   /// ****** Load background tiles *******
   tempBitmap = al_load_bitmap("Graphics/Backgrounds.png");
   if(!tempBitmap) {
      printf("%s(%d): Error loading backgrounds.\n", __FILE__, __LINE__);
      shut_down();
      exit(1);
   }

   for(int i = 0; i < 20; i++) {
      background[i] = al_create_bitmap(TILE_SIZE, TILE_SIZE);
      if(!background[i]) {
         printf("%s(%d): Error creating background[%d] bitmap.\n", __FILE__, __LINE__, i);
         shut_down();
         exit(1);
      }
      al_set_target_bitmap(background[i]);
      al_draw_bitmap_region(tempBitmap, i * 32, 0, 32, 32, 0, 0, 0);
      al_lock_bitmap(background[i], ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
      //al_unlock_bitmap(ALLEGRO_BITMAP *bitmap)
   }
   al_destroy_bitmap(tempBitmap);
   tempBitmap = NULL;

   /// ****** Load pills *******
   tempBitmap = al_load_bitmap("Graphics/Pill_Sheet.png");
   if(!tempBitmap) {
      printf("%s(%d): Error loading Pill_Sheet.png.\n", __FILE__, __LINE__);
      shut_down();
      exit(1);
   }

   for(int y = 0; y < 5; y++) {
      for(int x = 0; x < 8; x++) {
         pills[y][x] = al_create_bitmap(32, 32);
         if(!pills[y][x]) {
            printf("%s(%d): Error creating pills[%d][%d] bitmap.\n", __FILE__, __LINE__, y, x);
            shut_down();
            exit(1);
         }
         al_set_target_bitmap(pills[y][x]);
         al_draw_bitmap_region(tempBitmap, x * 32, y * 32, 32, 32, 0, 0, 0);
         al_lock_bitmap(pills[y][x], ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
      }
   }
   al_destroy_bitmap(tempBitmap);
   tempBitmap = NULL;

   /// ******* Load lines *******
   printf("Loading line graphics.\n");

   // Here we'll check to see how many line files there are
   int linecount = 0;   // We'll use this to count how many we find
   bool file_found;     // This will be true if we find one, false if we do not
   do {
      snprintf(s, TEXT_BUFFER, "Graphics/Lines/Line%02d.png", linecount);
      file_found = al_filename_exists(s);
      if(file_found) linecount++;   // Increment the number we found so far
   }
   while(file_found);   // As long as we are finding files, keep looking

   if(!linecount) {  // If we found no line files at all, there must be a problem, report it and exit.
      printf("%s(%d): No lines found in pakfile.\n", __FILE__, __LINE__);
      shut_down();
      exit(1);
   }

   printf("Lines found in pak: %d\n", linecount);  // Print to the console how many found

   // We now know exactly how many exist in the pak file, we can load them all in
   for(int i = 0; i < linecount; i++) {
      snprintf(s, TEXT_BUFFER, "Graphics/Lines/Line%02d.png", i);       // Set the filename to check for
      tempBitmap = al_load_bitmap(s);                                   // Attempt to load it
      if(!tempBitmap) {                                                 // If it does load, exit game.
         printf("%s(%d): Error loading \"%s\".\n", __FILE__, __LINE__, s);
         shut_down();   // Call the shut_down function first, to free up memory
         exit(1);       // Exit game.
      }
      for(int j = 0; j < 16; j++) {                                     // There are 16 tiles per line strip
         lines[i][j] = al_create_bitmap(TILE_SIZE, TILE_SIZE);          // Create a bitmap for each line tile
         al_set_target_bitmap(lines[i][j]);                             // set the target to draw it to the lines array
         al_draw_bitmap_region(tempBitmap, j * 32, 0, 32, 32, 0, 0, 0); // draw it
         al_lock_bitmap(lines[i][j], ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
      }
      al_destroy_bitmap(tempBitmap); tempBitmap = NULL;                 // We no longer need the temporary bitmap
   }

   int max_linesets = linecount; // Set the maximum line sets we found so far.

   // Check for custom line sets from 0 to 99 (100 total)
   al_set_standard_file_interface();
   int custom = 0;
   for(int i = 0; i < 100; i++) {
      snprintf(s, TEXT_BUFFER, "Custom\\Line%02d.png", i);  // We'll look in ths Custom folder
      tempBitmap = al_load_bitmap(s);
      if(!tempBitmap) { // Nothing was loaded, so we reached the last custom lineset (if any) found.
         if(i >= linecount) { // Only stop if i is greater in number than what was found in the pak file
            printf("Custom line sets found: %d\n", custom);
            break;   // Break out of the for loop
         }
      }
      else {   // We found something...
         // First destroy any existing lines this may be replacing...
         for(int j = 0; j < 16; j++) {
            if(lines[i][j]) {
               al_unlock_bitmap(lines[i][j]);
               al_destroy_bitmap(lines[i][j]);
               lines[i][j] = NULL;
            }
         }
         for(int j = 0; j < 16; j++) {
            lines[i][j] = al_create_bitmap(TILE_SIZE, TILE_SIZE);
            al_set_target_bitmap(lines[i][j]);
            al_draw_bitmap_region(tempBitmap, j * 32, 0, 32, 32, 0, 0, 0);
            al_lock_bitmap(lines[i][j], ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
         }
         al_destroy_bitmap(tempBitmap); tempBitmap = NULL;
         if(i >= linecount) max_linesets++;
         custom++;
      }
   }
   al_set_physfs_file_interface();

   printf("Maximum lineset# = %d (some custom sets may replace existing sets)\n", max_linesets - 1);

   /// ******* Load Pickups *******
   pickup.sheet = al_load_bitmap("Graphics/Pickups.png");
   if(!pickup.sheet) {
      a5_error(AT, setting.screen, "Failed to load Pickups.png.");
      shut_down();
      exit(1);
   }
   al_lock_bitmap(pickup.sheet, al_get_bitmap_format(pickup.sheet), ALLEGRO_LOCK_READONLY);
   for(int y = 0; y < 7; y++) { // bitmaps are 7 rows
      for(int x = 0; x < 5; x++) { // by 5 columns
         pickup.bitmap[5 * y + x] = NULL;
         pickup.bitmap[5 * y + x] = al_create_sub_bitmap(pickup.sheet, x * SPRITE_SIZE, y * SPRITE_SIZE,
                                    SPRITE_SIZE, SPRITE_SIZE);
         if(!pickup.bitmap[5 * y + x]) {
            a5_error(AT, setting.screen, "Error creating pickup sub_bitmap.");
            shut_down();
            exit(1);
         }
      }
   }

   /// ******* Load Tools *******
   tool.sheet = al_load_bitmap("Graphics/Tools.png");
   if(!tool.sheet) {
      a5_error(AT, setting.screen, "Failed to load Tools.png.");
      shut_down();
      exit(1);
   }
   al_lock_bitmap(tool.sheet, al_get_bitmap_format(tool.sheet), ALLEGRO_LOCK_READONLY);
   for(int y = 0; y < 5; y++) { // bitmaps are 5 rows
      for(int x = 0; x < 5; x++) { // by 5 columns
         tool.bitmap[5 * y + x] = NULL;
         tool.bitmap[5 * y + x] = al_create_sub_bitmap(tool.sheet, x * SPRITE_SIZE, y * SPRITE_SIZE,
                                  SPRITE_SIZE, SPRITE_SIZE);
         if(!tool.bitmap[5 * y + x]) {
            a5_error(AT, setting.screen, "Error creating tool sub_bitmap.");
            shut_down();
            exit(1);
         }
      }
   }

   shield = al_load_bitmap("Graphics/Shield.png");
   if(!shield) {
      a5_error(AT, setting.screen, "Failed to load Shield.png.");
      shut_down();
      exit(1);
   }

   bullet.bitmap = al_load_bitmap("Graphics/Bullet.png");
   if(!bullet.bitmap) {
      a5_error(AT, setting.screen, "Failed to load Bullet.png.");
      shut_down();
      exit(1);
   }

   /// ******* Load hearts *******
   heart[0] = al_load_bitmap("Graphics/Heart_Blue.png");
   if(!heart[0]) {
      a5_error(AT, setting.screen, "Error loading Heart_Blue.png.");
      shut_down();
      exit(1);
   }
   al_lock_bitmap(heart[0], ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
   heart[1] = al_load_bitmap("Graphics/Heart_Red.png");
   if(!heart[1]) {
      a5_error(AT, setting.screen, "Error loading Heart_Red.png.");
      shut_down();
      exit(1);
   }
   al_lock_bitmap(heart[1], ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

   /// ******* Load Ghost Spawn *******
   ghost_spawn = al_load_bitmap("Graphics/Ghost_Spawn.png");
   if(!ghost_spawn) {
      a5_error(AT, setting.screen, "Error loading Ghost_Spawn.png.");
      shut_down();
      exit(1);
   }
   al_lock_bitmap(ghost_spawn, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

   /// ******* Load Teleport *******
   teleport = al_load_bitmap("Graphics/Teleport.png");
   if(!teleport) {
      a5_error(AT, setting.screen, "Error loading Teleport.png.");
      shut_down();
      exit(1);
   }
   al_lock_bitmap(teleport, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

   /// ******* Load Pickup Spawn *******
   pickup_spawn = al_load_bitmap("Graphics/Pickup_Spawn.png");
   if(!pickup_spawn) {
      a5_error(AT, setting.screen, "Error loading Pickup_Spawn.png.");
      shut_down();
      exit(1);
   }
   al_lock_bitmap(pickup_spawn, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

   /// ******* Create bitmap for drawing the level on *******
   levelmap = al_create_bitmap(MAPX * 32, MAPY * 32);
   if(!levelmap) {
      a5_error(AT, setting.screen, "Error creating levelmap.");
      shut_down();
      exit(1);
   }

   /// ******* Initialize Pacman *******
   // Load Pacman sprite sheet first
   pacman.sheet = al_load_bitmap("Graphics/Pacman.png");
   if(!pacman.sheet) {
      a5_error(AT, setting.screen, "Failed to load Pacman bitmaps.");
      shut_down();
      exit(1);
   }
   al_lock_bitmap(pacman.sheet, al_get_bitmap_format(pacman.sheet), ALLEGRO_LOCK_READONLY);
   for(int y = 0; y < 5; y++) { // bitmaps are 5 rows
      for(int x = 0; x < 5; x++) { // by 5 columns
         pacman.bitmap[5 * y + x] = NULL;
         pacman.bitmap[5 * y + x] = al_create_sub_bitmap(pacman.sheet, x * SPRITE_SIZE, y * SPRITE_SIZE,
                                    SPRITE_SIZE, SPRITE_SIZE);
         if(!pacman.bitmap[5 * y + x]) {
            a5_error(AT, setting.screen, "Error creating pacman sub_bitmap.");
            shut_down();
            exit(1);
         }
      }
   }

   /// ****** Initialize Ghosts *******
   // Load Ghosts bitmaps
   ghost_sheet = al_load_bitmap("Graphics/Ghosts.png");
   if(!ghost_sheet) {
      a5_error(AT, setting.screen, "Failed to load Ghosts.png");
      shut_down();
      exit(1);
   }
   al_lock_bitmap(ghost_sheet, al_get_bitmap_format(ghost_sheet), ALLEGRO_LOCK_READONLY);
   for(int y = 0; y < 4; y++) {    // 4 rows
      for(int x = 0; x < 5; x++) { // 5 columns
         ghost[y].bitmap[x] = al_create_sub_bitmap(ghost_sheet, x * SPRITE_SIZE, y * SPRITE_SIZE,
                              SPRITE_SIZE, SPRITE_SIZE);
      }
   }

   // Load Blue Ghost and Eyes bitmaps
   blue_eyes_sheet = al_load_bitmap("Graphics/Blue_Eyes.png");
   if(!blue_eyes_sheet) {
      a5_error(AT - 2, setting.screen, "Failed to load Blue_Eyes.png");
      shut_down();
      exit(1);
   }
   al_lock_bitmap(blue_eyes_sheet, al_get_bitmap_format(blue_eyes_sheet), ALLEGRO_LOCK_READONLY);
   for(int y = 0; y < 2; y++) {     // 2 rows
      for(int x = 0; x < 4; x++) {  // 4 columns
         if(y == 0) // Eye bitmaps
            ghost_eyes[x] = al_create_sub_bitmap(blue_eyes_sheet, x * SPRITE_SIZE, y * SPRITE_SIZE,
                                                 SPRITE_SIZE, SPRITE_SIZE);
         else if(y == 1) // Blue ghosts
            blue_ghost[x] = al_create_sub_bitmap(blue_eyes_sheet, x * SPRITE_SIZE, y * SPRITE_SIZE,
                                                 SPRITE_SIZE, SPRITE_SIZE);
      }
   }

   // Load ice_cube used when ghosts are frozen.
   ice_cube = al_load_bitmap("Graphics/Ice_Cube.png");
   if(!ice_cube) {
      a5_error(AT - 2, setting.screen, "Failed to load Ice_Cube.png");
      shut_down();
      exit(1);
   }

   // reset the target bitmap back to the display
   al_set_target_bitmap(al_get_backbuffer(setting.screen));

   // register the new display in the event_queue
   al_register_event_source(event_queue, al_get_display_event_source(setting.screen));
}


/* Initialize:
 *    Initializes allegro + add-ons,
 */
void initialize(void)
{
   char s[TEXT_BUFFER] = "";

   if(!al_init()) {
      a5_error(AT, setting.screen, "al_init() failed.");
      shut_down();
      exit(1);
   }

   uint32_t version = al_get_allegro_version();
   unsigned char major = version >> 24;
   unsigned char minor = (version >> 16) & 255;
   unsigned char revision = (version >> 8) & 255;

   // print C version in console
   printf("C version: %lu\n", __STDC_VERSION__);
   printf("Allegro version: %i.%i.%i\n", major, minor, revision);

   al_init_image_addon();
   printf("Image addon installed\n");
   al_init_primitives_addon();
   printf("Primitives addon installed\n");
   al_init_font_addon();
   printf("font addon installed\n");
   al_init_ttf_addon();
   printf("TTF addon installed\n");
   al_install_keyboard();
   printf("Keyboard installed\n");
   al_install_mouse();
   printf("Mouse installed\n");
   /// JOYSTICK
   al_install_joystick();

   srand(time(NULL)); // seed the random number generator

   // Make certain the Screenshots folder exists
   if(!al_filename_exists("Screenshots")) {
      printf("\"Screenshots\" not detected, creating... ");
      al_make_directory("Screenshots");
      printf("done.\n");
   }

   // Make certain the Custom folder exists
   if(!al_filename_exists("Custom")) {
      printf("\"Custom\" not detected, creating... ");
      al_make_directory("Custom");
      printf("done.\n");
   }

   load_config(&setting);

   /// *** Security Check ***
   ALLEGRO_FILE *file = NULL;
   file = al_fopen("Deluxe Pacman 2.pak", "rb");
   int64_t size_check = al_fsize(file);
   al_fclose(file);

   /// ****** Initialize PHYSFS *******
   PHYSFS_init(NULL);
   if(!PHYSFS_mount("Deluxe Pacman 2.pak", "/", 1)) {
      a5_error(AT, setting.screen, "PHSYFS_init() failed.");
      exit(1);
   }
   al_set_physfs_file_interface();

   /// ****** Create event queue *******
   // it's important to create this before timers & display
   event_queue = al_create_event_queue();
   if(event_queue == NULL) {
      a5_error(AT, setting.screen, "Failed to create event queue.");
      shut_down();
      exit(1);
   }
   al_register_event_source(event_queue, al_get_keyboard_event_source());
   al_register_event_source(event_queue, al_get_mouse_event_source());
   /// JOYSTICK
   al_register_event_source(event_queue, al_get_joystick_event_source());

   /// ****** Set Up Display *******

   init_display(false); // false = this isn't a reset

#ifdef DEBUG
   if(size_check != PAKSIZE) printf("Pakfile size has changed, new size is %I64d.\n", size_check);
#endif // DEBUG

   /// ****** Create redraw_timer *******
   // Create a timer for updating the screen.
   // This is also used for animation timing.
   setting.redraw_timer = al_create_timer(1.0 / REDRAW_TIMER);
   if(setting.redraw_timer == NULL) {
      a5_error(AT, setting.screen, "Failed to create redraw timer.");
      shut_down();
      exit(1);
   }
   al_register_event_source(event_queue, al_get_timer_event_source(setting.redraw_timer));
   al_start_timer(setting.redraw_timer);

   /// ****** Initialize Pacman *******
   // Set pacman variables to defaults after
   // bitmap loads so pacman.size can be set
   init_pacman();

   // Create a timer for updating pacman movement
   // once InitPacman is done
   pacman.timer = al_create_timer(1.0f / pacman.ts);
   if(!pacman.timer) {
      a5_error(AT, setting.screen, "Failed to create pacman timer.");
      shut_down();
      exit(1);
   }
   al_register_event_source(event_queue, al_get_timer_event_source(pacman.timer));
   al_start_timer(pacman.timer);

   /// ****** Initialize Ghosts *******
   // Set ghost variables to defaults after
   // bitmap loads so ghost.size can be set
   init_ghosts();

   // Create a timer for updating ghosts movement
   ghost[0].timer = al_create_timer(1.0f / ghost[0].ts);
   if(!ghost[0].timer) {
      a5_error(AT, setting.screen, "Failed to create ghost timer.");
      shut_down();
      exit(1);
   }
   al_register_event_source(event_queue, al_get_timer_event_source(ghost[0].timer));
   al_start_timer(ghost[0].timer);

   /// ******* Initialize Audio *******
   if(!al_install_audio()) {
      a5_error(AT, setting.screen, "al_install_audio() failed.");
      shut_down();
      exit(1);
   }

   if(!al_init_acodec_addon()) {
      a5_error(AT, setting.screen, "al_init_acodec_addon() failed.");
      shut_down();
      exit(1);
   }

   if(!al_reserve_samples(RESERVED_SAMPLES)) {
      a5_error(AT, setting.screen, "al_reserve_samples() failed.");
      shut_down();
      exit(1);
   }
   printf("Audio initialized\n");
   /// ******* Load Sound Samples *******
//al_set_standard_file_interface();
   sfx_pill = al_load_sample("Sound/Pill.ogg");
   if(!sfx_pill) {
      a5_error(AT, setting.screen, "Failed to load Pill.ogg");
      shut_down();
      exit(1);
   }
//al_set_physfs_file_interface();

   sfx_ppill = al_load_sample("Sound/Power_Pill.ogg");
   if(!sfx_ppill) {
      a5_error(AT, setting.screen, "Failed to load Power_Pill.ogg");
      shut_down();
      exit(1);
   }

   sfx_ghost1 = al_load_sample("Sound/Eat_Ghost1.ogg");
   if(!sfx_ghost1) {
      a5_error(AT, setting.screen, "Failed to load Eat_Ghost1.ogg");
      shut_down();
      exit(1);
   }

   sfx_ghost2 = al_load_sample("Sound/Eat_Ghost2.ogg");
   if(!sfx_ghost2) {
      a5_error(AT, setting.screen, "Failed to load Eat_Ghost2.ogg");
      shut_down();
      exit(1);
   }

   sfx_ghost3 = al_load_sample("Sound/Eat_Ghost3.ogg");
   if(!sfx_ghost3) {
      a5_error(AT, setting.screen, "Failed to load Eat_Ghost3.ogg");
      shut_down();
      exit(1);
   }

   sfx_new_life = al_load_sample("Sound/New_Life.ogg");
   if(!sfx_new_life) {
      a5_error(AT, setting.screen, "Failed to load New_Life.ogg");
      shut_down();
      exit(1);
   }

   sfx_death = al_load_sample("Sound/Player_Death.ogg");
   if(!sfx_death) {
      a5_error(AT, setting.screen, "Failed to load Player_Death.ogg");
      shut_down();
      exit(1);
   }

   sfx_laugh = al_load_sample("Sound/Laugh.ogg");
   if(!sfx_laugh) {
      a5_error(AT, setting.screen, "Failed to load Laugh.ogg");
      shut_down();
      exit(1);
   }

   sfx_end_of_level1 = al_load_sample("Sound/End_Level_1.ogg");
   if(!sfx_end_of_level1) {
      a5_error(AT, setting.screen, "Failed to load End_Level_1.ogg");
      shut_down();
      exit(1);
   }

   sfx_end_of_level2 = al_load_sample("Sound/End_Level_2.ogg");
   if(!sfx_end_of_level2) {
      a5_error(AT, setting.screen, "Failed to load End_Level_2.ogg");
      shut_down();
      exit(1);
   }

   sfx_pickup = al_load_sample("Sound/Pickup.ogg");
   if(!sfx_pickup) {
      a5_error(AT, setting.screen, "Failed to load Pickup.ogg");
      shut_down();
      exit(1);
   }

   sfx_tool_pickup = al_load_sample("Sound/Tool.ogg");
   if(!sfx_tool_pickup) {
      a5_error(AT, setting.screen, "Failed to load Tool.ogg");
      shut_down();
      exit(1);
   }

   sfx_eat_pickup = al_load_sample("Sound/Eat_Pickup.ogg");
   if(!sfx_eat_pickup) {
      a5_error(AT, setting.screen, "Failed to load Eat_Pickup.ogg");
      shut_down();
      exit(1);
   }

   sfx_type = al_load_sample("Sound/Type.ogg");
   if(!sfx_type) {
      a5_error(AT, setting.screen, "Failed to load Type.ogg");
      shut_down();
      exit(1);
   }

   sfx_background1 = al_load_sample("Sound/Background1.ogg");
   if(!sfx_background1) {
      a5_error(AT, setting.screen, "Failed to load Background1.ogg");
      shut_down();
      exit(1);
   }

   sfx_background2 = al_load_sample("Sound/Background2.ogg");
   if(!sfx_background2) {
      a5_error(AT, setting.screen, "Failed to load sfx_background2");
      shut_down();
      exit(1);
   }

   sfx_heaven = al_load_sample("Sound/Heaven.ogg");
   if(!sfx_heaven) {
      a5_error(AT, setting.screen, "Failed to load sfx_background2");
      shut_down();
      exit(1);
   }

   sfx_anthem = al_load_sample("Sound/Anthem.ogg");
   if(!sfx_anthem) {
      a5_error(AT, setting.screen, "Failed to load sfx_anthem");
      shut_down();
      exit(1);
   }

   sfx_diamond = al_load_sample("Sound/Diamond.ogg");
   if(!sfx_diamond) {
      a5_error(AT, setting.screen, "Failed to load sfx_diamond");
      shut_down();
      exit(1);
   }

   sfx_boom = al_load_sample("Sound/Boom.ogg");
   if(!sfx_boom) {
      a5_error(AT, setting.screen, "Failed to load sfx_boom");
      shut_down();
      exit(1);
   }

   sfx_freeze = al_load_sample("Sound/Freeze.ogg");
   if(!sfx_freeze) {
      a5_error(AT, setting.screen, "Failed to load sfx_freeze");
      shut_down();
      exit(1);
   }

   sfx_precious = al_load_sample("Sound/Precious.ogg");
   if(!sfx_precious) {
      a5_error(AT, setting.screen, "Failed to load sfx_precious");
      shut_down();
      exit(1);
   }

   sfx_camera = al_load_sample("Sound/Camera.ogg");
   if(!sfx_camera) {
      a5_error(AT, setting.screen, "Failed to load sfx_camera");
      shut_down();
      exit(1);
   }

   sfx_teleport = al_load_sample("Sound/Teleport.ogg");
   if(!sfx_teleport) {
      a5_error(AT, setting.screen, "Failed to load sfx_teleport");
      shut_down();
      exit(1);
   }

   music_main = al_load_sample("Sound/Title.ogg");
   if(!music_main) {
      a5_error(AT, setting.screen, "Failed to load Title.ogg");
      shut_down();
      exit(1);
   }

   bullet.sound = al_load_sample("Sound/Bullet.ogg");
   if(!bullet.sound) {
      a5_error(AT, setting.screen, "Failed to load bullet.sound");
      shut_down();
      exit(1);
   }

#ifdef HACK_PROTECTION
   // Note: Set "pakman=1" in settings ini file to disable .pak size check, so you can alter it.
   if(size_check != PAKSIZE && !setting.pakman) hack_detected = true;
#endif // HACK_PROTECTION
}


// Returns whether or not a specified key is being pressed
bool key_pressed(int key)
{
   ALLEGRO_KEYBOARD_STATE s;
   al_get_keyboard_state(&s);
   return al_key_down(&s, key);
}


// Waits a specified time for a key to be pressed.
// Will also respond if a mouse or joystick button is clicked.
// If the time passes before a key/mouse/joystick is pressed, the function returns
// If the time specified is zero, the function will wait indefinitely.
bool wait_for_keypress(double wait_time)
{
   double ts = al_get_time(); // time stamp
   double elapsed_time;

   al_flush_event_queue(event_queue);
   frames = 0;

   bool done = false;

   // Wait until the specified time passes or a key is pressed.
   do {
      al_wait_for_event(event_queue, &event);

      if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
         shut_down();
         exit(0);
      }

      else if(wait_time && event.type == ALLEGRO_EVENT_TIMER) {
         elapsed_time = al_get_time() - ts;
         if(elapsed_time >= wait_time) return false;
      }

      // Wait for key to be pressed and released to continue
      else if(event.type == ALLEGRO_EVENT_KEY_UP) {
         if(event.keyboard.keycode == ALLEGRO_KEY_F12) {
            if(setting.sound_on)
               al_play_sample(sfx_camera, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            a5_screenshot("Screenshots/DeluxePacman2");
         }
         else if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) return true;
         else done = true;
      }

      // And if a key wasn't pressed, maybe a mouse button was...
      else if(event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
         done = true;
      }

      /// JOYSTICK
      else if(event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP) {
         if(event.joystick.button == 0) done = true;  // Button 0 = A (green, like pressing ENTER)
         if(event.joystick.button == 1) return true;  // Button 1 = B (red, used like pressing ESC)
         // Button 7 = Start (unused here)
      }

      else if(event.type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
         done = true;
      }

      else if(event.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION) {
         al_reconfigure_joysticks();
      }
   }
   while(!done);

   return false;
}


void message(const char *text_message)
{
   ALLEGRO_FONT *font = font_verdana_bmp;
   const int shadow = 13;

   int textWidth = al_get_text_width(font, text_message);
   int textHeight = al_get_font_ascent(font);

   int leftSide = WIDTH / 2 - textWidth / 2 - 12;
   int rightSide = WIDTH / 2 + textWidth / 2 + 12;
   int topSide = HEIGHT / 2 - textHeight - 7;
   int bottomSide = HEIGHT / 2 + textHeight + 7;

   al_set_target_bitmap(al_get_backbuffer(setting.screen));
   //al_draw_bitmap(menuback, 0, 0, 0);

   // Draw the shadows first
   al_draw_filled_rounded_rectangle(leftSide + shadow, topSide + shadow, rightSide + shadow,
                                    bottomSide + shadow, 15, 15, al_map_rgba_f(0, 0, 0, .4));
   al_draw_rounded_rectangle(leftSide + shadow, topSide + shadow, rightSide + shadow,
                             bottomSide + shadow, 15, 15, al_map_rgba_f(0, 0, 0, .2), 4);

   al_draw_textf(font, al_map_rgba_f(0, 0, 0, .5),
                 WIDTH / 2 + 10, HEIGHT / 2 - textHeight + shadow, ALLEGRO_ALIGN_CENTRE,
                 text_message);
   al_draw_textf(font, al_map_rgba_f(0, 0, 0, .5),
                 WIDTH / 2 + 10, HEIGHT / 2 + shadow, ALLEGRO_ALIGN_CENTRE,
                 "(click / press key)");


   // Shadows are drawn, so now we'll draw the actual box on top
   al_draw_filled_rounded_rectangle(leftSide, topSide, rightSide, bottomSide, 15, 15,
                                    al_map_rgba_f(0, 0, .1, .4));
   al_draw_rounded_rectangle(leftSide, topSide, rightSide, bottomSide, 15, 15, al_map_rgb_f(.25, .25, 1), 4);

   al_draw_textf(font, al_map_rgb_f(0, 1, 0),
                 WIDTH / 2, HEIGHT / 2 - textHeight, ALLEGRO_ALIGN_CENTRE,
                 text_message);
   al_draw_textf(font, al_map_rgb_f(.5, 5, 5),
                 WIDTH / 2, HEIGHT / 2, ALLEGRO_ALIGN_CENTRE,
                 "(click / press key)");

   al_flip_display();

   wait_for_keypress(0);
}


// Displays an Intro screen,
// containing "Neil Roy presents"
// Used in all my games before the main game
// title screen
void intro(void)
{
   ALLEGRO_SAMPLE *swamp = NULL;
   ALLEGRO_BITMAP *introback = NULL;
   ALLEGRO_BITMAP *titleback = NULL;

   swamp = al_load_sample("Sound/Crickets.ogg");
   if(!swamp) {
      a5_error(AT, setting.screen, "Failed to load Crickets.ogg");
      shut_down();
      exit(1);
   }

   introback = al_load_bitmap("Graphics/Intro.png");
   if(!introback) {
      a5_error(AT, setting.screen, "Failed to load Intro.png");
      shut_down();
      exit(1);
   }
   int ibh = al_get_bitmap_height(introback);
   int ibw = al_get_bitmap_width(introback);

   titleback = al_load_bitmap("Graphics/Title.png");
   if(!titleback) {
      a5_error(AT, setting.screen, "Failed to load Title.png");
      shut_down();
      exit(1);
   }
   int tbh = al_get_bitmap_height(titleback);
   int tbw = al_get_bitmap_width(titleback);

   al_set_target_bitmap(titleback);
   al_draw_text(font_verdana_bmp, al_map_rgb_f(1, 1, 1), WIDTH / 2, HEIGHT - 64, ALLEGRO_ALIGN_CENTRE, VERSION);
   al_draw_text(font_verdana, al_map_rgb_f(1, 1, 1), WIDTH / 2, HEIGHT - 32, ALLEGRO_ALIGN_CENTRE, COPYRIGHT);
   al_set_target_bitmap(al_get_backbuffer(setting.screen));

   if(setting.sound_on) al_play_sample(swamp, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, NULL);

   // Expand intro screen from the right side of the screen until it fills
   // the entire screen.
   for(int x = WIDTH; x >= 0; x -= 16) {
      al_draw_scaled_bitmap(introback, 0, 0, tbw, tbh, x, 0, WIDTH - x, HEIGHT, 0);
      al_flip_display();
      al_rest(.015);
   }
   al_draw_bitmap(introback, 0, 0, 0);
   al_flip_display();

   wait_for_keypress(5);
   if(setting.music_on || setting.sound_on) al_stop_samples();

   // Move intro off screen and title on screen by compressing intro into left side
   // of the screen and expanding title from the right side at the same time.
   for(int y = HEIGHT; y >= 0; y -= 16) {
      al_draw_scaled_bitmap(introback, 0, 0, ibw, ibh, 0, 0, WIDTH, y, 0);
      al_draw_scaled_bitmap(titleback, 0, 0, tbw, tbh, 0, y, WIDTH, HEIGHT - y, 0);
      al_flip_display();
      al_rest(.015);
   }
   al_draw_bitmap(titleback, 0, 0, 0);
   al_flip_display();

   if(setting.music_on) al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);

   wait_for_keypress(0);

   //if(setting.music_on || setting.sound_on) al_stop_samples();

   for(float alpha = 1.0f; alpha >= 0.0f; alpha -= .05f) {
      al_clear_to_color(al_map_rgb(0, 0, 0));
      al_draw_tinted_bitmap(titleback, al_map_rgba_f(1 * alpha, 1 * alpha, 1 * alpha, alpha), 0, 0, 0);
      //al_draw_scaled_bitmap(titleback, 0, 0, tbw, tbh, 0, 0, x, HEIGHT, 0);
      al_flip_display();
      al_rest(.015);
   }

   al_destroy_bitmap(titleback);
   al_destroy_bitmap(introback);
   al_destroy_sample(swamp);
}


// returns true if loaded properly
bool load_map(LEVEL *lp, const char *filename, bool disk_load)
{
   char filepath[TEXT_BUFFER];

   if(disk_load) {
      al_set_standard_file_interface();
      strcpy(filepath, "Custom/");
      strcat(filepath, filename);
      printf("filepath = \"%s\"\n", filepath);
   }
   else {
      strcpy(filepath, "Levels/");
      strcat(filepath, filename);
      printf("filepath = \"%s\"\n", filepath);
   }

   ALLEGRO_FILE *file = NULL;

   file = al_fopen(filepath, "rb");
   if(file == NULL) {
      if(disk_load) {
         printf("%s(%d): \"%s\" not found on disk.\n", __FILE__, __LINE__, filepath);
         al_set_physfs_file_interface();
      }
      else
         printf("%s(%d): \"%s\" not found on in pak.\n", __FILE__, __LINE__, filepath);

      return 0;
   }

   al_fread(file, &lp->map_id, sizeof(char)*strlen(MAP_ID));
   lp->map_ver = al_fgetc(file);

   if(lp->map_ver <= MAP_VER && strcmp(lp->map_id, MAP_ID) != 0) {
      printf("%s(%d): Invalid map type or version.\n", __FILE__, __LINE__);
      printf("Map version was %d, expected version 1 or 2.\n", lp->map_ver);
      printf("Map id was \"%s\", expected \"%s\".\n", lp->map_id, MAP_ID);
      al_fclose(file);
      if(disk_load) al_set_physfs_file_interface();
      return false;
   }

   lp->validated = al_fgetc(file);
   lp->line_set = al_fgetc(file);
   if(lp->line_set >= max_linesets) lp->line_set = 0;
   lp->player.x = al_fgetc(file);
   lp->player.y = al_fgetc(file);
   for(int i = 0; i < 4; i++) {
      lp->ghost[i].x = al_fgetc(file);
      lp->ghost[i].y = al_fgetc(file);
   }
   lp->pickup.x = al_fgetc(file);
   lp->pickup.y = al_fgetc(file);
   if(lp->map_ver >= 3) {  // Teleports are in map version 3 or greater.
      for(int i = 0; i < 2; i++) {
         lp->teleport[i].x = al_fgetc(file);
         lp->teleport[i].y = al_fgetc(file);
      }
   }
   else {   // Old version map, set default teleport values (99 = not set)
      for(int i = 0; i < 2; i++) {
         lp->teleport[i].x = 99;
         lp->teleport[i].y = 99;
      }
   }
   lp->background = al_fgetc(file);
   if(!game_loaded) {
      for(unsigned char y = 0; y < MAPY; y++) {
         for(unsigned char x = 0; x < MAPX; x++) {
            lp->map[y][x].tile = al_fgetc(file);
            lp->map[y][x].is_pill = al_fgetc(file);
            lp->map[y][x].is_powerpill = al_fgetc(file);
            // Version 3+ maps have the "is_protected" flag. Used only in the level editor.
            if(level->map_ver >= 3) lp->map[y][x].is_protected = al_fgetc(file);
            else lp->map[y][x].is_protected = false;  // Older map, so just set it to false.
         }
      }
      al_fread(file, &lp->pills, sizeof(unsigned short));
   }

   // never forget to close the file
   al_fclose(file);

   if(disk_load) al_set_physfs_file_interface();

   return true;
}

bool load_map_image(LEVEL *lp, const char *filename, bool disk_load)
{
   ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);

   if(disk_load) {   // If disk_load is true, than we are loading a Custom level image
      al_set_standard_file_interface();   // Set the interface to load from the hard drive
      al_append_path_component(path, "Custom");
   }
   else {   // disk_load is not true, so we must be loading an image from the pakfile
      al_append_path_component(path, "Levels");
   }

   al_set_path_filename(path, filename);

   printf("path = \"%s\"\n", al_path_cstr(path, '/'));  // Output this info to the console for debugging

   ALLEGRO_BITMAP *level_intro = NULL;    // We will need a new bitmap for this image
   level_intro = al_load_bitmap(al_path_cstr(path, '/'));   // Try and load the image in
   if(disk_load) al_set_physfs_file_interface();   // If this was a Custom load, switch back to the pakfile
   al_destroy_path(path);  // Destroy this once we're done with it
   if(!level_intro) {   // If the level image didn't exist, report it for debugging and return.
      printf("No image found\n");
      return false;
   }
   // Center of screen - center of bitmap will give us the location to center small images on screen
   int bw = WIDTH / 2 - al_get_bitmap_width(level_intro) / 2;
   int bh = HEIGHT / 2 - al_get_bitmap_height(level_intro) / 2;

   al_set_target_bitmap(al_get_backbuffer(setting.screen)); // Set target drawing location to the buffer
   al_draw_bitmap(level_intro, bw, bh, 0);   // and draw the bitmap centered
   al_flip_display();   // flip the display, which changes our view to the buffer

   al_destroy_bitmap(level_intro);  // We're done with the bitmap, so we can destroy it (free memory)

   wait_for_keypress(5);   // Wait for a keypress, or 5 seconds, whichever comes first

   return true;
}


// display the help screen
void help(void)
{
   char buffer[TEXT_BUFFER];
   ALLEGRO_BITMAP *help_screen = NULL;
   const int HELP_SCREEN_COUNT = 5;

   for(int i = 1; i <= HELP_SCREEN_COUNT; i++) {
      snprintf(buffer, TEXT_BUFFER, "Graphics/Help/Help_%1d.png", i);
      help_screen = al_load_bitmap(buffer);
      if(!help_screen) {
         a5_error(AT, setting.screen, "Failed to load Help screen.");
         break;
      }
      al_draw_bitmap(help_screen, 0, 0, 0);
      al_flip_display();
      al_destroy_bitmap(help_screen); help_screen = NULL;
      if(wait_for_keypress(0)) break; // Wait for key press and exit if ESC is pressed.
   }
}


// display the credits screen
void credits(void)
{
   char buffer[TEXT_BUFFER];
   ALLEGRO_BITMAP *credits_screen = NULL;

   for(int i = 1; i <= 4; i++) {
      snprintf(buffer, TEXT_BUFFER, "Graphics/Credits/Credits_%1d.png", i);
      credits_screen = al_load_bitmap(buffer);
      if(!credits_screen) {
         a5_error(AT, setting.screen, "Failed to load Credits screen.");
         break;
      }
      al_draw_bitmap(credits_screen, 0, 0, 0);
      al_flip_display();
      al_destroy_bitmap(credits_screen); credits_screen = NULL;
      if(wait_for_keypress(0)) break; // Wait for key press and exit if ESC is pressed.
   }
}



// Loads a saved game
bool load_game(int cplayer)
{
   ALLEGRO_STATE state;

   al_store_state(&state, ALLEGRO_STATE_NEW_FILE_INTERFACE);
   al_set_standard_file_interface();

   const char filename[] = "Deluxe Pacman 2.sav";
   ALLEGRO_FILE *file = NULL;

   file = al_fopen(filename, "rb");
   if(file == NULL) {
      printf("%s(%d): Error opening \"%s\" for reading.\n", __FILE__, __LINE__, filename);
      message("NO SAVED GAME FOUND");
      al_restore_state(&state);
      return false;   // return false if load fails
   }

   al_fread(file, &player[cplayer].lives, sizeof(int));
   al_fread(file, &player[cplayer].oldlives, sizeof(int));
   al_fread(file, &player[cplayer].clevel, sizeof(int));
   al_fread(file, &player[cplayer].oldlevel, sizeof(int));
   al_fread(file, &player[cplayer].pvalue, sizeof(unsigned long));
   al_fread(file, &player[cplayer].extra, sizeof(int));
   al_fread(file, &player[cplayer].wrap, sizeof(bool));
   al_fread(file, &player[cplayer].energy, sizeof(int));

   for(unsigned char y = 0; y < MAPY; y++) {
      for(unsigned char x = 0; x < MAPX; x++) {
         player[cplayer].level.map[y][x].tile = al_fgetc(file);
         player[cplayer].level.map[y][x].is_pill = al_fgetc(file);
         player[cplayer].level.map[y][x].is_powerpill = al_fgetc(file);
      }
   }

   al_fread(file, &player[cplayer].level.pills, sizeof(unsigned short));

   al_fclose(file);

   al_restore_state(&state);

   return true;  // return true if save is successful
}


int main_menu(int cplayer)
{
   int i;
   typedef enum MENU_SELECTION { NONE = -1, PLAY, LOAD, FAME, OPTIONS, HELP, CREDITS, WEBSITE, QUIT } MENU_SELECTION;
   ALLEGRO_BITMAP *menuback = NULL;
   BUTTON b_play, b_load, b_fame, b_options, b_help, b_credits, b_website, b_quit;
   BUTTON b_1player, b_2player, b_3player, b_4player;
   bool menu_shutdown = false;
   bool done = false, menu_done = false;
   game_loaded = false;
   int start_y = 49; // If ABOUT is ever added again, this will be 49
   int players = 0;
   const int player_buttons = 4;
   BUTTON *p_menu_buttons[] = {&b_play, &b_load, &b_fame, &b_options, &b_help, &b_credits, &b_website, &b_quit};
   BUTTON *p_player_buttons[] = {&b_1player, &b_2player, &b_3player, &b_4player};
   MENU_SELECTION selection = PLAY; // PLAY is the first, maximum value is QUIT
   ALLEGRO_SAMPLE *menu_select = NULL, *menu_enter = NULL;
   ALLEGRO_KEYBOARD_STATE keys;
   ALLEGRO_MOUSE_STATE mouse;
   bool joy_up, joy_down, joy_left, joy_right, joy_button0, joy_button1, joy_start; /// JOYSTICK

   if(!new_button(&b_play, false, "Graphics/Buttons/Play.png", 272, start_y)) {
      printf("%s(%d): Error loading Play.png.\n", __FILE__, __LINE__);
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   if(!new_button(&b_load, false, "Graphics/Buttons/Load.png", 272, start_y + 64)) {
      printf("%s(%d): Error loading Load.png.\n", __FILE__, __LINE__);
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   if(!new_button(&b_fame, false, "Graphics/Buttons/Hall_Of_Fame.png", 272, start_y + 64 * 2)) {
      printf("%s(%d): Error loading Hall_Of_Fame.png.\n", __FILE__, __LINE__);
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   if(!new_button(&b_options, false, "Graphics/Buttons/Options.png", 272, start_y + 64 * 3)) {
      printf("%s(%d): Error loading Options.png.\n", __FILE__, __LINE__);
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   if(!new_button(&b_help, false, "Graphics/Buttons/Help.png", 272, start_y + 64 * 4)) {
      printf("%s(%d): Error loading Help.png.\n", __FILE__, __LINE__);
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   if(!new_button(&b_credits, false, "Graphics/Buttons/Credits.png", 272, start_y + 64 * 5)) {
      printf("%s(%d): Error loading Credits.png.\n", __FILE__, __LINE__);
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   if(!new_button(&b_website, false, "Graphics/Buttons/Website.png", 272, start_y + 64 * 6)) {
      printf("%s(%d): Error loading Website.png.\n", __FILE__, __LINE__);
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   if(!new_button(&b_quit, false, "Graphics/Buttons/Quit.png", 272, start_y + 64 * 7)) {
      printf("%s(%d): Error loading Quit.png.\n", __FILE__, __LINE__);
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   // Load 1, 2, 3 & 4 Player buttons
   if(!new_button(&b_1player, false, "Graphics/Buttons/1_Player.png", 272, start_y + 64)) {
      printf("%s(%d): Error loading 1_Player.png.\n", __FILE__, __LINE__);
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   if(!new_button(&b_2player, false, "Graphics/Buttons/2_Player.png", 272, start_y + 64 * 2)) {
      printf("%s(%d): Error loading 2_Player.png.\n", __FILE__, __LINE__);
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   if(!new_button(&b_3player, false, "Graphics/Buttons/3_Player.png", 272, start_y + 64 * 3)) {
      printf("%s(%d): Error loading 3_Player.png.\n", __FILE__, __LINE__);
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   if(!new_button(&b_4player, false, "Graphics/Buttons/4_Player.png", 272, start_y + 64 * 4)) {
      printf("%s(%d): Error loading 4_Player.png.\n", __FILE__, __LINE__);
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   // Load background image
   menuback = al_load_bitmap("Graphics/Menu_Background.png");
   if(!menuback) {
      a5_error(AT, setting.screen, "Failed to load Menu_Background.png");
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   menu_select = al_load_sample("Sound/Menu_Select.ogg");
   if(!menu_select) {
      a5_error(AT, setting.screen, "Failed to load Menu_Select.ogg");
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   menu_enter = al_load_sample("Sound/Menu_Enter.ogg");
   if(!menu_enter) {
      a5_error(AT, setting.screen, "Failed to load Menu_Enter.ogg");
      menu_shutdown = true;
      goto SHUTDOWN;
   }

   // We'll set the redraw state to true so the buttons get drawn the first time through
   setting.redraw = true;

   p_menu_buttons[selection]->state = 1;

   done = false;

   al_flush_event_queue(event_queue);

   do {
      al_wait_for_event(event_queue, &event);

      if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
         printf("Close button detected.\n");
         menu_shutdown = true;
         goto SHUTDOWN;
      }

      /// MOUSE CHECKS
      else if(event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
         al_get_mouse_state(&mouse);

         for(i = 0; i <= QUIT; i++) {
            if(check_button(p_menu_buttons[i], &mouse)) {
               p_menu_buttons[selection]->state = 0;
               selection = i;
               p_menu_buttons[selection]->state = 2;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
         }
      }

      else if(event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
         al_get_mouse_state(&mouse);
         setting.redraw = true;

         if(check_button(p_menu_buttons[selection], &mouse)) { // make certain mouse was over a button
            int options_select;  // cannot define this inside switch()

            switch(selection) {
               case PLAY: // PLAY - number of players are selected after end of while loop
                  menu_done = true;
                  break;
               case LOAD: // LOAD (saved game)
                  if(load_game(cplayer)) {
                     players = 1;   // note: if players == 1, than players selection screen will not show
                     game_loaded = true;
                     menu_done = true;
                  }
                  break;
               case FAME: // HALL OF FAME
                  if(score_table(player[cplayer].score, cplayer, player[cplayer].clevel)) {
                     menu_shutdown = true;
                     goto SHUTDOWN;
                  }
                  break;
               case OPTIONS: // OPTIONS
                  options_select = options_menu();
                  switch(options_select) {
                     case 0:  // Normal exit
                        setting.redraw = true;
                        break;
                     case 1:  // Screen mode changed, reset view
                        players = -1;  // we'll use -1 to indicate we need to reset the display
                        goto SHUTDOWN;
                     case 2:  // Error or close button, exit game.
                        menu_shutdown = true;
                        goto SHUTDOWN;
                  }
                  break;
               case HELP: // HELP
                  help();
                  break;
               case CREDITS: // CREDITS
                  credits();
                  break;
               case WEBSITE: // WEBSITE
                  system(DP2_WEBSITE);
                  break;
               case QUIT: // QUIT
                  menu_done = true;
                  menu_shutdown = true;
                  break;
               default:
                  printf("(%i)Invalid menu selection: %i\n", __LINE__, selection);
            }
         }

         // We need to check all the buttons and redraw due to how the mouse can jump around
         for(i = 0; i <= QUIT; i++) {
            setting.redraw = check_button(p_menu_buttons[i], &mouse) | setting.redraw;
         }
      }

      // This changes the buttons based on whether the mouse is over them, but we
      // probably shouldn't use this because we want the buttons to remain lit up
      // for keyboard reference.
      else if(event.type == ALLEGRO_EVENT_MOUSE_AXES) {
         al_get_mouse_state(&mouse);

         for(i = 0; i <= QUIT; i++) {
            bool cb = check_button(p_menu_buttons[i], &mouse);
            if(cb && (selection != i)) {
               p_menu_buttons[selection]->old_state = 1;
               p_menu_buttons[selection]->state = 0;
               selection = i;
               p_menu_buttons[selection]->old_state = 0;
               p_menu_buttons[selection]->state = 1;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            else if(selection == i) { // if our selection is no longer lit, relight it
               if(p_menu_buttons[selection]->state == 1 && p_menu_buttons[selection]->old_state == 0) {
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               }
            }
         }
      }

      /// JOYSTICK
      else if(event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
         if(event.joystick.button == 0) {                   // Button 0 = A (green, like pressing ENTER)
            p_menu_buttons[selection]->state = 2;
            setting.redraw = true;
            if(setting.sound_on)
               al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
         }
         else if(event.joystick.button == 1) {              // Button 1 = B (red, used like pressing ESC)
            p_menu_buttons[selection]->state = 0;
            selection = QUIT;
            p_menu_buttons[selection]->state = 2;
            menu_done = true;
            menu_shutdown = true;
            setting.redraw = true;
            if(setting.sound_on)
               al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
         }
         else if(event.joystick.button == 7) {              // Button 7 = Start (like clicking PLAY)
            p_menu_buttons[selection]->state = 0;
            selection = PLAY;
            p_menu_buttons[selection]->state = 2;
            setting.redraw = true;
            if(setting.sound_on)
               al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
         }
      }

      else if(event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP) {
         if(event.joystick.button == 0) {                   // Button 0 = A (green, like pressing ENTER)
            p_menu_buttons[selection]->state = 1;
            setting.redraw = true;

            int options_select;

            switch(selection) {
               case PLAY: // PLAY - number of players are selected after end of while loop
                  menu_done = true;
                  break;
               case LOAD: // LOAD (saved game)
                  if(load_game(cplayer)) {
                     players = 1;   // note: if players == 1, than players selection screen will not show
                     game_loaded = true;
                     menu_done = true;
                  }
                  break;
               case FAME: // HALL OF FAME
                  if(score_table(player[cplayer].score, cplayer, player[cplayer].clevel)) {
                     menu_shutdown = true;
                     goto SHUTDOWN;
                  }
                  break;
               case OPTIONS: // OPTIONS
                  options_select = options_menu();
                  switch(options_select) {
                     case 0:  // Normal exit
                        setting.redraw = true;
                        break;
                     case 1:  // Screen mode change, reset view.
                        players = -1;  // we'll use -1 to indicate we need to reset the display
                        goto SHUTDOWN;
                     case 2:  // Error or Close Button, exit game.
                        menu_shutdown = true;
                        goto SHUTDOWN;
                  }
                  break;
               case HELP: // HELP
                  help();
                  break;
               case CREDITS: // CREDITS
                  credits();
                  break;
               case WEBSITE: // WEBSITE
                  system(DP2_WEBSITE);
                  break;
               case QUIT: // QUIT
                  menu_done = true;
                  menu_shutdown = true;
                  break;
               default:
                  printf("(%i)Invalid menu selection: %i\n", __LINE__, selection);
            }
         }
         else if(event.joystick.button == 7) {              // Button 7 = Start (like clicking PLAY)
            p_menu_buttons[selection]->state = 1;
            setting.redraw = true;
            menu_done = true;
         }
      }

      else if(event.type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
         if(event.joystick.axis == 1) {         // Axis 1 = Y, 0 = X
            if(event.joystick.pos == -1.0f) {        // Up
               p_menu_buttons[selection]->state = 0;
               selection--;
               if(selection < PLAY) selection = QUIT;
               p_menu_buttons[selection]->state = 1;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            else if(event.joystick.pos == 1.0f) {   // Down
               p_menu_buttons[selection]->state = 0;
               selection++;
               if(selection > QUIT) selection = PLAY;
               p_menu_buttons[selection]->state = 1;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
         }
      }

      else if(event.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION) {
         al_reconfigure_joysticks();
      }

      /// Keyboard Checks
      else if(event.type == ALLEGRO_EVENT_KEY_DOWN) {
         switch(event.keyboard.keycode) {
            case ALLEGRO_KEY_DOWN:
            case ALLEGRO_KEY_PAD_2:
            case ALLEGRO_KEY_S:
               p_menu_buttons[selection]->state = 0;
               selection++;
               if(selection > QUIT) selection = PLAY;
               p_menu_buttons[selection]->state = 1;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               break;
            case ALLEGRO_KEY_UP:
            case ALLEGRO_KEY_PAD_8:
            case ALLEGRO_KEY_W:
               p_menu_buttons[selection]->state = 0;
               selection--;
               if(selection < PLAY) selection = QUIT;
               p_menu_buttons[selection]->state = 1;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               break;
            case ALLEGRO_KEY_ESCAPE:
               p_menu_buttons[selection]->state = 0;
               selection = QUIT;
               p_menu_buttons[selection]->state = 2;
               menu_done = true;
               menu_shutdown = true;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               break;
            case ALLEGRO_KEY_ENTER:
               p_menu_buttons[selection]->state = 2;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               break;
         }
      }

      else if(event.type == ALLEGRO_EVENT_KEY_UP) {
         // When ENTER is released, the buttons should pop back up.
         if(event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
            p_menu_buttons[selection]->state = 1;
            setting.redraw = true;

            int options_select;

            switch(selection) {
               case PLAY: // PLAY - number of players are selected after end of while loop
                  menu_done = true;
                  break;
               case LOAD: // LOAD (saved game)
                  if(load_game(cplayer)) {
                     players = 1;   // note: if players == 1, than players selection screen will not show
                     game_loaded = true;
                     menu_done = true;
                  }
                  break;
               case FAME: // HALL OF FAME
                  if(score_table(player[cplayer].score, cplayer, player[cplayer].clevel)) {
                     menu_shutdown = true;
                     goto SHUTDOWN;
                  }
                  break;
               case OPTIONS: // OPTIONS
                  options_select = options_menu();
                  switch(options_select) {
                     case 0:  // Normal exit
                        setting.redraw = true;
                        break;
                     case 1:  // Screen mode change, reset view.
                        players = -1;  // we'll use -1 to indicate we need to reset the display
                        goto SHUTDOWN;
                     case 2:  // Error or Close Button, exit game.
                        menu_shutdown = true;
                        goto SHUTDOWN;
                  }
                  break;
               case HELP: // HELP
                  help();
                  break;
               case CREDITS: // CREDITS
                  credits();
                  break;
               case WEBSITE: // WEBSITE
                  system(DP2_WEBSITE);
                  break;
               case QUIT: // QUIT
                  menu_done = true;
                  menu_shutdown = true;
                  break;
               default:
                  printf("(%i)Invalid menu selection: %i\n", __LINE__, selection);
            }
         }
         else if(event.keyboard.keycode == ALLEGRO_KEY_F12) {
            if(setting.sound_on)
               al_play_sample(sfx_camera, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            a5_screenshot("Screenshots/DeluxePacman2");
         }
      }

      if(event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
         printf("Display Switched In\n");
         setting.redraw = true;
      }

      if(event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
         printf("Display Switched Out\n");
      }

      if(event.type == ALLEGRO_EVENT_DISPLAY_FOUND) {
         printf("Display Switched Found\n");
         setting.redraw = true;
      }

      if(event.type == ALLEGRO_EVENT_DISPLAY_LOST) {
         printf("Display Switched Lost\n");
      }

      if(setting.redraw) {
         setting.redraw = false;

         al_set_target_bitmap(al_get_backbuffer(setting.screen));

         al_draw_bitmap(menuback, 0, 0, 0);

         for(i = 0; i <= QUIT; i++) draw_button(p_menu_buttons[i]);

         al_flip_display();
         done = menu_done;
      }
   }
   while(!done);

   // rest long enough for the player to see the button pressed animation and hear the sound effect
   al_rest(.75);

   // Select number of players here
   if(!players & !menu_shutdown) {  // players == 1 if a game is loaded, menu_shutdown is set on game exit
      selection = 0; // reset selection number
      setting.redraw = true;  // reset redraw state so the buttons get drawn the first time
      p_player_buttons[selection]->state = 1;
      done = false; menu_done = false;

      do {
         al_wait_for_event(event_queue, &event);

         if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            printf("Close button detected.\n");
            menu_shutdown = true;
            goto SHUTDOWN;
         }

         /// MOUSE CHECKS
         else if(event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            al_get_mouse_state(&mouse);

            for(i = 0; i < player_buttons; i++) {
               if(check_button(p_player_buttons[i], &mouse)) {
                  p_player_buttons[selection]->state = 0;
                  selection = i;
                  p_player_buttons[selection]->state = 2;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               }
            }
         }

         else if(event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            al_get_mouse_state(&mouse);
            setting.redraw = true;

            if(check_button(p_player_buttons[selection], &mouse)) { // make certain mouse was over a button
               switch(selection) {
                  case 0: // 1 PLAYER
                     players = 1;
                     menu_done = true;
                     break;
                  case 1: // 2 PLAYER
                     players = 2;
                     menu_done = true;
                     break;
                  case 2: // 3 PLAYER
                     players = 3;
                     menu_done = true;
                     break;
                  case 3: // 4 PLAYER
                     players = 4;
                     menu_done = true;
                     break;
                  default:
                     printf("(%i)Invalid menu selection: %i\n", __LINE__, selection);
               }
            }

            // We need to check all the buttons and redraw due to how the mouse can jump around
            for(i = 0; i < player_buttons; i++) {
               setting.redraw = check_button(p_player_buttons[i], &mouse) | setting.redraw;
            }
         }

         // This changes the buttons based on whether the mouse is over them, but we
         // probably shouldn't use this because we want the buttons to remain lit up
         // for keyboard reference.
         else if(event.type == ALLEGRO_EVENT_MOUSE_AXES) {
            al_get_mouse_state(&mouse);

            for(i = 0; i < player_buttons; i++) {
               bool cb = check_button(p_player_buttons[i], &mouse);
               if(cb && (selection != i)) {
                  p_player_buttons[selection]->old_state = 1;
                  p_player_buttons[selection]->state = 0;
                  selection = i;
                  p_player_buttons[selection]->old_state = 0;
                  p_player_buttons[selection]->state = 1;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               }
               else if(selection == i) { // if our selection is no longer lit, relight it
                  if(p_player_buttons[selection]->state == 1 && p_player_buttons[selection]->old_state == 0) {
                     setting.redraw = true;
                     if(setting.sound_on)
                        al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  }
               }
            }
         }

         /// JOYSTICK
         else if(event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
            if(event.joystick.button == 0) {                   // Button 0 = A (green, like pressing ENTER)
               p_player_buttons[selection]->state = 2;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            else if(event.joystick.button == 1) {              // Button 1 = B (red, used like pressing ESC)
               // On the player # selection screen, ESC will go back to the main menu
               p_player_buttons[selection]->state = 0;
               players = 0;  // we'll use 0 to signify that we wish to rerun the main menu
               menu_done = true;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
         }

         else if(event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP) {
            if(event.joystick.button == 0) {                   // Button 0 = A (green, like pressing ENTER)
               p_player_buttons[selection]->state = 1;
               setting.redraw = true;

               switch(selection) {
                  case 0: // 1 PLAYER
                     players = 1;
                     menu_done = true;
                     break;
                  case 1: // 2 PLAYER
                     players = 2;
                     menu_done = true;
                     break;
                  case 2: // 3 PLAYER
                     players = 3;
                     menu_done = true;
                     break;
                  case 3: // 4 PLAYER
                     players = 4;
                     menu_done = true;
                     break;
                  default:
                     printf("(%i)Invalid menu selection: %i\n", __LINE__, selection);
               }
            }
         }

         else if(event.type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
            if(event.joystick.axis == 1) {         // Axis 1 = Y, 0 = X
               if(event.joystick.pos == -1.0f) {        // Up
                  p_player_buttons[selection]->state = 0;
                  selection--;
                  if(selection < 0) selection = player_buttons - 1;
                  p_player_buttons[selection]->state = 1;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               }
               else if(event.joystick.pos == 1.0f) {   // Down
                  p_player_buttons[selection]->state = 0;
                  selection++;
                  if(selection > player_buttons - 1) selection = 0;
                  p_player_buttons[selection]->state = 1;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               }
            }
         }

         else if(event.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION) {
            al_reconfigure_joysticks();
         }

         /// Keyboard Checks
         else if(event.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch(event.keyboard.keycode) {
               case ALLEGRO_KEY_DOWN:
               case ALLEGRO_KEY_PAD_2:
               case ALLEGRO_KEY_S:
                  p_player_buttons[selection]->state = 0;
                  selection++;
                  if(selection > player_buttons - 1) selection = 0;
                  p_player_buttons[selection]->state = 1;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  break;
               case ALLEGRO_KEY_UP:
               case ALLEGRO_KEY_PAD_8:
               case ALLEGRO_KEY_W:
                  p_player_buttons[selection]->state = 0;
                  selection--;
                  if(selection < 0) selection = player_buttons - 1;
                  p_player_buttons[selection]->state = 1;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  break;
               case ALLEGRO_KEY_ESCAPE:
                  // On the player # selection screen, ESC will go back to the main menu
                  p_player_buttons[selection]->state = 0;
                  players = 0;  // we'll use 0 to signify that we wish to rerun the main menu
                  menu_done = true;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  break;
               case ALLEGRO_KEY_ENTER:
                  p_player_buttons[selection]->state = 2;
                  setting.redraw = true;
                  if(setting.sound_on)
                     al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  break;
            }
         }

         else if(event.type == ALLEGRO_EVENT_KEY_UP) {
            al_get_keyboard_state(&keys);

            // When ENTER is released, the buttons should pop back up.
            if(event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
               p_player_buttons[selection]->state = 1;
               setting.redraw = true;

               switch(selection) {
                  case 0: // 1 PLAYER
                     players = 1;
                     menu_done = true;
                     break;
                  case 1: // 2 PLAYER
                     players = 2;
                     menu_done = true;
                     break;
                  case 2: // 3 PLAYER
                     players = 3;
                     menu_done = true;
                     break;
                  case 3: // 4 PLAYER
                     players = 4;
                     menu_done = true;
                     break;
                  default:
                     printf("(%i)Invalid menu selection: %i\n", __LINE__, selection);
               }
            }
            else if(event.keyboard.keycode == ALLEGRO_KEY_F12) {
               if(setting.sound_on)
                  al_play_sample(sfx_camera, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               a5_screenshot("Screenshots/DeluxePacman2");
            }
         }

         if(event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
            printf("Display Switched In\n");
            setting.redraw = true;
         }

         if(event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
            printf("Display Switched Out\n");
         }

         if(event.type == ALLEGRO_EVENT_DISPLAY_FOUND) {
            printf("Display Switched Found\n");
            setting.redraw = true;
         }

         if(event.type == ALLEGRO_EVENT_DISPLAY_LOST) {
            printf("Display Switched Lost\n");
         }

         if(setting.redraw) {
            setting.redraw = false;

            al_set_target_bitmap(al_get_backbuffer(setting.screen));

            al_draw_bitmap(menuback, 0, 0, 0);

            for(i = 0; i < player_buttons; i++) draw_button(p_player_buttons[i]);

            al_flip_display();
            done = menu_done;
         }
      }
      while(!done);

      // rest long enough for the player to see the button pressed animation and hear the sound effect
      al_rest(.75);

   }


SHUTDOWN:
   al_destroy_sample(menu_select);
   al_destroy_sample(menu_enter);
   al_destroy_bitmap(menuback);

   for(i = 0; i <= QUIT; i++) destroy_button(p_menu_buttons[i]);
   for(i = 0; i < player_buttons; i++) destroy_button(p_player_buttons[i]);

   if(menu_shutdown) {
      shut_down();
      exit(0);
   }

   // Return the number of players.
   return players;
}


// Saves the current level the player is on
bool save_game(int cplayer)
{
   ALLEGRO_STATE state;

   al_store_state(&state, ALLEGRO_STATE_NEW_FILE_INTERFACE);
   al_set_standard_file_interface();

   const char filename[] = "Deluxe Pacman 2.sav";
   ALLEGRO_FILE *file = NULL;

   file = al_fopen(filename, "wb");
   if(file == NULL) {
      printf("%s(%d): Error opening \"%s\" for writing.\n", __FILE__, __LINE__, filename);
      return false;   // return false if save fails
   }

   al_fwrite(file, &player[cplayer].lives, sizeof(int));
   al_fwrite(file, &player[cplayer].oldlives, sizeof(int));
   al_fwrite(file, &player[cplayer].clevel, sizeof(int));
   al_fwrite(file, &player[cplayer].oldlevel, sizeof(int));
   al_fwrite(file, &player[cplayer].pvalue, sizeof(unsigned long));
   al_fwrite(file, &player[cplayer].extra, sizeof(int));
   al_fwrite(file, &player[cplayer].wrap, sizeof(bool));
   al_fwrite(file, &player[cplayer].energy, sizeof(int));

   for(unsigned char y = 0; y < MAPY; y++) {
      for(unsigned char x = 0; x < MAPX; x++) {
         al_fputc(file, level->map[y][x].tile);
         al_fputc(file, level->map[y][x].is_pill);
         al_fputc(file, level->map[y][x].is_powerpill);
      }
   }

   al_fwrite(file, &level->pills, sizeof(unsigned short));

   al_fclose(file);

   al_restore_state(&state);

   return true;  // return true if save is successful
}


bool game_menu(int cplayer)
{
   ALLEGRO_BITMAP *menuback = NULL;
   BUTTON b_resume, b_save, b_exit;
   bool exit_game = false;
   bool done = false, menu_done = false;
   int start_y = 236;
   int start_x = 322;
   int separation = 48;
   ALLEGRO_SAMPLE *menu_select = NULL, *menu_enter = NULL;
   ALLEGRO_KEYBOARD_STATE keys;
   ALLEGRO_MOUSE_STATE mouse;
   const int num_buttons = 3;
   BUTTON *p_select[] = {&b_resume, &b_save, &b_exit};
   int selection = 0; // RESUME is the first, maximum value is 3 (EXIT) at this time.
   bool close_button = false;

   if(!new_button(&b_resume, false, "Graphics/Buttons/Resume.png", start_x, start_y)) {
      printf("%s(%d): Error loading Resume.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }

   if(!new_button(&b_save, false, "Graphics/Buttons/Save_Small.png", start_x, start_y + separation)) {
      printf("%s(%d): Error loading Save_Small.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }

   if(!new_button(&b_exit, false, "Graphics/Buttons/Exit.png", start_x, start_y + separation * 2)) {
      printf("%s(%d): Error loading Quit.png.\n", __FILE__, __LINE__);
      goto SHUTDOWN;
   }

   // Copy game screen onto a bitmap
   menuback = al_create_bitmap(setting.w, setting.h);
   if(!menuback) {
      a5_error(AT, setting.screen, "Failed to create menuback");
      goto SHUTDOWN;
   }
   al_set_target_bitmap(menuback);
   al_draw_bitmap(al_get_backbuffer(setting.screen), 0, 0, 0);
   al_set_target_bitmap(al_get_backbuffer(setting.screen));

   menu_select = al_load_sample("Sound/Menu_Select.ogg");
   if(!menu_select) {
      a5_error(AT, setting.screen, "Failed to load Menu_Select.ogg");
      goto SHUTDOWN;
   }

   menu_enter = al_load_sample("Sound/Menu_Enter.ogg");
   if(!menu_enter) {
      a5_error(AT, setting.screen, "Failed to load Menu_Enter.ogg");
      goto SHUTDOWN;
   }

   // We'll set the redraw state to true so the buttons get drawn the first time through
   setting.redraw = true;

   p_select[selection]->state = 1;

   done = false;

   al_show_mouse_cursor(setting.screen);

   do {
      al_wait_for_event(event_queue, &event);

      if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
         close_button = true;
         done = true;
      }

      /// MOUSE CHECKS
      if(event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
         al_get_mouse_state(&mouse);

         for(int i = 0; i < num_buttons; i++) {
            if(check_button(p_select[i], &mouse)) {
               p_select[selection]->state = 0;
               selection = i;
               setting.redraw = true;
            }
         }
      }

      else if(event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
         al_get_mouse_state(&mouse);

         if(check_button(p_select[selection], &mouse)) { // make certain mouse was over a button
            p_select[selection]->state = 1;
            setting.redraw = true;

            switch(selection) {
               case 0: // RESUME - resume game play, may need to restart certain sounds
                  menu_done = true;
                  break;
               case 1: // SAVE - save game
                  if(!save_game(cplayer)) {
                     printf("Save game failed.\n");
                  }
                  else {
                     message("SAVE GAME SUCCESSFUL");
                     printf("Save game successful.\n");
                  }
                  break;
               case 2: // EXIT - Exit Game menu to main menu
                  menu_done = true;
                  exit_game = true;
                  break;
               default:
                  printf("(%i)Invalid menu selection: %i\n", __LINE__, selection);
            }
         }

         for(int i = 1; i < num_buttons - 1; i++) setting.redraw = check_button(p_select[i], &mouse) | setting.redraw;
      }

      // This changes the buttons based on whether the mouse is over them, but we
      // probably shouldn't use this because we want the buttons to remain lit up
      // for keyboard reference.
      else if(event.type == ALLEGRO_EVENT_MOUSE_AXES) {
         al_get_mouse_state(&mouse);
         for(int i = 0; i < num_buttons; i++) {
            bool cb = check_button(p_select[i], &mouse);
            if(cb && (selection != i)) {
               p_select[selection]->state = 0;
               selection = i;
               p_select[selection]->state = 1;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
         }
      }

      /// JOYSTICK
      else if(event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
         if(event.joystick.button == 0) {                   // Button 0 = A (green, like pressing ENTER)
            p_select[selection]->state = 2;
            setting.redraw = true;
            if(setting.sound_on)
               al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
         }
         else if(event.joystick.button == 1) {              // Button 1 = B (red, used like pressing ESC)
            p_select[selection]->state = 0;
            selection = num_buttons - 1;
            p_select[selection]->state = 2;
            menu_done = true;
            exit_game = true;
            setting.redraw = true;
            if(setting.sound_on)
               al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
         }
      }

      else if(event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP) {
         if(event.joystick.button == 0) {                   // Button 0 = A (green, like pressing ENTER)
            p_select[selection]->state = 1;
            setting.redraw = true;
            switch(selection) {
               case 0: // RESUME - resume game play, may need to restart certain sounds
                  menu_done = true;
                  break;
               case 1: // SAVE - save game
                  if(!save_game(cplayer)) {
                     printf("Save game failed.\n");
                  }
                  else {
                     message("SAVE GAME SUCCESSFUL");
                     printf("Save game successful.\n");
                  }
                  break;
               case 2: // EXIT - Exit Game menu to main menu
                  menu_done = true;
                  exit_game = true;
                  break;
               default:
                  printf("(%i)Invalid menu selection: %i\n", __LINE__, selection);
            }
         }
      }

      else if(event.type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
         if(event.joystick.axis == 1) {         // Axis 1 = Y, 0 = X
            if(event.joystick.pos <= -1.0) {        // Up
               p_select[selection]->state = 0;
               selection--;
               if(selection < 0) selection = num_buttons - 1;
               p_select[selection]->state = 1;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            else if(event.joystick.pos >= 1.0) {   // Down
               p_select[selection]->state = 0;
               selection++;
               if(selection == num_buttons) selection = 0;
               p_select[selection]->state = 1;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
         }
      }

      else if(event.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION) {
         al_reconfigure_joysticks();
      }

      /// Keyboard Checks
      else if(event.type == ALLEGRO_EVENT_KEY_DOWN) {
         switch(event.keyboard.keycode) {
            case ALLEGRO_KEY_DOWN:
            case ALLEGRO_KEY_PAD_2:
            case ALLEGRO_KEY_S:
               p_select[selection]->state = 0;
               selection++;
               if(selection == num_buttons) selection = 0;
               p_select[selection]->state = 1;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               break;
            case ALLEGRO_KEY_UP:
            case ALLEGRO_KEY_PAD_8:
            case ALLEGRO_KEY_W:
               p_select[selection]->state = 0;
               selection--;
               if(selection < 0) selection = num_buttons - 1;
               p_select[selection]->state = 1;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_select, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               break;
            case ALLEGRO_KEY_ESCAPE:
               p_select[selection]->state = 0;
               selection = num_buttons - 1;
               p_select[selection]->state = 2;
               menu_done = true;
               exit_game = true;
               setting.redraw = true;
               if(setting.sound_on)
                  al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               break;
            case ALLEGRO_KEY_ENTER:
               p_select[selection]->state = 2;
               setting.redraw = true;

               if(setting.sound_on)
                  al_play_sample(menu_enter, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               break;
            case ALLEGRO_KEY_F12:
               if(setting.sound_on)
                  al_play_sample(sfx_camera, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               a5_screenshot("Screenshots/DeluxePacman2");
               break;
         }
      }

      else if(event.type == ALLEGRO_EVENT_KEY_UP) {
         // When ENTER is released, the buttons should pop back up.
         if(event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
            p_select[selection]->state = 1;
            setting.redraw = true;
            switch(selection) {
               case 0: // RESUME - resume game play, may need to restart certain sounds
                  menu_done = true;
                  break;
               case 1: // SAVE - save game
                  /// TODO: Add in ability to have your own save filename
                  if(!save_game(cplayer)) {
                     printf("Save game failed.\n");
                  }
                  else {
                     message("SAVE GAME SUCCESSFUL");
                     printf("Save game successful.\n");
                  }
                  break;
               case 2: // EXIT - Exit Game menu to main menu
                  menu_done = true;
                  exit_game = true;
                  break;
               default:
                  printf("(%i)Invalid menu selection: %i\n", __LINE__, selection);
            }
         }
      }

      else if(event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
         printf("Game Menu: Display Switched In\n");
         setting.redraw = true;
      }

      else if(event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
         printf("Game Menu: Display Switched Out\n");
      }

      else if(event.type == ALLEGRO_EVENT_DISPLAY_FOUND) {
         printf("Game Menu: Display Switched Found\n");
         setting.redraw = true;
      }

      else if(event.type == ALLEGRO_EVENT_DISPLAY_LOST) {
         printf("Game Menu: Display Switched Lost\n");
      }


      if(setting.redraw) {
         setting.redraw = false;

         al_set_target_bitmap(al_get_backbuffer(setting.screen));
         if(setting.fullscreen == 1) al_draw_scaled_bitmap(menuback, offset_x, offset_y, WIDTH * scale_x, HEIGHT * scale_y,
                  0, 0, WIDTH, HEIGHT, 0);
         else al_draw_bitmap(menuback, 0, 0, 0);

         if(setting.shadows)
            al_draw_filled_rounded_rectangle(314, start_y - 10, 510, start_y + separation * num_buttons + 20,
                                             15, 15, al_map_rgba_f(0, 0, 0, .5));

         // Shadows are drawn, so now we'll draw the actual box on top
         al_draw_filled_rounded_rectangle(301, start_y - 23, 497, start_y + separation * num_buttons + 7,
                                          15, 15, al_map_rgba_f(0, 0, 0, .3));
         al_draw_rounded_rectangle(301, start_y - 23, 497, start_y + separation * num_buttons + 7,
                                   15, 15, al_map_rgb_f(.25, .25, 1), 4);


         for(int i = 0; i < num_buttons; i++) draw_button(p_select[i]);

         al_flip_display();
         done = menu_done;
      }
   }
   while(!done);

   al_hide_mouse_cursor(setting.screen);

   // rest long enough for the player to see the button pressed animation and hear the sound effect
   al_rest(.75);

SHUTDOWN:

   al_destroy_sample(menu_select);
   al_destroy_sample(menu_enter);
   al_destroy_bitmap(menuback);
   for(int i = 0; i < num_buttons; i++) destroy_button(p_select[i]);

   if(close_button) {
      shut_down();
      exit(0);
   }

   if(exit_game) return true;

   return false;
}

void reset_active_tools(void)
{
   tool.active = INACTIVE;

   // set tool_inuse[] array to 0 (false)
   memset(tool_inuse, 0, sizeof(tool_inuse));

   for(int i = 0; i < 4; i++) ghost[i].frozen = false;

   bullet.state = INACTIVE;
}


void pickup_check(int cplayer)
{
   int64_t redraw_timer = al_get_timer_count(setting.redraw_timer);

   // pickup or tool selection
   if(!pickup.active && (redraw_timer - pickup.cntr >= pickup.wait) &&
         !tool.active   && (redraw_timer - tool.cntr >= tool.wait)) {

      int r;

      // activate pickup or tool (0 = tool, 1+ = pickup)
      if(hack_detected) r = rand() % 4; /// *** HACK CHECK *** 25% chance tool will activate rather than 50%.
      else r = rand() % 2;

      // did player use a cheat mode key to force a tool active?
      if(tool.force) r = 0;

      reset_active_tools();
      tool.active = INACTIVE;
      pickup.active = INACTIVE;

      if(!r) {
         // activate tool!
         tool.active = ACTIVE;
         tool.current = rand() % MAX_TOOLS + 1;
         tool.cntr = redraw_timer;
         pickup.cntr = redraw_timer;

         /// This may need to be changed to a switch() statement in order to
         /// set the current tool properly in case the tools are not in the same order
         /// that the force tool enum is.
         if(tool.force) {  // tool.force is used when a CHEAT key is pressed
            tool.current = tool.force;
            tool.force = OFF;
         }

         // Check if player already has the EXTRA letter chosen and if so,
         // switch to a random food pickup instead.
         if((tool.current >= EXTRA_E && tool.current <= EXTRA_A)
               && (player[cplayer].extra & (int)pow(2, tool.current - EXTRA_E))) {
            r = 1;
            tool.active = INACTIVE;
         }
         // Check if the player already has the time tool and if so,
         // switch to a random food pickup instead.
         else if(tool.current == TIME && player[cplayer].extra_time) {
            r = 1;
            tool.active = INACTIVE;
         }
         // We definitely have a tool active, lets play the tool pickup sound
         else {
            if(setting.sound_on)
               al_play_sample(sfx_tool_pickup, setting.sound_volume, (pickup.x / PAN_RATIO), 1, ALLEGRO_PLAYMODE_ONCE, NULL);
         }
      }
      if(r) {
         // activate pickup!
         pickup.active = ACTIVE;
         pickup.current = rand() % MAX_PICKUPS;
         pickup.cntr = redraw_timer;
         tool.cntr = redraw_timer;

         // Play the normal food pickup sound
         if(setting.sound_on)
            al_play_sample(sfx_pickup, setting.sound_volume, (pickup.x / PAN_RATIO), 1, ALLEGRO_PLAYMODE_ONCE, NULL);
      }
   }

   // pickup active checks
   if(pickup.active && (redraw_timer - pickup.cntr >= pickup.screen_time)) pickup.active = INACTIVE;

   if(pickup.active) {
      if(setting.shadows)
         al_draw_tinted_bitmap(pickup.bitmap[pickup.current], al_map_rgba_f(0, 0, 0, .5),
                               pickup.x + SHADOW - 9, pickup.y + SHADOW - 9, 0);
      al_draw_bitmap(pickup.bitmap[pickup.current], pickup.x - 9, pickup.y - 9, 0);
   }

   // tool active checks
   if(tool.active && (redraw_timer - tool.cntr >= tool.screen_time)) tool.active = INACTIVE;

   if(redraw_timer - tool.cntr >= tool.use_time) reset_active_tools();

   if(tool.active) {
      if(setting.shadows)
         al_draw_tinted_bitmap(tool.bitmap[tool.current - 1], al_map_rgba_f(0, 0, 0, .5),
                               tool.x - 9 + SHADOW, tool.y - 9 + SHADOW, 0);
      al_draw_bitmap(tool.bitmap[tool.current - 1], tool.x - 9, tool.y - 9, 0);
   }
}

// Show a tiny floating score on screen
void float_points(unsigned long value, int x, int y)
{
   int64_t redraw_timer = al_get_timer_count(setting.redraw_timer);

   for(int i = 0; i < MAX_POINTS; i++) {
      if(point[i].value == 0) {
         point[i].value = value;
         point[i].x = x;
         point[i].y = y;
         point[i].counter = redraw_timer;
         point[i].colour = 1.0f;
         break;
      }
   }
}


void add_points(unsigned long points, bool isapill, bool isappill, bool isfloat, int cplayer)
{
   // If the present tool (tool 8) is active, add a random amount of points
   // between 1000 and 5000 if the player picked up a pill.
   if(isapill) {
      if(tool_inuse[PRESENT]) {
         points = (rand() % 4000) + 1000;
         isfloat = true;
         if(setting.sound_on) al_play_sample(sfx_pill, setting.sound_volume, setting.pan, 1,
                                                ALLEGRO_PLAYMODE_ONCE, NULL);
      }
      else if(tool_inuse[BLUE_DIAMOND]) {
         points = 1000;
         isfloat = true;
         if(setting.sound_on) al_play_sample(sfx_diamond, setting.sound_volume, setting.pan, 1,
                                                ALLEGRO_PLAYMODE_ONCE, NULL);
      }
      else if(tool_inuse[PINK_DIAMOND]) {
         points = 2000;
         isfloat = true;
         if(setting.sound_on) al_play_sample(sfx_diamond, setting.sound_volume, setting.pan, 1,
                                                ALLEGRO_PLAYMODE_ONCE, NULL);
      }
      else {
         if(setting.sound_on) al_play_sample(sfx_pill, setting.sound_volume, setting.pan, 1,
                                                ALLEGRO_PLAYMODE_ONCE, NULL);
      }
   }

   if(isapill || isappill) level->pills--;

   if(tool_inuse[TIMES2]) {
      points *= 2;
      isfloat = true;
   }
   else if(tool_inuse[TIMES5]) {
      points *= 5;
      isfloat = true;
   }
   else if(tool_inuse[TIMES7]) {
      points *= 7;
      isfloat = true;
   }

#ifdef HACK_PROTECTION
   /// *** HACK CHECK ***
   if(hack_detected) points = (points / points); // they get 1 point no matter what: points / points = 1 ;)
#endif // HACK_PROTECTION

   // Finally add the points to the current players score.
   player[cplayer].score += points;

   // Show small floating points on screen at current position
   if(isfloat) float_points(points, pacman.x, pacman.y);
}


void add_energy(int energy, int cplayer)
{
#ifdef HACK_PROTECTION
   /// *** HACK CHECK ***
   if(hack_detected) {
      if(SIGN(energy) == -1) energy <<= 1;   // If energy is a negative, we double it! :)
      else energy = SIGN(energy);            // If energy is positive, we change it to 1 (leaving it with SIGN macro to confuse them)
   }
#endif // HACK_PROTECTION
   player[cplayer].energy += energy;
}


void activate_bullet(void)
{
   int bullet_speed = 4 * MOVE_PIXELS;

   if(tool_inuse[GUN] && !bullet.state) {
      bullet.x = pacman.x;          // Bullet is spawned at the same place as pacman
      bullet.y = pacman.y;
      bullet.dx = pacman.dx * bullet_speed;    // We take the direction of pacman and multiply it by 4 so it moves faster.
      bullet.dy = pacman.dy * bullet_speed;
      bullet.d = 0;                 // The bullets duration on screen.
      bullet.rot = pacman.rot;      // The bullet needs to be rotated and flipped the same as pacman
      bullet.flip = pacman.flip;
      bullet.state = ACTIVE;        // Tell the drawing function to draw this bullet.
      if(setting.sound_on) {
         al_play_sample(bullet.sound, setting.sound_volume, (bullet.x / PAN_RATIO), 1,
                        ALLEGRO_PLAYMODE_ONCE, NULL);
      }
   }
}


void move_bullet(int cplayer)
{
   int bullet_speed = 4 * MOVE_PIXELS;
   int duration = 500 / bullet_speed;

   // Move bullet
   bullet.x += bullet.dx;
   bullet.y += bullet.dy;

   // Increment it's duration or life, it can travel 500 pixels.
   bullet.d++;
   if(bullet.d >= duration) bullet.state = INACTIVE;

   // Check to see if bullet went off screen
   else if(bullet.x >= setting.w) bullet.x = 0;
   else if(bullet.x < 0) bullet.x = setting.w - 1;
   else if(bullet.y >= setting.h) bullet.y = 0;
   else if(bullet.y < 0) bullet.y = setting.h - 1;

   // Check to see if bullet and ghost collided
   for(int i = 0; i < 4; i++) {  // check all 4 ghosts
      if(!ghost[i].dead) {
         // distance betweem ghost and bullet
         int xd = (ghost[i].x + SPRITE_SIZE / 2) - bullet.x; // distance away from ghost horizontally
         int yd = (ghost[i].y + SPRITE_SIZE / 2) - bullet.y; // distance away from ghost vertically
         int dist = SPRITE_SIZE / 2 + al_get_bitmap_width(bullet.bitmap) / 2; // distance for collision
         if(xd < dist && xd > -dist && yd < dist && yd > -dist) {
            ghost[i].dead = true;
            ghost[i].scared = false;
            ghost[i].frozen = false;
            add_points((unsigned long)(rand() % 4000 + 1000), false, false, true, cplayer);
            bullet.state = INACTIVE;
         }
      }
   }
}


void draw_map(int cplayer)
{
   al_store_state(&state, ALLEGRO_STATE_TARGET_BITMAP);

   al_set_target_bitmap(levelmap);

   al_clear_to_color(al_map_rgb_f(0, 0, 0));

   for(int y = 0; y < MAPY * TILE_SIZE; y += TILE_SIZE) {
      for(int x = 0; x < MAPX * TILE_SIZE; x += TILE_SIZE) {
         al_draw_bitmap(background[level->background], x, y, 0);
      }
   }

   // Draw Ghost spawns before lines so shadows show up on them
   for(int i = 0; i < 4; i++) {
      int x = ghost[i].spawn.x * TILE_SIZE;
      int y = ghost[i].spawn.y * TILE_SIZE;
      switch(i) { // draws colours squares where ghost spawns are
         case 0:
            al_draw_tinted_bitmap(ghost_spawn, al_map_rgb_f(1, .5, .5), x, y, 0);
            break;
         case 1:
            al_draw_tinted_bitmap(ghost_spawn, al_map_rgb_f(.5, 1, .5), x, y, 0);
            break;
         case 2:
            al_draw_tinted_bitmap(ghost_spawn, al_map_rgb_f(.5, 1, 1), x, y, 0);
            break;
         case 3:
            al_draw_tinted_bitmap(ghost_spawn, al_map_rgb_f(1, .5, 1), x, y, 0);
            break;
      }
   }

   // Draw pickup/tool spawn (but only if not on a bonus level)
   if(!player[cplayer].bonus_level)
      al_draw_tinted_bitmap(pickup_spawn, al_map_rgb_f(.75f, .75f, .75f), pickup.x, pickup.y, 0);


   // Draw teleports (if they exist)
   for(int i = 0; i < 2; i++) {
      if(level->teleport[i].x < 99) {
         int x = level->teleport[i].x * TILE_SIZE;
         int y = level->teleport[i].y * TILE_SIZE;
         al_draw_tinted_bitmap(teleport, al_map_rgb_f(1.0f, 1.0f, 1.0f), x, y, 0);
      }
   }

   for(int y = 0; y < MAPY; y++) {
      for(int x = 0; x < MAPX; x++) {
         int xpos = x * TILE_SIZE;
         int ypos = y * TILE_SIZE;
         if(!level->map[y][x].is_pill && level->map[y][x].tile) {
            int l = level->map[y][x].tile - 1;
            if(setting.shadows) {
               if(y == 0) {
                  int ly = level->map[MAPY - 1][x].tile - 1;
                  if(ly & 4) {
                     al_draw_tinted_bitmap(lines[level->line_set][ly], al_map_rgba(0, 0, 0, 127),
                                           xpos + SHADOW, ypos + SHADOW - TILE_SIZE, 0);
                  }
               }
               if(x == 0) {
                  int lx = level->map[y][MAPX - 1].tile - 1;
                  if(lx & 2) {
                     al_draw_tinted_bitmap(lines[level->line_set][lx], al_map_rgba(0, 0, 0, 127),
                                           xpos + SHADOW - TILE_SIZE, ypos + SHADOW, 0);
                  }
               }
               al_draw_tinted_bitmap(lines[level->line_set][l], al_map_rgba(0, 0, 0, 127),
                                     xpos + SHADOW, ypos + SHADOW, 0);
            }
            al_draw_bitmap(lines[level->line_set][l], xpos, ypos, 0);
         }
      }
   }

   al_restore_state(&state);
}


bool update_screen(int cplayer)
{
   bool bar = false;
   static bool oldbar = false;
   int redraw_timer = al_get_timer_count(setting.redraw_timer);

#ifdef HACK_PROTECTION
   /// *** HACK CHECK ***
   int HACK_VARIABLE1 = 0;  // I am setting random variables throughout my code to subvert hack software. :)
   int HACK_VARIABLE2 = 0;  // I am setting random variables throughout my code to subvert hack software. :)
#endif // HACK_PROTECTION

   ALLEGRO_BITMAP *tempBitmap = NULL;
   tempBitmap = al_create_bitmap(736, 544);
   if(!tempBitmap) {
      printf("%s(%d): Error creating tempBitmap.\n", __FILE__, __LINE__);
      return false;
   }

   al_set_target_bitmap(tempBitmap);

   al_draw_bitmap(levelmap, 0, 0, 0);

   // Draw pills
   for(int y = 0; y < MAPY; y++) {
      for(int x = 0; x < MAPX; x++) {
         int xpos = x * 32;
         int ypos = y * 32;
         if(level->map[y][x].is_pill && level->map[y][x].tile) {
            int p = level->map[y][x].tile - 1;
            int py = (int)(p / 8);
            int px = p - py * 8;
            if(tool_inuse[BLUE_DIAMOND] && !level->map[y][x].is_powerpill) {
               if(setting.shadows)
                  al_draw_tinted_bitmap(tool.bitmap[BLUE_DIAMOND - 1], al_map_rgba_f(0, 0, 0, .5),
                                        xpos - 9 + SHADOW, ypos - 9 + SHADOW, 0);
               al_draw_bitmap(tool.bitmap[BLUE_DIAMOND - 1], xpos - 9, ypos - 9, 0);
            }
            else if(tool_inuse[PINK_DIAMOND] && !level->map[y][x].is_powerpill) {
               if(setting.shadows)
                  al_draw_tinted_bitmap(tool.bitmap[PINK_DIAMOND - 1], al_map_rgba_f(0, 0, 0, .5),
                                        xpos - 9 + SHADOW, ypos - 9 + SHADOW, 0);
               al_draw_bitmap(tool.bitmap[PINK_DIAMOND - 1], xpos - 9, ypos - 9, 0);
            }
            else {
               if(setting.shadows)
                  al_draw_tinted_bitmap(pills[py][px], al_map_rgba_f(0, 0, 0, .5),
                                        xpos + SHADOW, ypos + SHADOW, 0);
               al_draw_bitmap(pills[py][px], xpos, ypos, 0);
            }
         }
      }
   }

   // check for active pickups or tools (not used for bonus levels)
   if(!pacman.dead && !player[cplayer].bonus_level) pickup_check(cplayer);

   if(setting.shadows) {
      // Draw Ghost shadows
      for(int i = 0; i < 4; i++) {
         if(ghost[i].dead)
            al_draw_tinted_bitmap(ghost_eyes[ghost[i].eyes], al_map_rgba_f(0, 0, 0, .5),
                                  ghost[i].x - 9 + SHADOW, ghost[i].y - 9 + SHADOW, 0);
         else if(ghost[i].scared && !ghost[i].frozen)
            al_draw_tinted_bitmap(blue_ghost[ghost[i].cur_img], al_map_rgba_f(0, 0, 0, .5),
                                  ghost[i].x - 9 + SHADOW, ghost[i].y - 9 + SHADOW, 0);
         else if(!ghost[i].frozen) {
            al_draw_tinted_bitmap_region(ghost[i].bitmap[ghost[i].eyes], al_map_rgba_f(0, 0, 0, .5),
                                         0, 0, 50, 44, ghost[i].x - 9 + SHADOW, ghost[i].y - 9 + SHADOW, 0);
            al_draw_tinted_bitmap_region(ghost[i].bitmap[ghost[i].cur_img], al_map_rgba_f(0, 0, 0, .5),
                                         0, 45, 50, 5, ghost[i].x - 9 + SHADOW, ghost[i].y + 44 - 9 + SHADOW, 0);
         }
      }
      // Draw Pacman's shadow (draw all shadows before actual characters)
      al_draw_tinted_rotated_bitmap(pacman.bitmap[pacman.cur_img], al_map_rgba_f(0, 0, 0, .5),
                                    25, 25, pacman.x + SHADOW, pacman.y + SHADOW, pacman.rot, pacman.flip);

      if(bullet.state) al_draw_tinted_rotated_bitmap(bullet.bitmap, al_map_rgba_f(0, 0, 0, .5),
               7.5f, 7.5f, bullet.x + SHADOW, bullet.y + SHADOW, bullet.rot, bullet.flip);

   }

   // Draw Ghosts
   for(int i = 0; i < 4; i++) {
      if(ghost[i].dead)
         al_draw_bitmap(ghost_eyes[ghost[i].eyes], ghost[i].x - 9, ghost[i].y - 9, 0);
      else if(ghost[i].scared && (ghost[i].stime - ghost[i].stimer <= (ghost[i].stime / 3)) && (ghost[i].stimer % 2))
         // flashing ghost transition from scared to pissed off ;)
         al_draw_bitmap(ghost[i].bitmap[4], ghost[i].x - 9, ghost[i].y - 9, 0);
      else if(ghost[i].scared) // Just draw blue (scared) ghost
         al_draw_bitmap(blue_ghost[ghost[i].cur_img], ghost[i].x - 9, ghost[i].y - 9, 0);
      else {
         al_draw_bitmap_region(ghost[i].bitmap[ghost[i].eyes], 0, 0, 50, 44,
                               ghost[i].x - 9, ghost[i].y - 9, 0);
         al_draw_bitmap_region(ghost[i].bitmap[ghost[i].cur_img], 0, 45, 50, 5,
                               ghost[i].x - 9, ghost[i].y + 35, 0);
      }
   }

   // Draw an ice cube over each frozen ghost
   if(tool_inuse[FREEZE]) {
      int frozen_ghost = 0;
      for(int i = 0; i < 4; i++) {
         if(ghost[i].frozen) {
            al_draw_bitmap(ice_cube, ghost[i].x - 19, ghost[i].y - 19, 0);
            frozen_ghost++;
         }
      }
      if(frozen_ghost == 0) tool_inuse[FREEZE] = false;  // No more ghostsicles, turn off the freeze timer.
   }

   // Draw Pacman
   al_draw_rotated_bitmap(pacman.bitmap[pacman.cur_img], 25, 25, pacman.x, pacman.y, pacman.rot, pacman.flip);

   if(bullet.state) al_draw_rotated_bitmap(bullet.bitmap, 7.5f, 7.5f, bullet.x, bullet.y, bullet.rot, bullet.flip);

   // Draw shield if active
   if(tool_inuse[SHIELDS] && !pacman.dead)
      al_draw_rotated_bitmap(shield, 37.5, 37.5, pacman.x, pacman.y, 0, 0);

   // "God" mode just draws a circle to differentiate it from the Shield tool.
   if(player[cplayer].god_mode && !pacman.dead)
      al_draw_circle(pacman.x, pacman.y, 30, al_map_rgba_f(.5, .5, .5, .5), 3);

   // update any tiny points
   if(!pacman.dead) {
      for(int i = 0; i < MAX_POINTS; i++) {
         if(point[i].value != 0) {
            int redraw_timer = al_get_timer_count(setting.redraw_timer);
            if(redraw_timer - point[i].counter >= point[i].delay) {
               point[i].moved++;
               point[i].y -= 2;
               point[i].colour = 1.0f - (1.0f / point[i].move) * point[i].moved;

               if(point[i].moved == point[i].move) { // point has vanished, remove it.
                  point[i].value = 0;
                  point[i].moved = 0;
               }
               else // point is still visible, draw it.
                  point[i].counter = redraw_timer;
            }
            int x = point[i].x;
            int y = point[i].y;
            float c = point[i].colour;

            // Shadows for floating numbers are drawn here so they appear on top of everything
            if(setting.shadows)
               al_draw_textf(font_radio_space_bold, al_map_rgba_f(0, 0, 0, c), x + SHADOW, y + SHADOW,
                             ALLEGRO_ALIGN_CENTRE, "%lu", point[i].value);
            al_draw_textf(font_radio_space_bold, al_map_rgba_f(c, c, c, c), x, y, ALLEGRO_ALIGN_CENTRE,
                          "%lu", point[i].value);
         }
      }
   }

   al_set_target_bitmap(al_get_backbuffer(setting.screen));

   al_clear_to_color(al_map_rgb_f(0, 0, 0));

   al_draw_bitmap(tempBitmap, 32, 32, 0);

   al_destroy_bitmap(tempBitmap);   tempBitmap = NULL;

   // Draw player score on screen
   al_draw_textf(font_fatfont_grn, al_map_rgb_f(1, 1, 1), 64, 6, ALLEGRO_ALIGN_LEFT,
                 "%dUP", cplayer + 1);
   al_draw_textf(font_digital, al_map_rgb_f(0, 1, 0), 124, 6, ALLEGRO_ALIGN_LEFT,
                 "%9ld", player[cplayer].score);

   // Draw the FPS counter at the top center of the screen
   if(setting.cheat_mode)
      al_draw_textf(font_verdana_bmp, al_map_rgb_f(1, 1, 1), 364, 5, ALLEGRO_ALIGN_LEFT, "%.0f FPS", fps);

   // Draw high score on screen
   al_draw_textf(font_fatfont_red, al_map_rgb_f(1, 1, 1), 544, 6, ALLEGRO_ALIGN_LEFT,
                 "HI");
   al_draw_textf(font_digital, al_map_rgb_f(1, 0, 0), 604, 6, ALLEGRO_ALIGN_LEFT,
                 "%9ld", hiscore);

   // Draw Hearts, first draw one red heart for each life remaining
   for(int i = 0; i < player[cplayer].lives; i++) al_draw_bitmap(heart[1], 64 + 28 * i, 575, 0);
   // Then draw one blue heart for each one lost.
   for(int i = player[cplayer].lives; i < MAX_LIVES; i++) al_draw_bitmap(heart[0], 64 + 28 * i, 575, 0);

   // Draw the EXTRA letters the player has collected at the bottom center
   for(int i = 0; i < 5; i++) {
      if(player[cplayer].extra & (int)pow(2, i)) {
         int bw = al_get_bitmap_width(tool.bitmap[15 + i]);
         int bh = al_get_bitmap_height(tool.bitmap[15 + i]);
         al_draw_scaled_bitmap(tool.bitmap[EXTRA_E - 1 + i], 0, 0, bw, bh, 308 + i * 24, 575, 24, 24, 0);
      }
   }

   // Draw energy bar (using constant EMULT we calculated at the top to fit it inside the box).
   al_draw_filled_rectangle(600.5, 580.5, 600.5 + player[cplayer].energy * EMULT, 595.5, al_map_rgb_f(1, .54, 0));
   // Draw grey box around total energy area.
   al_draw_rectangle(600.5, 580.5, 736.5, 595.5, al_map_rgb_f(.5, .5, .5), 2);

   int current_tool = 0;
   // display the tool active bar on the right side
   for(int i = 1; i < MAX_TOOLS - 1; i++) { // 1 to MAX_TOOLS-1 (last tool is SKULL and zero is not used)
      if(tool_inuse[i]) {                  // (actually, zero is used internally for OFF)
         bar = true;
         current_tool = i - 1; // bitmaps are 0 to MAX_TOOLS-1, so we subtract 1
         break;
      }
   }

   if(player[cplayer].extra_time) {
      int bw = al_get_bitmap_width(tool.bitmap[TIME - 1]);
      int bh = al_get_bitmap_height(tool.bitmap[TIME - 1]);
      al_draw_scaled_bitmap(tool.bitmap[TIME - 1], 0, 0, bw, bh, 768, 340, 32, 32, 0);
   }

   if(bar) {
      int bw = al_get_bitmap_width(tool.bitmap[current_tool]);
      int bh = al_get_bitmap_height(tool.bitmap[current_tool]);
      al_draw_scaled_bitmap(tool.bitmap[current_tool], 0, 0, bw, bh, 768, 0, 32, 32, 0);

      float y2 = ((float)redraw_timer - (float)tool.cntr) / ((float)tool.use_time / 300.0f);
      int x = 776; // 800 - 24
      int y = (int)y2 + 32;
      al_draw_filled_rounded_rectangle(x, y, x + 16, 337, 5, 5, al_map_rgb_f(0.0f, 1.0f, 0.0f));
      if(setting.sound_on && !oldbar) {
         al_play_sample(sfx_type, setting.sound_volume / 2, 0, 1, ALLEGRO_PLAYMODE_LOOP, &sample_id);
         sample_id_on = true;
         oldbar = true;
      }
   }
   else if(oldbar) {
      if(sample_id_on) al_stop_sample(&sample_id);
      sample_id_on = false;  // hacking my way around Allegro 5's horrible sound functions
      oldbar = false;
   }
   else {
      if(pacman.ts != TIMER_SPEED) {
         pacman.ts = TIMER_SPEED + (TIMER_SPEED * setting.suicide);
         if(pacman.fast) al_set_timer_speed(pacman.timer, 1.0 / (pacman.ts * 1.5));
         else al_set_timer_speed(pacman.timer, 1.0 / pacman.ts);
      }
      if(ghost[0].ts != TIMER_SPEED) {
         ghost[0].ts = TIMER_SPEED + (TIMER_SPEED * setting.suicide);
         al_set_timer_speed(ghost[0].timer, 1.0 / ghost[0].ts);
      }
   }

#ifdef HACK_PROTECTION
   /// *** HACK CHECK ***
   if(player[cplayer].lives > MAX_LIVES) {
      hack_detected = true;
#ifdef DEBUG
      printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
   }
   if(HACK_VARIABLE1 == 0) {
      HACK_VARIABLE1 = NEW_LIFE_SCORE;
   }
   else {
      hack_detected = true; // If the hack variable doesn't equal 0, someone changed it externally.
#ifdef DEBUG
      printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
   }
   if(HACK_VARIABLE2 == 0) {
      HACK_VARIABLE2 = MAX_LIVES;
   }
   else {
      hack_detected = true; // If the hack variable doesn't equal 0, someone changed it externally.
#ifdef DEBUG
      printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
   }
#endif // HACK_PROTECTION

   return true;
}


// Player died
void player_dies(int cplayer)
{
   ALLEGRO_BITMAP *gameover = NULL;
   gameover = al_load_bitmap("Graphics/Game_Over.png");
   if(!gameover) {
      a5_error(AT, setting.screen, "Failed to Game_Over.png");
      shut_down();
      exit(1);
   }
   int gh = al_get_bitmap_height(gameover);
   int gw = al_get_bitmap_width(gameover);
   al_lock_bitmap(gameover, al_get_bitmap_format(gameover), ALLEGRO_LOCK_READONLY);

#ifdef HACK_PROTECTION
   /// *** HACK CHECK ***
   player[cplayer].oldenergy = player[cplayer].energy;
   player[cplayer].oldlives = player[cplayer].lives;
#endif // HACK_PROTECTION

   ALLEGRO_BITMAP *tempBitmap = NULL;
   tempBitmap = al_create_bitmap(736, 544);
   if(!tempBitmap) {
      printf("%s(%d): Error creating tempBitmap.\n", __FILE__, __LINE__);
      shut_down();
      exit(1);
   }

   if(setting.sound_on) al_stop_samples();

   init_pacman();
   al_set_timer_speed(pacman.timer, 1.0 / pacman.ts);

   // If ghost was moving slow (GLUE) than reset it's speed to normal.
   if(ghost[0].ts != TIMER_SPEED) {
      ghost[0].ts = TIMER_SPEED + (TIMER_SPEED * setting.suicide);
      al_set_timer_speed(ghost[0].timer, 1.0 / ghost[0].ts);
   }

   pickup.active = INACTIVE;
   tool.active = INACTIVE;

   // Reset all tools to inactive, or zero.
   memset(tool_inuse, 0, sizeof(tool_inuse));

   if(setting.sound_on)
      al_play_sample(sfx_death, setting.sound_volume, setting.pan, 1, ALLEGRO_PLAYMODE_ONCE, NULL);

   // Show Pacman death animation.
   for(int i = 4; i < 25; i++) {
      pacman.cur_img = i;
      update_screen(cplayer);
      al_flip_display();
      al_rest(.08);
   }

   player[cplayer].lives--;
   if(player[cplayer].lives < 0) {
      player[cplayer].lives = 0;
      player[cplayer].extra = 0;
   }

   player[cplayer].energy = 0;

   player[cplayer].extra_time = false;
   tool.use_time = (TOOL_USE_TIME >> setting.suicide);

   // If player runs out of lives, show GAME OVER animation...
   if(player[cplayer].lives == 0) {
      if(setting.sound_on)
         al_play_sample(sfx_laugh, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);

      for(int i = 50; i <= gh; i++) {
         update_screen(cplayer);

         al_draw_bitmap_region(gameover, 0, 0, gw, i, WIDTH / 2 - gw / 2, HEIGHT / 2 - gh / 2, 0);
         al_flip_display();
         al_rest(.1);
      }
   }
#ifdef HACK_PROTECTION
   else if(player[cplayer].oldlives == player[cplayer].lives) {
      hack_detected = true; /// *** HACK CHECK ***
#ifdef DEBUG
      printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
   }

   /// *** HACK CHECK ***
   if(player[cplayer].energy) {
      hack_detected = true;
#ifdef DEBUG
      printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
   }
#endif // HACK_PROTECTION

   al_rest(2);

   // Destroy temporary bitmaps
   al_unlock_bitmap(gameover);
   al_destroy_bitmap(gameover);
   gameover = NULL;

   al_destroy_bitmap(tempBitmap);
   tempBitmap = NULL;

   // Player died, lets make sure we clear any key presses from the queue.
   al_flush_event_queue(event_queue);

   if(player[cplayer].lives) {
      // Reset pacman's position and animation frame.
      pacman.cur_img = 1;
      pacman.map = level->player;
      pacman.x = pacman.ox = pacman.map.x * TILE_SIZE + TILE_SIZE / 2; // tile# * 32pixels wide + half tile width
      pacman.y = pacman.oy = pacman.map.y * TILE_SIZE + TILE_SIZE / 2; // tile# * 32pixels high + half tile height
      pacman.dx = 1;
      pacman.dy = 0;
   }
}


int main(int argc, char *argv[])
{
   bool redraw = false;
   bool pressed_key[ALLEGRO_KEY_MAX];
   double start_time;
   char levelname[TEXT_BUFFER] = "";
   bool player_found = false;
   bool scared_ghost = false;
   int players = 0;
   int cplayer = 0;
   bool skip_level = false;
   ALLEGRO_SAMPLE_ID back1, back2;
   int backplay, oldbackplay;
   float pitch;
   int jump;   // new level to jump to
   //int path_node[MAPY][MAPX];

#ifdef HACK_PROTECTION
   /// *** HACK CHECK ***
   int HACK_VARIABLE1 = 0, HACK_VARIABLE2 = 0;
#endif // HACK_PROTECTION

   initialize();
   if(init_hiscore()) {
      shut_down();
      exit(1);
   }
   highscore_initialized = true;

   // Scan for the number of levels in the pakfile
   int levelcount = 0;
   while(1) {
      snprintf(levelname, TEXT_BUFFER, "Levels/level%03d.dp2", levelcount + 1);
      ALLEGRO_FILE *file = NULL;
      file = al_fopen(levelname, "rb");
      if(file) {
         levelcount++;
         al_fclose(file);
      }
      else break;
   }
   printf("Pak Levels found: %d\n", levelcount);
   al_draw_textf(font_verdana, al_map_rgb_f(1, 1, 1), WIDTH / 2, HEIGHT / 2 + 20, ALLEGRO_ALIGN_CENTRE, "Pak Levels found: %d", levelcount);
   al_flip_display();
   int paklevels = levelcount;   // Store number of pakfile levels found.

   levelcount = 0;   // Reset levelcount so we can check for replacement levels as well.
   int custom_levels = 0;
   al_set_standard_file_interface();
   while(1) {
      snprintf(levelname, TEXT_BUFFER, "Custom/level%03d.dp2", levelcount + 1);
      ALLEGRO_FILE *file = NULL;
      file = al_fopen(levelname, "rb");
      if(file) {
         levelcount++;
         al_fclose(file);
         custom_levels++;
      }
      else {
         // only break if we don't find any levels above the number in the pakfile
         // otherwise we should keep scanning as some may be just odd replacement levels
         // here and there.  Once we are above the number in the pakfile, then we can
         // break if we don't find anymore.
         if(levelcount > paklevels) {
            if(custom_levels == 0) levelcount = paklevels;
            break;
         }
         else levelcount++;   // increment the level as this matches a pak level
      }
   }
   al_set_physfs_file_interface();

   printf("Custom Levels found: %d\n", custom_levels);
   al_draw_textf(font_verdana, al_map_rgb_f(1, 1, 1), WIDTH / 2, HEIGHT / 2 + 40, ALLEGRO_ALIGN_CENTRE,
                 "Custom Levels found: %d", custom_levels);
   al_flip_display();
   printf("Highest level ### = %d\n", (levelcount > paklevels) ? levelcount : paklevels);

   // Now we check which has the higher level number, the pakfile or the current levelcount
   // which now has only the custom level number max.  Which ever is higher is the one
   // we will store into levelcount which can then be safely used later on.
   if(paklevels > levelcount) levelcount = paklevels;

   al_draw_textf(font_verdana, al_map_rgb_f(1, 1, 1), WIDTH / 2, HEIGHT / 2 + 60, ALLEGRO_ALIGN_CENTRE,
                 "Highest level number: %d", levelcount);
   al_flip_display();


   int bonuscount = 0;
   bool file_found;
   do {
      snprintf(levelname, TEXT_BUFFER, "Levels/bonus%03d.dp2", bonuscount + 1);
      file_found = al_filename_exists(levelname);
      if(file_found) bonuscount++;
   }
   while(file_found);

   printf("Bonus levels found: %d\n", bonuscount);

#ifdef HACK_PROTECTION
   /// *** HACK CHECK*** (only check this if pakman is not set)
   if(!setting.pakman) {
      if(bonuscount != MAX_BONUS) {
         hack_detected = true;
#ifdef DEBUG
         printf("*** Hack Detected: %d ***\n", __LINE__);
         printf("bonuscount = %d (should be %d)\n", bonuscount, MAX_BONUS);
#endif // DEBUG
      }
      if(paklevels != MAX_LEVELS) {
         hack_detected = true;
#ifdef DEBUG
         printf("*** Hack Detected: %d ***\n", __LINE__);
         printf("paklevels = %d (should be %d)\n", paklevels, MAX_LEVELS);
#endif // DEBUG
      }
   }
   /// *** HACK CHECK ***
   if(player[cplayer].lives > MAX_LIVES) {
      hack_detected = true;
#ifdef DEBUG
      printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
   }
   if(HACK_VARIABLE1 == 0) {
      HACK_VARIABLE1 = NEW_LIFE_SCORE;
   }
   else {
      hack_detected = true; // If the hack variable doesn't equal 0, someone changed it externally.
#ifdef DEBUG
      printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
   }
   if(HACK_VARIABLE2 == 0) {
      HACK_VARIABLE2 = MAX_LIVES;
   }
   else {
      hack_detected = true; // If the hack variable doesn't equal 0, someone changed it externally.
#ifdef DEBUG
      printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
   }
#endif // HACK_PROTECTION

   // Wait at least 5 seconds on loading screen so people can see how many levels were found.
   wait_for_keypress(5);

   // Show the intro sequence before the main menu loop so we don't keep seeing it
   intro(); // be sure to do this AFTER initialize()


   while(!exit_game) {
      init_player();
      cplayer = 0;
      players = 0;
      pacman.dead = false;

      // Display Main Menu
      while(players < 1) {
         players = main_menu(cplayer);
         if(players == -1) {
            shutdown_display();
            init_display(true);
         }
      }

      // Don't stop the music until after we're totally done with the main menu.
      if(setting.music_on || setting.sound_on) al_stop_samples();

      // Get #1 Highscore for this difficulty
      hiscore = get_hiscore(setting.difficulty, 0);

      // At this point we will know how many players there will be from
      // the main menu selection
      if(!game_loaded) {
         for(int i = 0; i < players; i++) {
            player[i].lives = 3;       // set starting lives to 3, maximum is 5
            player[i].oldlives = 3;
         }
      }

      init_pacman();
      init_ghosts();

      // Quick hack to fix the suicide not turning back off bug.
      if(!setting.suicide) {
         pacman.ts = TIMER_SPEED;
         if(pacman.fast) al_set_timer_speed(pacman.timer, 1.0 / (pacman.ts * 1.5));
         else al_set_timer_speed(pacman.timer, 1.0 / pacman.ts);
         ghost[0].ts = TIMER_SPEED;
         al_set_timer_speed(ghost[0].timer, 1.0 / ghost[0].ts);
         tool.use_time = TOOL_USE_TIME;
         for(int i = 0; i < 4; i++) ghost[i].stime = (int)(REDRAW_TIMER * 0.75f) >> setting.suicide;
      }

      game_over = false;
      bool disk_load = true;

      al_hide_mouse_cursor(setting.screen);

      while(!game_over) {
         init_variables();
         //printf("%d: game_over = %s\n", __LINE__, game_over?"true":"false");

         // when a player dies, find the next player that has lives
         if(pacman.dead) {
            player_found = false;

            // first we check the next players AFTER the current player to see if they have a turn coming up.
            if(cplayer < 3) {
               for(int i = cplayer + 1; i < 4; i++) {
                  if(player[i].lives > 0) {
                     cplayer = i;
                     player_found = true;
                     break;
                  }
               }
            }

            // if we found nobody, then we check the players that come before the current player, this way
            // we ensure the players get to play in the proper order.  I am sure there is a better way to do
            // this, I'm just to lazy to redo this mess. :)
            if(cplayer > 0 && !player_found) {
               for(int i = 0; i < cplayer; i++) {
                  if(player[i].lives > 0) {
                     cplayer = i;
                     player_found = true;
                     break;
                  }
               }
            }
         }

         player[cplayer].oldlives = player[cplayer].lives;

         // create a pointer to the level data or some of these lines can get long.
         level = &player[cplayer].level;
         if((player[cplayer].clevel != player[cplayer].oldlevel) || player[cplayer].bonus_level || game_loaded) {
            // first we try to load a custom level from disk...
            if(player[cplayer].bonus_level) {
               snprintf(levelname, TEXT_BUFFER, "bonus%03d.dp2", player[cplayer].bonus_level);
               disk_load = false;
            }
            else {
               snprintf(levelname, TEXT_BUFFER, "level%03d.dp2", player[cplayer].clevel);
               disk_load = true;
            }

            printf("Loading %s...\n", levelname);
            if(player[cplayer].bonus_level) { // note: we don't check the validated flag on bonus levels
               if(!load_map(level, levelname, disk_load)) {  // bonus levels ONLY load from pak file
                  printf("%s(%d): Error loading %s.\n", __FILE__, __LINE__, levelname);
                  shut_down();
                  exit(1);
               }
            }
            // Try and load custom levels if they exist, and if loading them is enabled.
            else if(!setting.user_levels || !load_map(level, levelname, disk_load) || !level->validated) {
               // no level on disk or user_levels turned off, check PAK for default
               disk_load = false;
               if(!load_map(level, levelname, disk_load)) {
                  // no more levels, completed all levels, wrap.
                  player[cplayer].wrap = true;
                  /*** TODO: Increase game speed here ***/
                  player[cplayer].clevel = 1;

                  disk_load = true;
                  // attempt to load level001 for restart
                  if(!setting.user_levels || !load_map(level, "level001.dp2", disk_load) || !level->validated) {
                     // no level on disk or user_levels turned off, check PAK for default
                     disk_load = false;
                     if(!load_map(level, "level001.dp2", disk_load)) {
                        printf("%s(%d): Error loading level001.dp2.\n", __FILE__, __LINE__);
                        shut_down();
                        exit(1);
                     }
                  }
               }
            }
         }
         if(!player[cplayer].bonus_level) {
            init_pickups(level);
            init_tools(level);
            if(player[cplayer].extra_time) tool.use_time = (tool.wait >> setting.suicide);
            else tool.use_time = (TOOL_USE_TIME >> setting.suicide);
         }
         int redraw_timer = al_get_timer_count(setting.redraw_timer);
         pickup.cntr = tool.cntr = redraw_timer;
         // set Pacman and ghost initial position according to level data
         // set positions for pacman to CENTRE of tile, this is important for rotations
         pacman.cur_img = 1;
         pacman.map = level->player;
         pacman.x = pacman.ox = pacman.map.x * TILE_SIZE + TILE_SIZE / 2; // tile# * 32pixels wide + half tile width
         pacman.y = pacman.oy = pacman.map.y * TILE_SIZE + TILE_SIZE / 2; // tile# * 32pixels high + half tile height
         pacman.dx = 1;    // pacman starts out facing right
         pacman.dy = 0;    // not facing up or down
         pacman.rot = 0.0f;   // not rotated
         pacman.flip = 0;  // not flipped

         for(int i = 0; i < 4; i++) {
            // make sure the ghosts are no longer scared, frozen or "dead"
            ghost[i].scared = false;
            ghost[i].dead = false;
            ghost[i].frozen = false;
            // ghost x/y map location based on map level data
            ghost[i].map = ghost[i].spawn = level->ghost[i];
            // ghost x/y pixel location = map tile # multiplied by the size of the tiles (32)
            // ghosts aren't rotated so there is no need for a center co-ordinate like pacman has.
            ghost[i].x = ghost[i].ox = ghost[i].map.x * TILE_SIZE;
            ghost[i].y = ghost[i].oy = ghost[i].map.y * TILE_SIZE;
            // find a path to pacman and set the ghost eyes in the direction the ghost is moving
            ghost[i].eyes = dir(level, &ghost[i], &pacman.map, true); // false
            ghost[i].dx = 0;
            ghost[i].dy = 0;
            // set the direction to move based on the ghosts eyes
            // no sense checking the shortest path twice when we already did it for the eyes
            switch(ghost[i].eyes) {
               case 0:  // Down
                  ghost[i].dy = 1;
                  break;
               case 1:  // Left
                  ghost[i].dx = -1;
                  break;
               case 2:  // Right
                  ghost[i].dx = 1;
                  break;
               case 3:  // Up
                  ghost[i].dy = -1;
                  break;
            }
         }

         draw_map(cplayer);

         // Only slide this level in from the right if it is a new level
         // not if the player died.
         if((player[cplayer].oldlevel != player[cplayer].clevel) || player[cplayer].bonus_level || game_loaded) {
            int lmh = al_get_bitmap_height(levelmap);
            int lmw = al_get_bitmap_width(levelmap);

            for(int x = 736; x >= 32; x -= 16) {
               al_draw_scaled_bitmap(levelmap, 0, 0, lmw, lmh, x + 32, 32, 736 - x, 544, 0);
               al_flip_display();
               al_rest(.01);
            }
         }

         if(!player[cplayer].bonus_level || game_loaded) player[cplayer].oldlevel = player[cplayer].clevel;

         pacman.dead = false;
         game_loaded = false;

         // If updating the screens fails for any reason, exit cleanly.
         if(!update_screen(cplayer)) {
            shut_down(); // release memory before exiting
            exit(1);
         }

         // New, loads an image prior to starting a level, if one exists.
         if(!player[cplayer].bonus_level) {
            snprintf(levelname, TEXT_BUFFER, "level%03d.png", player[cplayer].clevel);
            if(!load_map_image(level, levelname, true)) {   // first try from the custom folder
               load_map_image(level, levelname, false);     // if not in custom, try in the pak
            }
         }
         // Update the screen again to show the level.
         if(!update_screen(cplayer)) {
            shut_down(); // release memory before exiting
            exit(1);
         }

         // Display level start message
         if(!player[cplayer].bonus_level) level_message(player[cplayer].clevel, cplayer + 1, false);
         else level_message(player[cplayer].bonus_level, cplayer + 1, true);

         al_flip_display();

         //Stop the timers while we wait for a keypress
         al_stop_timer(pacman.timer);
         al_stop_timer(ghost[0].timer);
         al_stop_timer(setting.redraw_timer);

         // wait for keypress and if it returns true (ESC pressed), exit game loop.
         bool break_loop = false;
         if(wait_for_keypress(0)) break_loop = true;

         //Restart timers
         al_start_timer(setting.redraw_timer);
         al_start_timer(ghost[0].timer);
         al_start_timer(pacman.timer);

         // We needed to restart the timers before breaking out of the loop
         if(break_loop) break;

         ALLEGRO_KEYBOARD_STATE keys;
         ALLEGRO_MOUSE_STATE mouse;

         // Check the direction key the player is pressing (if any) and alter accordingly
         // default direction is right, so this is ommitted.
         // If no key was pressed, this will skip all cases by default and continue on.
         switch(event.keyboard.keycode) {
            case ALLEGRO_KEY_LEFT:
            case ALLEGRO_KEY_PAD_4:
            case ALLEGRO_KEY_A:
               pacman.dx = -1;
               pacman.dy = 0;
               pacman.flip = ALLEGRO_FLIP_HORIZONTAL;
               update_screen(cplayer);
               break;
            case ALLEGRO_KEY_UP:
            case ALLEGRO_KEY_PAD_8:
            case ALLEGRO_KEY_W:
               pacman.dy = -1;
               pacman.dx = 0;
               pacman.rot = -1.57f;
               update_screen(cplayer);
               break;
            case ALLEGRO_KEY_DOWN:
            case ALLEGRO_KEY_PAD_2:
            case ALLEGRO_KEY_S:
               pacman.dy = 1;
               pacman.dx = 0;
               pacman.rot = 1.57f;
               update_screen(cplayer);
               break;
         }

         /// JOYSTICK
         // The person may not have pressed a key, if they pressed the joystick, get the direction
         // the default direction (right) is ommitted, no need to check it as those values are already set.
         if(event.joystick.axis == 0) {            // X axis
            if(event.joystick.pos == -1.0f) {      // Left
               pacman.dx = -1;
               pacman.dy = 0;
               pacman.flip = ALLEGRO_FLIP_HORIZONTAL;
               update_screen(cplayer);
            }
         }
         else if(event.joystick.axis == 1) {       // Y axis
            if(event.joystick.pos == -1.0f) {      // Up
               pacman.dy = -1;
               pacman.dx = 0;
               pacman.rot = -1.57f;
               update_screen(cplayer);
            }
            else if(event.joystick.pos == 1.0f) {  // Down
               pacman.dy = 1;
               pacman.dx = 0;
               pacman.rot = 1.57f;
               update_screen(cplayer);
            }
         }

         // Reset pressed_key array before main loop
         memset(pressed_key, 0, sizeof(pressed_key));

         start_time = al_get_time(); // Get the time game was started for FPS

         int direction = -1;
         backplay = 0; // restart the background sound
         pitch = 1.0f / (float)level->pills;

         bool just_teleported = false;

         done = false;

         while(!done && !pacman.dead) {
#ifdef HACK_PROTECTION
            /// *** HACK CHECK ***
            if(player[cplayer].lives > MAX_LIVES) {
               hack_detected = true;
#ifdef DEBUG
               printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
            }
            HACK_VARIABLE1 = NEW_LIFE_SCORE;
            HACK_VARIABLE2 = MAX_LIVES;
#endif // HACK_PROTECTION

            while(!al_is_event_queue_empty(event_queue)) {  // empty the event queue
               al_wait_for_event(event_queue, &event);
               switch(event.type) {
                  case ALLEGRO_EVENT_DISPLAY_CLOSE:
                     shut_down();
                     exit(0);
                     break;
                  /// JOYSTICK
                  case ALLEGRO_EVENT_JOYSTICK_AXIS:
                     switch(event.joystick.axis) {
                        case 0:     // X axis 0
                           if(event.joystick.pos <= -0.5f && direction != 2)     direction = 2; // Left
                           else if(event.joystick.pos >= 0.5f && direction != 3) direction = 3; // Right
                           #ifdef DEBUG
                           printf("Axis 0: Direction = %d\n", direction);
                           #endif // DEBUG
                           break;
                        case 1:     // Y axis 1
                           if(event.joystick.pos <= -0.5f && direction != 0)     direction = 0; // Up
                           else if(event.joystick.pos >= 0.5f && direction != 1) direction = 1; // Down
                           #ifdef DEBUG
                           printf("Axis 1: Direction = %d\n", direction);
                           #endif // DEBUG
                           break;
                     }
                     break;
                  case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
                     if(event.joystick.button == 0) {       // Button 0 = A (green, used for shooting gun or pressing ENTER)
                        pressed_key[ALLEGRO_KEY_SPACE] = true;
                     }
                     if(event.joystick.button == 1) {  // Button 1 = B (red, like pressing ESC)
                        if(setting.music_on || setting.sound_on) {
                           al_stop_samples();
                           backplay = 0;
                        }
                        // Temporarily stop timers while the menu is displayed
                        al_stop_timer(pacman.timer);
                        al_stop_timer(ghost[0].timer);
                        al_stop_timer(setting.redraw_timer);
                        if(game_menu(cplayer)) {
                           for(int i = 0; i < 4; i++) {
                              player[i].lives = 0;
                              player[i].dead = true;
                           }
                           al_flush_event_queue(event_queue);
                           done = true;
                           game_over = true;
                           pacman.dead = true;
                        }
                        al_start_timer(setting.redraw_timer);
                        al_start_timer(ghost[0].timer);
                        al_start_timer(pacman.timer);
                     }
                     if(event.joystick.button == 2) {  // Button 2 = X (blue, like pressing CTRL for energy)
                        if(player[cplayer].energy > 0) {    // Speed up if there's enough energy.
                           if(!pacman.fast) {
                              al_set_timer_speed(pacman.timer, 1.0 / (pacman.ts * 1.5));
                              pacman.fast = true;
                           }
                        }
                        else if(pacman.fast) { // No Energy, slow down.
                           al_set_timer_speed(pacman.timer, 1.0 / pacman.ts);
                           pacman.fast = false;
                        }
                     }
                     break;
                  case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
                     if(event.joystick.button == 0) {    // Button 0 = A (green, used for shooting gun or pressing ENTER)
                        pressed_key[ALLEGRO_KEY_SPACE] = false;
                     }
                     if(event.joystick.button == 2) {    // Button 2 = X (blue, like releasing CTRL)
                        if(pacman.fast) {
                           al_set_timer_speed(pacman.timer, 1.0 / pacman.ts);
                           pacman.fast = false;
                        }
                     }
                     break;
                  case ALLEGRO_EVENT_JOYSTICK_CONFIGURATION:
                     al_reconfigure_joysticks();
                     break;
                  case ALLEGRO_EVENT_KEY_DOWN: {
                        /*** CHEAT KEYS ***
                        F1      = Changes all ghosts to blue (scared)
                        F2      = Suicide Speed on
                        F3      = Suicide Speed off
                        F5      = Give maximum energy
                        F9      = God mode enabled
                        F10     = God mode disabled
                        F11     = Gives you all 5 lives
                        PageUP  = Ends current level
                        B       = Force the next pickup to be the Glue tool
                        F       = Force the next pickup to be the Freeze tool
                        G       = Force the next pickup to be the Gun tool
                        H       = Force the next pickup to be the Speed tool
                        J       = Force the next pickup to be the Jump tool
                        Y       = Force the next pickup to be the Dynamite
                        M       = Force the next pickup to be the Time tool
                        N       = Force the next pickup to be the Bonus
                        O       = Force the next pickup to be the Present tool (the letter Oh that is ;))
                        X       = Force the next pickup to be the Shields (temporary, not god mode)
                        U       = Force the next pickup to be the Autism
                        */
                        int k = event.keyboard.keycode;
                        switch(k) {
                           case ALLEGRO_KEY_ESCAPE:
                              if(setting.music_on || setting.sound_on) {
                                 al_stop_samples();
                                 backplay = 0;
                              }
                              // Temporarily stop timers while the menu is displayed
                              al_stop_timer(pacman.timer);
                              al_stop_timer(ghost[0].timer);
                              al_stop_timer(setting.redraw_timer);
                              if(game_menu(cplayer)) {
                                 for(int i = 0; i < 4; i++) {
                                    player[i].lives = 0;
                                    player[i].dead = true;
                                 }
                                 al_flush_event_queue(event_queue);
                                 done = true;
                                 game_over = true;
                                 pacman.dead = true;
                              }
                              al_start_timer(setting.redraw_timer);
                              al_start_timer(ghost[0].timer);
                              al_start_timer(pacman.timer);
                              break;
                           case ALLEGRO_KEY_UP:
                           case ALLEGRO_KEY_PAD_8:
                           case ALLEGRO_KEY_W:
                              direction = 0;
                              break;
                           case ALLEGRO_KEY_DOWN:
                           case ALLEGRO_KEY_PAD_2:
                           case ALLEGRO_KEY_S:
                              direction = 1;
                              break;
                           case ALLEGRO_KEY_LEFT:
                           case ALLEGRO_KEY_PAD_4:
                           case ALLEGRO_KEY_A:
                              direction = 2;
                              break;
                           case ALLEGRO_KEY_RIGHT:
                           case ALLEGRO_KEY_PAD_6:
                           case ALLEGRO_KEY_D:
                              direction = 3;
                              break;
                           case ALLEGRO_KEY_LCTRL:
                           case ALLEGRO_KEY_RCTRL:
                              if(player[cplayer].energy > 0) { // Speed up if there's enough energy.
                                 if(!pacman.fast) {
                                    al_set_timer_speed(pacman.timer, 1.0 / (pacman.ts * 1.5));
                                    pacman.fast = true;
                                 }
                              }
                              else if(pacman.fast) { // No Energy, slow down.
                                 al_set_timer_speed(pacman.timer, 1.0 / pacman.ts);
                                 pacman.fast = false;
                              }
                              break;
                           case ALLEGRO_KEY_SPACE:
                              // Used for shooting gun, but no code needed here.
                              // pressed_key[] is set true for this which is all that is needed at this time.
                              break;
                           case ALLEGRO_KEY_F1:
                              if(setting.cheat_mode && !hack_detected) {
                                 for(int i = 0; i < 4; i++) {
                                    if(!ghost[i].dead) {
                                       ghost[i].scared = true;
                                       ghost[i].stimer = 0;
                                       ghostscore = 500; // Otherwise this could get way too high!
                                    }
                                 }
                              }
                              break;
                           case ALLEGRO_KEY_F2:    /// CHEAT: SUICIDE SPEED ON
                              if(setting.cheat_mode && !setting.suicide) {
                                 setting.suicide = 1;
                                 pacman.ts = TIMER_SPEED * 2;
                                 if(pacman.fast) al_set_timer_speed(pacman.timer, 1.0 / (pacman.ts * 1.5));
                                 else al_set_timer_speed(pacman.timer, 1.0 / pacman.ts);
                                 ghost[0].ts = TIMER_SPEED * 2;
                                 al_set_timer_speed(ghost[0].timer, 1.0 / ghost[0].ts);
                                 tool.use_time = TOOL_USE_TIME / 2;
                                 for(int i = 0; i < 4; i++) ghost[i].stime = (int)(REDRAW_TIMER * 0.75f) >> setting.suicide;
                              }
                              break;
                           case ALLEGRO_KEY_F3:    /// CHEAT: SUICIDE SPEED OFF
                              if(setting.cheat_mode && setting.suicide) {
                                 setting.suicide = 0;
                                 pacman.ts = TIMER_SPEED;
                                 if(pacman.fast) al_set_timer_speed(pacman.timer, 1.0 / (pacman.ts * 1.5));
                                 else al_set_timer_speed(pacman.timer, 1.0 / pacman.ts);
                                 ghost[0].ts = TIMER_SPEED;
                                 al_set_timer_speed(ghost[0].timer, 1.0 / ghost[0].ts);
                                 tool.use_time = TOOL_USE_TIME;
                                 for(int i = 0; i < 4; i++) ghost[i].stime = (int)(REDRAW_TIMER * 0.75f) >> setting.suicide;
                              }
                              break;
                           case ALLEGRO_KEY_F5:    /// CHEAT: MAXIMUM ENERGY
                              if(setting.cheat_mode) {
                                 if(!hack_detected) player[cplayer].energy = MAX_ENERGY;
                                 else player[cplayer].energy = 0;
                              }
                              break;
                           case ALLEGRO_KEY_F9:    /// CHEAT: GOD MODE ON
                              if(setting.cheat_mode) player[cplayer].god_mode = true;
                              break;
                           case ALLEGRO_KEY_F10:   /// CHEAT: GOD MODE OFF
                              if(setting.cheat_mode) player[cplayer].god_mode = false;
                              break;
                           case ALLEGRO_KEY_F11:   /// CHEAT: MAXIMUM PLAYER LIVES
                              if(setting.cheat_mode) {
                                 if(!hack_detected) player[cplayer].lives = MAX_LIVES;
                                 else player[cplayer].lives = 0;
                              }
                              break;
                           case ALLEGRO_KEY_F12: // SCREENSHOT
                              if(setting.sound_on)
                                 al_play_sample(sfx_camera, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                              if(!hack_detected) a5_screenshot("Screenshots/DeluxePacman2");
                              break;
                           case ALLEGRO_KEY_PGUP:
                              if(setting.cheat_mode) {
                                 level->pills = 0;
                                 skip_level = true;
                                 done = true;
                              }
                              break;
                           case ALLEGRO_KEY_B:
                              if(setting.cheat_mode) tool.force = GLUE;
                              break;
                           case ALLEGRO_KEY_F:
                              if(setting.cheat_mode) tool.force = FREEZE;
                              break;
                           case ALLEGRO_KEY_G:
                              if(setting.cheat_mode) tool.force = GUN;
                              break;
                           case ALLEGRO_KEY_H:
                              if(setting.cheat_mode) tool.force = SPEED;
                              break;
                           case ALLEGRO_KEY_J:
                              if(setting.cheat_mode) tool.force = JUMP;
                              break;
                           case ALLEGRO_KEY_Y:
                              if(setting.cheat_mode) tool.force = DYNAMITE;
                              break;
                           case ALLEGRO_KEY_M:
                              if(setting.cheat_mode) tool.force = TIME;
                              break;
                           case ALLEGRO_KEY_N:
                              if(setting.cheat_mode) tool.force = BONUS;
                              break;
                           case ALLEGRO_KEY_O: // The letter O, not number 0 ;)
                              if(setting.cheat_mode) tool.force = PRESENT;
                              break;
                           case ALLEGRO_KEY_X:
                              if(setting.cheat_mode) tool.force = SHIELDS;
                              break;
                           case ALLEGRO_KEY_U:
                              if(setting.cheat_mode) tool.force = AUTISM;
                              break;
                        }
                        if(hack_detected && tool.force) tool.force = SKULL;
                        pressed_key[k] = true;
                        break;
                     }
                  case ALLEGRO_EVENT_KEY_UP: {
                        int k = event.keyboard.keycode;
                        switch(k) {
                           case ALLEGRO_KEY_LCTRL:
                           case ALLEGRO_KEY_RCTRL:
                              if(pacman.fast) {
                                 al_set_timer_speed(pacman.timer, 1.0 / pacman.ts);
                                 pacman.fast = false;
                              }
                              break;
                        }
                        pressed_key[k] = false;
                        break;
                     }
                  case ALLEGRO_EVENT_TIMER:
                     // PACMAN TIMER
                     if(event.timer.source == pacman.timer) {
                        //redraw = true;
                        redraw_timer = al_get_timer_count(setting.redraw_timer);
                        if(pacman.fast && player[cplayer].energy > 0) {
#ifdef HACK_PROTECTION
                           player[cplayer].oldenergy = player[cplayer].energy; /// *** HACK CHECK ***
#endif // HACK_PROTECTION
                           if(pacman.ox != pacman.x || pacman.oy != pacman.y) {
                              add_energy(-MOVE_PIXELS, cplayer);
#ifdef HACK_PROTECTION
                              if(player[cplayer].oldenergy == player[cplayer].energy) {
                                 hack_detected = true; /// *** HACK CHECK ***
#ifdef DEBUG
                                 printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
                              }
#endif // HACK_PROTECTION
                           }
                        }
                        else if(pacman.fast) {
                           al_set_timer_speed(pacman.timer, 1.0 / pacman.ts);
                           pacman.fast = false;
                        }

                        switch(direction) {
                           case 0: // UP
                              if((pacman.dy == 1) || (pacman.x % 32 == 16 && pacman.map.x > 0 && pacman.map.x < 22)) {
                                 if(level->map[pacman.map.y - 1][pacman.map.x].is_pill ||
                                       !level->map[pacman.map.y - 1][pacman.map.x].tile) {
                                    pacman.dy = -1;
                                    pacman.dx = 0;
                                    pacman.rot = -1.57f;
                                    pacman.flip = 0;
                                 }
                              }
                              break;
                           case 1: // DOWN
                              if((pacman.dy == -1) || (pacman.x % 32 == 16 && pacman.map.x > 0 && pacman.map.x < 22)) {
                                 if(level->map[pacman.map.y + 1][pacman.map.x].is_pill ||
                                       !level->map[pacman.map.y + 1][pacman.map.x].tile) {
                                    pacman.dy = 1;
                                    pacman.dx = 0;
                                    pacman.rot = 1.57f;
                                    pacman.flip = 0;
                                 }
                              }
                              break;
                           case 2: // LEFT
                              if((pacman.dx == 1) || (pacman.y % 32 == 16 && pacman.map.y > 0 && pacman.map.y < 16)) {
                                 if(level->map[pacman.map.y][pacman.map.x - 1].is_pill ||
                                       !level->map[pacman.map.y][pacman.map.x - 1].tile) {
                                    pacman.dx = -1;
                                    pacman.dy = 0;
                                    pacman.rot = 0.0f;
                                    pacman.flip = ALLEGRO_FLIP_HORIZONTAL;
                                 }
                              }
                              break;
                           case 3: // RIGHT
                              if((pacman.dx == -1) || (pacman.y % 32 == 16 && pacman.map.y > 0 && pacman.map.y < 16)) {
                                 if(level->map[pacman.map.y][pacman.map.x + 1].is_pill ||
                                       !level->map[pacman.map.y][pacman.map.x + 1].tile) {
                                    pacman.dx = 1;
                                    pacman.dy = 0;
                                    pacman.rot = 0.0f;
                                    pacman.flip = 0;
                                 }
                              }
                              break;
                        }

                        // tiles are from X:0 to 22 and Y:0 to 16
                        bool movex = true;
                        bool movey = true;
                        int tx = pacman.map.x + pacman.dx;
                        int ty = pacman.map.y + pacman.dy;
                        if(tx >= 0 && tx <= 22 && ty >= 0 && ty <= 16) {
                           if(level->map[ty][tx].tile && !level->map[ty][tx].is_pill) {
                              if(pacman.dx && pacman.x % 32 == 16) movex = false;
                              if(pacman.dy && pacman.y % 32 == 16) movey = false;
                           }
                        }

                        // Map Position on screen: 32, 32 - 736, 544
                        pacman.ox = pacman.x;
                        pacman.oy = pacman.y;
                        if(movex) pacman.x += pacman.dx * MOVE_PIXELS;
                        if(movey) pacman.y += pacman.dy * MOVE_PIXELS;
                        pacman.map.x = pacman.x / 32;
                        pacman.map.y = pacman.y / 32;
                        if(pacman.x > 760) pacman.x = -24;        // 759 & -24 = ideal
                        else if(pacman.x < -24) pacman.x = 760;

                        if(pacman.y > 568) pacman.y = -24;
                        else if(pacman.y < -24) pacman.y = 568;

                        setting.pan = (float)pacman.x / PAN_RATIO;

                        if(pressed_key[ALLEGRO_KEY_SPACE]) activate_bullet();
                        if(bullet.state == ACTIVE) move_bullet(cplayer);

                        tx = pacman.map.x;
                        ty = pacman.map.y;
                        if(tx >= 0 && tx < MAPX && ty >= 0 && ty < MAPY) {
                           if(level->map[ty][tx].tile && level->map[ty][tx].is_pill) {
                              level->map[ty][tx].tile = 0;
                              level->map[ty][tx].is_pill = false;
                              if(level->map[ty][tx].is_powerpill) {
                                 if(setting.sound_on)
                                    al_play_sample(sfx_ppill, setting.sound_volume, setting.pan, 1,
                                                   ALLEGRO_PLAYMODE_ONCE, NULL);
                                 // 1000 points, not a pill, is a ppill, no float.
                                 add_points(1000, false, true, false, cplayer); /// function has hack check in it
                                 add_energy(50, cplayer);
                                 ghostscore = 500; // reset the score for eating a ghost to 500
                                 level->map[ty][tx].is_powerpill = false;
                                 for(int i = 0; i < 4; i++) {
                                    if(!ghost[i].dead) {
                                       ghost[i].scared = true;
                                       ghost[i].stimer = 0;
                                       if(hack_detected) ghost[i].stime = ((int)(REDRAW_TIMER * 0.75f) >> setting.suicide) * 0.65f;
                                    }
                                 }
                              }
                              else {
                                 // 100 points, is a pill, not a ppill, no float.
                                 add_points(100, true, false, false, cplayer);
                                 add_energy(10, cplayer);
                              }
                              if(player[cplayer].energy > MAX_ENERGY) player[cplayer].energy = MAX_ENERGY;
                              if(level->pills == 0) done = true;
                           }
                           // Check to see if player is on pickup/tool tile
                           else if(level->pickup.x == tx && level->pickup.y == ty) {
                              if(pickup.active) { // is a pickup currently active?
                                 add_points(player[cplayer].pvalue, false, false, true, cplayer);
                                 add_energy(100, cplayer);
                                 if(player[cplayer].energy > MAX_ENERGY) player[cplayer].energy = MAX_ENERGY;
                                 pickup.active = INACTIVE;
                                 pickup.cntr = redraw_timer;
                                 tool.cntr = redraw_timer;
                                 /// TODO: Create new sound effect for pickup/tool?
                                 if(setting.sound_on)
                                    al_play_sample(sfx_eat_pickup, setting.sound_volume, setting.pan, 1,
                                                   ALLEGRO_PLAYMODE_ONCE, NULL);
                              }
                              else if(tool.active) { // is a tool currently active?
                                 memset(tool_inuse, 0, sizeof(tool_inuse));
                                 bool toolsfx = true;
                                 do {
                                    tool.active = INACTIVE;
                                    tool.cntr = redraw_timer;
                                    pickup.cntr = redraw_timer;
                                    tool_inuse[RANDOM] = false;
                                    switch(tool.current) {
                                       case RANDOM:   // Chooses a random tool
                                          tool_inuse[RANDOM] = true;
                                          while(tool.current == RANDOM) tool.current = rand() % MAX_TOOLS + 1;
                                          if(hack_detected) tool.current = SKULL; /// *** HACK DETECTED *** skull! :D
                                          break;
                                       case SHIELDS:  // Gives you a holy shield to protect you
                                          tool_inuse[SHIELDS] = true;
                                          if(setting.sound_on) // play heavenly choir sound :)
                                             al_play_sample(sfx_heaven, setting.sound_volume, 0, 1,
                                                            ALLEGRO_PLAYMODE_ONCE, NULL);
                                          toolsfx = false;
                                          break;
                                       case PRESENT:  // Gives random points for each pill
                                          tool_inuse[PRESENT] = true;
                                          break;
                                       case BLUE_DIAMOND:   // Gives 1000 points per pill
                                          tool_inuse[BLUE_DIAMOND] = true;
                                          break;
                                       case PINK_DIAMOND:   // Gives 2000 points per pill
                                          tool_inuse[PINK_DIAMOND] = true;
                                          break;
                                       case BONUS:
                                          // remember to set the bonus_level to random bonus level.
                                          // if player dies on the bonus level they do not lose a life.
                                          // when player completes the bonus level they continue on to the
                                          // next normal level.
                                          // If player clears this level they get bonus points and a free life.
                                          jump = (rand() % bonuscount) + 1;
                                          player[cplayer].bonus_level = jump;
                                          add_points(level->pills * 100, false, false, false, cplayer);
                                          if(setting.sound_on)
                                             al_play_sample(sfx_end_of_level2, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                                          toolsfx = false;
                                          done = true;
                                          break;
                                       case TIMES2:   // Multiplies points by 2 for everything
                                          tool_inuse[TIMES2] = true;
                                          break;
                                       case TIMES5:   // Multiplies points by 5 for everything
                                          tool_inuse[TIMES5] = true;
                                          break;
                                       case TIMES7:   // Multiplies points by 7 for everything
                                          tool_inuse[TIMES7] = true;
                                          break;
                                       case FREEZE:   // Freezes ghosts in an ice block
                                          tool_inuse[FREEZE] = true;
                                          if(setting.sound_on)
                                             al_play_sample(sfx_freeze, setting.sound_volume, 0, 1,
                                                            ALLEGRO_PLAYMODE_ONCE, NULL);
                                          toolsfx = false;
                                          // Freeze all ghosts that aren't "dead"... not sure how a ghost dies but... ;)
                                          int frozen_ghosts = 0;
                                          for(int i = 0; i < 4; i++) {
                                             if(!ghost[i].dead) {
                                                ghost[i].frozen = true;
                                                ghost[i].eyes = 4;
                                                frozen_ghosts++;
                                             }
                                          }
                                          if(frozen_ghosts == 0) tool_inuse[FREEZE] = false;
                                          break;
                                       case AUTISM:   // Gives 30000 points
                                          add_points(30000, false, false, true, cplayer);
                                          break;
                                       case PRECIOUS: // Gives 50000 points
                                          add_points(50000, false, false, true, cplayer);
                                          if(setting.sound_on)
                                             al_play_sample(sfx_precious, setting.sound_volume, 0, 1,
                                                            ALLEGRO_PLAYMODE_ONCE, NULL);
                                          toolsfx = false;
                                          break;
                                       case GOLDLEAF: // Gives 40000 points
                                          add_points(40000, false, false, true, cplayer);
                                          if(setting.sound_on)
                                             al_play_sample(sfx_anthem, setting.sound_volume, 0, 1,
                                                            ALLEGRO_PLAYMODE_ONCE, NULL);
                                          toolsfx = false;
                                          break;
                                       case DYNAMITE: // Blows up Ghosts real good! ;)
                                          if(setting.sound_on)
                                             al_play_sample(sfx_boom, setting.sound_volume, 0, 1,
                                                            ALLEGRO_PLAYMODE_ONCE, NULL);
                                          toolsfx = false;
                                          for(int i = 0; i < 4; i++) {
                                             if(!ghost[i].dead) {
                                                ghost[i].dead = true;
                                                ghost[i].scared = false;
                                                add_points(5000, false, false, false, cplayer);
                                                float_points(5000, ghost[i].x, ghost[i].y);
                                             }
                                          }
                                          break;
                                       case JUMP:  // Jumps to a random level
                                          tool_inuse[JUMP] = true;

                                          jump = player[cplayer].clevel;

                                          while(jump == player[cplayer].clevel) {
                                             jump = (rand() % levelcount) + 1;
                                          }

                                          player[cplayer].clevel = jump;

                                          if(player[cplayer].wrap) player[cplayer].pvalue = 20000;
                                          else player[cplayer].pvalue = 1000 * jump;

                                          add_points(level->pills * 100, false, false, false, cplayer);

                                          if(player[cplayer].pvalue > 20000) player[cplayer].pvalue = 20000;

                                          level->pills = 0;
                                          skip_level = true;
                                          done = true;
                                          break;
                                       case EXTRA_E:  // Gives the letter E (spell EXTRA for bonus life)
                                          player[cplayer].extra |= 1;
                                          break;
                                       case EXTRA_X:  // Gives the letter X (spell EXTRA for bonus life)
                                          player[cplayer].extra |= 2;
                                          break;
                                       case EXTRA_T:  // Gives the letter T (spell EXTRA for bonus life)
                                          player[cplayer].extra |= 4;
                                          break;
                                       case EXTRA_R:  // Gives the letter R (spell EXTRA for bonus life)
                                          player[cplayer].extra |= 8;
                                          break;
                                       case EXTRA_A:  // Gives the letter A (spell EXTRA for bonus life)
                                          player[cplayer].extra |= 16;
                                          break;
                                       case GUN:
                                          tool_inuse[GUN] = true;
                                          break;
                                       case GLUE:
                                          tool_inuse[GLUE] = true;
                                          ghost[0].ts *= .5;
                                          al_set_timer_speed(ghost[0].timer, 1.0 / ghost[0].ts);
                                          break;
                                       case SPEED:
                                          tool_inuse[SPEED] = true;
                                          pacman.ts *= 1.5;
                                          al_set_timer_speed(pacman.timer, 1.0 / pacman.ts);
                                          break;
                                       case TIME:
                                          player[cplayer].extra_time = true;
                                          tool.use_time = (tool.wait >> setting.suicide);
                                          break;
                                       case SKULL:  // Kills you if you grab it, even with sheilds or cheat mode
                                          init_pacman(); // reset pacman settings
                                          memset(pressed_key, 0, sizeof(pressed_key)); // clear key array
                                          al_set_timer_speed(pacman.timer, 1.0 / pacman.ts); // reset timer
                                          pacman.dead = true;
                                          player_dies(cplayer);
                                          redraw = false;
                                          done = true;
                                          break;
                                    }
                                 }
                                 while(tool_inuse[RANDOM]);
                                 if(setting.sound_on && toolsfx)
                                    al_play_sample(sfx_eat_pickup, setting.sound_volume, setting.pan, 1,
                                                   ALLEGRO_PLAYMODE_ONCE, NULL);
                              }
                           }
                           else {
                              bool telecheck[2] = {}; // Use these to check if he is on either teleport
                              for(int i = 0; i < 2; i++) {
                                 if(level->teleport[i].x == tx && level->teleport[i].y == ty) {
                                    // Don't teleport if we just came from a teleport.
                                    if(!just_teleported) {
                                       pacman.map.x = level->teleport[!i].x;
                                       pacman.map.y = level->teleport[!i].y;
                                       // Pacman's position = tile * tile_size + 1/2 the tile_size (center)
                                       pacman.x = pacman.ox = pacman.map.x * TILE_SIZE + TILE_SIZE / 2;
                                       pacman.y = pacman.oy = pacman.map.y * TILE_SIZE + TILE_SIZE / 2;
                                       al_play_sample(sfx_teleport, setting.sound_volume, setting.pan, 1,
                                                      ALLEGRO_PLAYMODE_ONCE, NULL);
                                       setting.pan = (float)pacman.x / PAN_RATIO;
                                       just_teleported = true;
                                    }
                                    telecheck[i] = true;
                                 }
                              }
                              // We don't set the just_teleported flag to false until he is off both teleports.
                              if(!telecheck[0] && !telecheck[1]) just_teleported = false;
                           }
                        }

#ifdef HACK_PROTECTION
                        bool lives_reduced = false;   /// *** HACK CHECK ***
                        player[cplayer].oldlives = player[cplayer].lives;
#endif // HACK_PROTECTION

                        // update the score
                        if(player[cplayer].oldscore != 0 && player[cplayer].score != 0) {
                           // Player gets free man every 200000, or whatever is defined in NEW_LIFE_SCORE
                           if((int)(player[cplayer].oldscore / NEW_LIFE_SCORE) != (int)(player[cplayer].score / NEW_LIFE_SCORE)) {
                              if(player[cplayer].lives < MAX_LIVES) {
                                 player[cplayer].lives++;
#ifdef HACK_PROTECTION
                                 lives_reduced = true;
#endif // HACK_PROTECTION
                                 if(setting.sound_on)
                                    al_play_sample(sfx_new_life, setting.sound_volume, 0, 1,
                                                   ALLEGRO_PLAYMODE_ONCE, NULL);
                              }
                           }
                        }

#ifdef HACK_PROTECTION
                        if(lives_reduced && player[cplayer].oldlives == player[cplayer].lives) {
                           hack_detected = true; /// *** HACK CHECK ***
#ifdef DEBUG
                           printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
                        }
#endif // HACK_PROTECTION

                        if(player[cplayer].score > hiscore) hiscore = player[cplayer].score;

                        player[cplayer].oldscore = player[cplayer].score;

                        // check to see if player got all EXTRA letters
                        if(((player[cplayer].extra & 255) == 31) && (player[cplayer].lives < MAX_LIVES)) {
                           player[cplayer].lives++;
                           player[cplayer].extra = 0;

                           if(setting.sound_on)
                              al_play_sample(sfx_new_life, setting.sound_volume, 0, 1,
                                             ALLEGRO_PLAYMODE_ONCE, NULL);
                        }


                        if(pacman.ox == pacman.x && pacman.oy == pacman.y) pacman.cntr = redraw_timer;

                        if(redraw_timer - pacman.cntr >= pacman.img_delay) {
                           pacman.cur_img += pacman.inc;
                           pacman.cntr = redraw_timer;
                        }
                        if(pacman.cur_img == 3) pacman.inc = -1;   // mouth close
                        else if(pacman.cur_img == 0) pacman.inc = 1;   // mouth open
                     }
                     // GHOST TIMER
                     if(event.timer.source == ghost[0].timer) {
                        //redraw = true;
                        redraw_timer = al_get_timer_count(setting.redraw_timer);
                        scared_ghost = false; // used for switching background scared sound

                        for(int i = 0; i < 4; i++) {
                           // 0 = Red
                           // 1 = Green
                           // 2 = Cyan
                           // 3 = Purple
                           ghost[i].ox = ghost[i].x;
                           ghost[i].oy = ghost[i].y;

                           // do ghost movement etc here
                           if(ghost[i].x % TILE_SIZE == 0 && ghost[i].y % TILE_SIZE == 0
                                 && ghost[i].map.x >= 0 && ghost[i].map.x < MAPX
                                 && ghost[i].map.y >= 0 && ghost[i].map.y < MAPY) {

                              int sd = setting.difficulty;
                              if(hack_detected) sd = 2;


                              if(ghost[i].dead) {
                                 switch(sd) {
                                    case 0: // Easy (random)
                                       ghost[i].eyes = dir(level, &ghost[i], &ghost[i].spawn, true);
                                       break;
                                    case 1: // Medium (25% chance to go home)
                                       ghost[i].eyes = dir(level, &ghost[i], &ghost[i].spawn, rand() % 4);
                                       break;
                                    case 2: // AI - dependant on the ghost
                                       switch(i) {
                                          case 0:  // RED
                                             ghost[i].eyes = dir(level, &ghost[i], &ghost[i].spawn, false);
                                             break;
                                          case 1:  // GRN
                                             ghost[i].eyes = dir(level, &ghost[i], &ghost[i].spawn, rand() % 2);
                                             break;
                                          case 2:  // CYN
                                             ghost[i].eyes = dir(level, &ghost[i], &ghost[i].spawn, rand() % 2);
                                             break;
                                          case 3:  // PUR
                                             ghost[i].eyes = dir(level, &ghost[i], &ghost[i].spawn, rand() % 3);
                                             break;
                                       }
                                 }
                              }
                              else if(ghost[i].scared)
                                 ghost[i].eyes = dir(level, &ghost[i], &pacman.map, true);
                              else if(ghost[i].frozen) // eyes don't look around while frozen
                                 ghost[i].eyes = 4;
                              else {
                                 MAP target = pacman.map;

                                 switch(sd) {
                                    case 0: // Easy (random)
                                       ghost[i].eyes = dir(level, &ghost[i], &target, true);
                                       break;
                                    case 1: // Medium (25% chance to chase you)
                                       ghost[i].eyes = dir(level, &ghost[i], &target, rand() % 4);
                                       break;
                                    case 2: // AI - dependant on the ghost
                                       switch(i) {
                                          case 0:  // RED
                                             ghost[i].eyes = dir(level, &ghost[i], &target, false);
                                             break;
                                          case 1:  // GRN
                                             // set destination to 4 tiles ahead of pacman
                                             // dir() will take care of any boundry or invalid destination problems
                                             target.x += pacman.dx * 4;
                                             target.y += pacman.dy * 4;
                                             ghost[i].eyes = dir(level, &ghost[i], &target, false);
                                             break;
                                          case 2:  // CYN
                                             // selects a targer two tiles in front of pacman and a destination
                                             // equal in distance from that tile to the red ghost, but in the
                                             // opposite direction.
                                             target.x += pacman.dx * 2;
                                             target.y += pacman.dy * 2;
                                             target.x += (target.x - ghost[0].map.x);
                                             target.y += (target.y - ghost[0].map.y);
                                             ghost[i].eyes = dir(level, &ghost[i], &target, false);
                                             break;
                                          case 3:  // PUR
                                             // Chases Pacman like RED, only runs home when he gets too close
                                             if((abs(ghost[i].map.x - target.x) + abs(ghost[i].map.y - target.y)) < 8)
                                                target = ghost[i].spawn;
                                             ghost[i].eyes = dir(level, &ghost[i], &target, false);
                                             break;
                                       }
                                 }
                              }

                              ghost[i].dx = 0;
                              ghost[i].dy = 0;
                              // set the direction to move based on the ghosts eyes
                              // no sense checking the shortest path twice when we already did it for the eyes
                              switch(ghost[i].eyes) {
                                 case 0:
                                    ghost[i].dy = 1;
                                    break;
                                 case 1:
                                    ghost[i].dx = -1;
                                    break;
                                 case 2:
                                    ghost[i].dx = 1;
                                    break;
                                 case 3:
                                    ghost[i].dy = -1;
                                    break;
                              }
                           }

                           int tx = ghost[i].map.x + ghost[i].dx;
                           int ty = ghost[i].map.y + ghost[i].dy;
                           if(tx >= 0 && tx <= 22 && ty >= 0 && ty <= 16) {
                              if(level->map[ty][tx].tile && !level->map[ty][tx].is_pill) {
                                 if(ghost[i].dx && ghost[i].x % 32 == 0) ghost[i].dx = 0;
                                 if(ghost[i].dy && ghost[i].y % 32 == 0) ghost[i].dy = 0;
                              }
                           }

                           if(!ghost[i].frozen) {
                              ghost[i].x += ghost[i].dx * MOVE_PIXELS;
                              if(ghost[i].x > 744) ghost[i].x = -40;       // 744 & -40 = ideal
                              else if(ghost[i].x < -40) ghost[i].x = 744;
                              ghost[i].map.x = ghost[i].x / 32;

                              ghost[i].y += ghost[i].dy * MOVE_PIXELS;
                              if(ghost[i].y > 552) ghost[i].y = -40;
                              else if(ghost[i].y < -40) ghost[i].y = 552;  // -40 & 552 = ideal
                              ghost[i].map.y = ghost[i].y / 32;

                              if(ghost[i].ox == ghost[i].x && ghost[i].oy == ghost[i].y) ghost[i].cntr = redraw_timer;
                              if(redraw_timer - ghost[i].cntr >= ghost[i].img_delay) {
                                 ghost[i].cur_img += ghost[i].inc;
                                 ghost[i].cntr = redraw_timer;
                              }
                           }

                           if(ghost[i].scared == true) {
                              // increment the scared timer
                              if(redraw_timer - ghost[i].scntr >= 16) {
                                 ghost[i].stimer++;
                                 ghost[i].scntr = redraw_timer;
                              }
                              // If the timer runs out, reset the ghost to normal
                              if(ghost[i].stimer > ghost[i].stime) {
                                 ghost[i].scared = false;
                                 ghost[i].inc = 1;
                                 ghost[i].cur_img = 0;
                              }
                              else scared_ghost = true;
                           }

                           if(!ghost[i].frozen) {
                              // scared cycles back and forth
                              if(ghost[i].cur_img == 3 && ghost[i].scared) ghost[i].inc = -1;
                              // normal animation loops
                              else if(ghost[i].cur_img == 4) ghost[i].cur_img = 0;

                              // whether scared or not, if the current image is 0, we increment
                              if(ghost[i].cur_img == 0) ghost[i].inc = 1;
                           }

                           // if ghost is dead (just eyes) and over their own home, resserect them. ;)
                           if(ghost[i].dead && ghost[i].map.x == ghost[i].spawn.x && ghost[i].map.y == ghost[i].spawn.y)
                              ghost[i].dead = false;
                           /// TODO: Add in a 3 second delay before the ghost re appears, perhaps some sort of
                           /// animation, flickering or slowly fading in, no collision should be possible with
                           /// the ghost while it fades back in. (body fades in with eyes showing, maybe a glowing
                           /// circle under it and pulsating, humming style sound effect)


                           if(dp2_collision(&pacman, &ghost[i], hack_detected)) {
                              if(!ghost[i].scared) {
                                 if(!player[cplayer].god_mode || hack_detected) {
                                    if(player[cplayer].bonus_level) {
                                       memset(pressed_key, 0, sizeof(pressed_key)); // clear key array
                                       al_set_timer_speed(pacman.timer, 1.0 / pacman.ts); // reset timer
                                       skip_level = true; // skip level blinking animation
                                       player[cplayer].clevel = player[cplayer].oldlevel;
                                       player[cplayer].dead = true;
                                       done = true;
                                       redraw = false;
                                       break;
                                    }
                                    else if(!tool_inuse[SHIELDS] && !ghost[i].frozen) {
                                       init_pacman(); // reset pacman settings
                                       memset(pressed_key, 0, sizeof(pressed_key)); // clear key array
                                       al_set_timer_speed(pacman.timer, 1.0 / pacman.ts); // reset timer
                                       pacman.dead = true;
                                       player_dies(cplayer);
                                       player[cplayer].dead = true;
                                       redraw = false;
                                       break; // break out of the loop
                                    }
                                 }
                              }
                              else {
                                 if(setting.sound_on) {
                                    int rs = rand() % 3;
                                    switch(rs) {
                                       case 0:
                                          al_play_sample(sfx_ghost1, setting.sound_volume, setting.pan, 1,
                                                         ALLEGRO_PLAYMODE_ONCE, NULL);
                                          break;
                                       case 1:
                                          al_play_sample(sfx_ghost2, setting.sound_volume, setting.pan, 1,
                                                         ALLEGRO_PLAYMODE_ONCE, NULL);
                                          break;
                                       case 2:
                                          al_play_sample(sfx_ghost3, setting.sound_volume, setting.pan, 1,
                                                         ALLEGRO_PLAYMODE_ONCE, NULL);
                                    }
                                    ghost[i].dead = true;
                                 }
                                 ghost[i].scared = false;
                                 ghost[i].frozen = false;
                                 // ghostscore, not a pill, not a ppill, float.
                                 add_points(ghostscore, false, false, true, cplayer);
                                 ghostscore *= 2;
                                 ghost[i].eyes = dir(level, &ghost[i], &ghost[i].spawn, true); // rand() % 2
                                 redraw = true;
                              }
                           }
                        }

                        if(setting.sound_on) {
                           oldbackplay = backplay;
                           if(scared_ghost == false) {
                              if(backplay != 1) {
                                 if(backplay >= 2) al_stop_sample(&back2);
                                 al_play_sample(sfx_background1, setting.sound_volume, 0, 1,
                                                ALLEGRO_PLAYMODE_LOOP, &back1);
                                 backplay = 1;
                              }
                           }
                           else {
                              if(backplay <= 1) {
                                 if(backplay == 1) al_stop_sample(&back1);
                                 if(level->pills >= 150) backplay = 2;
                                 else backplay = 8 - (int)(level->pills / 25);
                                 al_play_sample(sfx_background2, setting.sound_volume, 0,
                                                2 - (float)(pitch * level->pills), ALLEGRO_PLAYMODE_LOOP, &back2);
                              }
                              else {
                                 if(level->pills >= 150) backplay = 2;
                                 else backplay = 8 - (int)(level->pills / 25);
                                 if(backplay != oldbackplay) {
                                    al_stop_sample(&back2);
                                    al_play_sample(sfx_background2, setting.sound_volume, 0,
                                                   2 - (float)(pitch * level->pills), ALLEGRO_PLAYMODE_LOOP, &back2);
                                 }
                              }
                           }
                        }
                     }

                     if(event.timer.source == setting.redraw_timer) redraw = true;
                     break;
               }
            }

            //if((redraw == true) && (al_is_event_queue_empty(event_queue) == true)) {
            if(redraw) {
               redraw = false;

               if(!update_screen(cplayer)) {
                  shut_down();
                  exit(1);
               }

               al_flip_display();

               frames++;
               fps = (double)frames / (al_get_time() - start_time);
            }
         }

         // Turn off any looping background sound effects playing
         if(backplay == 1) al_stop_sample(&back1);
         else if(backplay >= 2) al_stop_sample(&back2);
         if(sample_id_on) al_stop_sample(&sample_id);

         game_over = true;

         for(int i = 0; i < 4; i++) if(player[i].lives > 0) game_over = false;

         if(player[cplayer].bonus_level && player[cplayer].dead) {
            player[cplayer].dead = false;
            level->pills = 0;
         }
         else if(player[cplayer].bonus_level && level->pills == 0) {
            if(!hack_detected) player[cplayer].score += 50000;
            if(player[cplayer].lives < MAX_LIVES) {
               player[cplayer].lives++;
               if(setting.sound_on)
                  al_play_sample(sfx_new_life, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               if(hack_detected) player[cplayer].lives = 1;
            }
         }

         if(level->pills == 0) {
            if(!skip_level) {
               // Flash completed level on and off three times.
               ALLEGRO_BITMAP *tempBitmap = NULL;
               tempBitmap = al_create_bitmap(MAPX * 32, MAPY * 32);
               if(!tempBitmap) {
                  printf("%s(%d): Error creating tempBitmap.\n", __FILE__, __LINE__);
                  shut_down();
                  exit(1);
               }
               al_set_target_bitmap(tempBitmap);
               for(int y = 0; y < MAPY * 32; y += 32) {
                  for(int x = 0; x < MAPX * 32; x += 32) {
                     al_draw_bitmap(background[level->background], x, y, 0);
                  }
               }
               al_set_target_bitmap(al_get_backbuffer(setting.screen));
               for(int i = 0; i < 3; i++) {
                  al_draw_bitmap(levelmap, 32, 32, 0);
                  al_flip_display();
                  al_rest(.3);
                  if(setting.sound_on)
                     al_play_sample(sfx_end_of_level1, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  al_draw_bitmap(tempBitmap, 32, 32, 0);
                  al_flip_display();
                  al_rest(.3);
               }
               al_draw_bitmap(levelmap, 32, 32, 0);
               al_flip_display();
               al_rest(.3);
               al_destroy_bitmap(tempBitmap);
            }
            else {
               if(hack_detected) player[cplayer].clevel = 0;
            }
            skip_level = false;
            if(setting.sound_on)
               al_play_sample(sfx_end_of_level2, setting.sound_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);

            player[cplayer].clevel++;
            pickup.active = INACTIVE;
            tool.active = INACTIVE;

            al_flush_event_queue(event_queue);
            memset(pressed_key, 0, sizeof(pressed_key));

            pacman.ts = TIMER_SPEED + (TIMER_SPEED * setting.suicide);
            al_set_timer_speed(pacman.timer, 1.0 / pacman.ts);
            pacman.fast = false;

            ghost[0].ts = TIMER_SPEED + (TIMER_SPEED * setting.suicide);
            al_set_timer_speed(ghost[0].timer, 1.0 / ghost[0].ts);

            // Give player points (based on speed) for completing the level
            /// TODO: take into account the difficulty level.
            if(!hack_detected) player[cplayer].score += 10000;
            else player[cplayer].score++;

            // Increase the pickup value for the next level (to a max of 20000)
            if(player[cplayer].pvalue < 20000) {
               if(!hack_detected) player[cplayer].pvalue += 1000;
               else player[cplayer].pvalue++;
            }
            player[cplayer].bonus_level = 0;
         }
      }
      al_show_mouse_cursor(setting.screen);

      ALLEGRO_SAMPLE *music_hiscore = NULL;
      music_hiscore = al_load_sample("Sound/High_Score.ogg");
      if(!music_hiscore) {
         a5_error(AT, setting.screen, "Failed to load High_Score.ogg");
         shut_down();
         exit(1);
      }

      printf("-> Checking highscore table...\n");
      if(music_hiscore) al_play_sample(music_hiscore, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, NULL);
      for(int i = 0; i < players; i++) {
         printf("->    ...for player %d\n", i + 1);
         if(score_table(player[i].score, i + 1, player[i].clevel)) {
            shut_down();
            exit(0);
         }
      }
      if(setting.music_on || setting.sound_on) al_stop_samples();

      al_destroy_sample(music_hiscore);   music_hiscore = NULL;

      if(!exit_game && setting.music_on) {
         al_play_sample(music_main, setting.music_volume, 0, 1, ALLEGRO_PLAYMODE_LOOP, &music_id);
      }
   }

   shut_down();

   return 0;
}
