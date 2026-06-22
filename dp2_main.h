#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_physfs.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_opengl.h>
#include <physfs.h>
#include "resource.h"
#include "a5_error.h"
#include "a5_scale_screen.h"
#include "a5_screenshot.h"
#include "dp2_config.h"
#include "dp2_pacman.h"
#include "dp2_ghost.h"
#include "dp2_map.h"
#include "dp2_collision.h"
#include "dp2_gui.h"
#include "dp2_sound.h"
#include "dp2_hiscore.h"
#include "dp2_options.h"
#include "macros.h"

//#define HACK_PROTECTION

#define PAKSIZE            6592160
#define MAX_LEVELS         50
#define MAX_BONUS          4
#define WIDTH              800
#define HEIGHT             600
#define RESERVED_SAMPLES   16
#define SPRITE_SIZE        50
#define MAX_POINTS         32
#define MAX_TOOLS          25
#define MAX_PICKUPS        35
#define MAX_LIVES          5
#define MAX_ENERGY         500
#define NEW_LIFE_SCORE     200000
#define SHADOW             6
#define PAN_RATIO          736.0f-0.5f // I want the pan range to be less than the screen width or -.5 to +.5
#define TEXT_BUFFER        4096
#define REDRAW_TIMER       500
#define TOOL_USE_TIME      (REDRAW_TIMER * 12)
#define IMAGE_DELAY        (REDRAW_TIMER / 30)
#define TIMER_SPEED        60
#define MOVE_PIXELS        4
#define DP2_WEBSITE        "start https://nitehackr.github.io/games_index.html"
#define DIFFICULTY         3


// These MUST be in the same order as the tool sheet graphics (left to right, top to bottom).
typedef enum TOOLS { OFF, RANDOM, SHIELDS, PRESENT, BLUE_DIAMOND, PINK_DIAMOND, BONUS, TIMES2, TIMES5, TIMES7,
                     FREEZE, AUTISM, PRECIOUS, GOLDLEAF, DYNAMITE, JUMP, EXTRA_E, EXTRA_X, EXTRA_T, EXTRA_R,
                     EXTRA_A, GUN, GLUE, SPEED, TIME, SKULL } TOOLS;

// Used for Ghost path finding
typedef struct DIRECTION {
   int b;         // path blocked flag (true or false)
   int x, y;
} DIRECTION;

// BIG changes in this struct, all level data is now in the level member, including pills.
// There is no longer a need for a temporary map to hold the data when diamonds are enabled
// because now that will be handled by the drawing function which will simply draw diamonds
// instead of pills when diamonds are enabled. (don't know why I never done this before!)
typedef struct PLAYER {
   int lives;                   // Number of lives the player has left
   int oldlives;                // used to see if the player suddenly gains more than 1 life (hack check)
   unsigned long      score;    // Current score
   unsigned long   oldscore;    // used to see how much the players score has changed
   int clevel;                  // current level player is on
   int oldlevel;                // the last level the player came from
   unsigned long pvalue;        // the current value of pickups
   LEVEL level;                 // the current map data (temp map no longer needed, see journal)
   int extra;                   // What EXTRA letters does the player have, this uses bits to represent each letter.
   bool wrap;                   // has the player done all the levels?
   bool dead;                   // is player dead
   int oldenergy;               // old energy used for speed
   int energy;                  // energy used for speed
   bool god_mode;               // God mode on/off switch.
   int suicide;                 // Suicide mode on = 1, off = 0 (important to use 1 & 0 rather than a bool)
   bool extra_time;             // Does player have extra tool time (TIME tool)?
   int bonus_level;             // Is player on a bonus level? 0 = no, 1+ = level to load
} PLAYER;

typedef enum STATE { INACTIVE, ACTIVE } STATE;

typedef struct BULLET {
   ALLEGRO_BITMAP *bitmap;
   ALLEGRO_SAMPLE *sound;
   int x, y;             // bullet screen co-ordinates
   int dx, dy;            // direction bullet is traveling x+/-1 y+/-1
   int d;                // distance bullet has traveled.
   float rot;              // pacman rotation
   int flip;               // pacman flip
   STATE state;            // is bullet active or not?
} BULLET;

typedef struct PICKUP {
   ALLEGRO_BITMAP *sheet;                 // the sprite sheet containing all our pickup images.
   ALLEGRO_BITMAP *bitmap[MAX_PICKUPS];   // this will be an array of sub bitmaps taken from sprite sheet.
   int current;                           // current pickup number (0 to max)
   int screen_time;                       // max time pickup should stay on screen
   int x, y;                              // on screen co-ordinates
   MAP map;                               // map co-ordinates
   STATE active;                          // is pickup currently on screen?
   int64_t cntr;                          // delay counter
   int wait;                              // time between pickups (1/60th sec)
} PICKUP;

typedef struct TOOL {
   ALLEGRO_BITMAP *sheet;              // the sprite sheet containing all our pickup images.
   ALLEGRO_BITMAP *bitmap[MAX_TOOLS];  // this will be an array of sub bitmaps taken from sprite sheet.
   int current;                        // current tool number (0 to max)
   int screen_time;                    // max time tool should stay on screen
   int x, y;                           // on screen co-ordinates
   MAP map;                            // map co-ordinates
   STATE active;                       // is tool currently on screen?
   int64_t cntr;                       // delay counter
   int wait;                           // time between tools (1/60th sec)
   int use_time;                       // time player can use the tool
   TOOLS force;                        // used for cheat mode to force a tool active
} TOOL;

typedef struct FPOINT {
   int x, y;                 // screen co-ordinates points initially appear at
   float colour;             // colour of text (0.0 - 1.0, 1.0 being bright white)
   int delay;                // delay between movement of points
   int64_t counter;              // when counter = delay, move points up screen
   unsigned long value;      // value of points to be displayed
   int move;                 // how many pixels should we move the points
   int moved;                // how many pixels have the points moved
} FPOINT;

void shutdown_display(void);
void shut_down(void);
void init_display(bool reset_display);
bool key_pressed(int key);
bool wait_for_keypress(double wait_time);
void message(const char *text_message);

extern SETTING setting;
extern ALLEGRO_EVENT_QUEUE *event_queue;
extern ALLEGRO_EVENT event;
extern ALLEGRO_FONT *font_amaze;
extern ALLEGRO_FONT *font_verdana;
extern ALLEGRO_FONT *font_verdana_bmp;
extern ALLEGRO_FONT *font_radio_space_bold;
extern ALLEGRO_FONT *font_fatfont_grn;
extern ALLEGRO_FONT *font_fatfont_red;
extern ALLEGRO_FONT *font_digital;
extern ALLEGRO_SAMPLE *music_main;
extern ALLEGRO_SAMPLE_ID music_id;
extern ALLEGRO_SAMPLE *sfx_type, *sfx_camera;
