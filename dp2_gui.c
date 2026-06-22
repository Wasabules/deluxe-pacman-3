#include "dp2_gui.h"


// new_button:
//    Creates a new button, loading in the file indicated.
//    The graphics provided MUST contain three images, lined up vertically.
//    The top image is the normal button that is displayed when it isn't being used.
//    The second image down is the button to display when the mouse is over it (but not clicked)
//    The third image is the button to display when the mouse is clicked on it.
//    This function assumes a new uninitialized BUTTON variable sent to it.
bool new_button(BUTTON *b, bool is_switch, const char *file, int x, int y)
{
   b->x = x;
   b->y = y;
   b->state = 0;
   b->old_state = 0;
   b->is_switch = is_switch;
   b->bmp = NULL;
   b->bmp = al_load_bitmap(file);
   if(!b->bmp) return false;
   b->w = al_get_bitmap_width(b->bmp);
   b->h = al_get_bitmap_height(b->bmp) / 3;
   b->up = NULL;
   b->up   = al_create_sub_bitmap(b->bmp, 0, 0, b->w, b->h);
   b->over = NULL;
   b->over = al_create_sub_bitmap(b->bmp, 0, b->h, b->w, b->h);
   b->down = NULL;
   b->down = al_create_sub_bitmap(b->bmp, 0, b->h * 2, b->w, b->h);

   return true;
}



// check_button:
//    Checks to see if the mouse is over or is clicking on a button
//    Returns true if an area was clicked in.
bool check_button(BUTTON *b, ALLEGRO_MOUSE_STATE *mouse)
{
   bool is_over = true;
   // Convert mouse position to proper position on scaled screen.
   int mouse_x = (mouse->x - offset_x) / scale_x;
   int mouse_y = (mouse->y - offset_y) / scale_y;

   // Calculate the right side.
   int ex = b->x + b->w - 1;

   // Calculate the bottom side.
   int ey = b->y + b->h - 1;

   // save the last state
   b->old_state = b->state;

   // set new state to up
   if(b->is_switch && b->state == 2) b->state = b->old_state;
   else b->state = 0;

   // If the mouse is outside of the area in question, return false.
   if(mouse_x < b->x || mouse_y < b->y || mouse_x > ex || mouse_y > ey) is_over = false;

   if(is_over) {
      if(!b->is_switch) b->state = 1;   // mouse is over
      else if(b->is_switch && b->state == 0) b->state = 1;
      if(mouse->buttons & 1) {
         if(b->is_switch && b->state == 2) b->state = 0;
         else b->state = 2;   // left button down?
      }
   }
   // if the state hasn't changed, return false
   if(b->state == b->old_state && !is_over) return false;

   // the state has changed, return true
   return true;
}


// draw_button:
//    Draws the button on screen using the indicated state to select which image.
void draw_button(BUTTON *b)
{
   switch(b->state) {
      case 0:  // Off
         al_draw_bitmap_region(b->up, 0, 0, b->w, b->h, b->x, b->y, 0);
         break;
      case 1:  // Mouse Over
         al_draw_bitmap_region(b->over, 0, 0, b->w, b->h, b->x, b->y, 0);
         break;
      case 2:  // Clicked On
         al_draw_bitmap_region(b->down, 0, 0, b->w, b->h, b->x, b->y, 0);
         break;
      default:
         printf("%s(%d): Error, Invalid state: %d", __FILE__, __LINE__, b->state);
   }
}


// destroy_button:
//    Destroys a BUTTON which has already been initialized and
//    frees up the memory associated with it.
void destroy_button(BUTTON *b)
{
   if(b->up) {
      al_destroy_bitmap(b->up);
      b->up = NULL;
   }
   if(b->over) {
      al_destroy_bitmap(b->over);
      b->over = NULL;
   }
   if(b->down) {
      al_destroy_bitmap(b->down);
      b->down = NULL;
   }
   if(b->bmp) {
      al_destroy_bitmap(b->bmp);
      b->bmp = NULL;
   }
}
