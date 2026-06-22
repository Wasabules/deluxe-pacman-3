// Scale Screen:
//    Sets up transforms in order to fit any resolution
//    onto an ALLEGRO_FULLSCREEN_WINDOW.
//
// (Special thanks to Matthew Leverton for help with this!)

#include "a5_scale_screen.h"



// Use to get mouse position on scaled screen
float scale_x = 1;
float scale_y = 1;
int offset_x = 0;
int offset_y = 0;



// usage: a5_scale_screen(BUFFER_WIDTH, BUFFER_HEIGHT, display_width, display_height);
void a5_scale_screen(int bw, int bh, int dw, int dh)
{
   ALLEGRO_TRANSFORM t;

   // Calculate the horizontal and vertial aspect ratios
   const float HAR = dw / (float)bw;
   const float VAR = dh / (float)bh;

   // The aspect ratio, x-offset and y-offset (in pixels)
   float ar, ox, oy;

   if(bw == dw && bh == dh) {
      // 1:1, just reset everything
      al_identity_transform(&t);
      al_use_transform(&t);
      al_set_clipping_rectangle(0, 0, bw, bh);
      scale_x = 1;
      scale_y = 1;
      offset_x = 0;
      offset_y = 0;
      al_clear_to_color(al_map_rgb(0, 0, 0));
      al_flip_display();
   }
   else {
      // Choose the smaller aspect ratio
      if(HAR < VAR) {
         // horizontal bars on the top and bottom
         ar = HAR;
         ox = 0;
         oy = (dh - (ar * bh)) / 2.0;
      }
      else {
         // vertical bars on the left and right
         ar = VAR;
         ox = (dw - (ar * bw)) / 2.0;
         oy = 0;
      }

      // set the global scale/offset so the mouse coords can be inverted
      // use the following code to get the proper mouse position
      // mouse_x = (event.mouse.x - offset_x) / scale_x;
      // mouse_y = (event.mouse.y - offset_y) / scale_y;
      scale_x = ar;
      scale_y = ar;
      offset_x = ox;
      offset_y = oy;

      // set up the transformation to scale and translate
      al_build_transform(&t, ox, oy, ar, ar, 0);
      al_use_transform(&t);

      // clear out the screen before setting the clipping
      al_set_clipping_rectangle(0, 0, dw, dh);
      al_clear_to_color(al_map_rgb(0, 0, 0));
      al_flip_display();

      // make sure nothing is drawn into the black bars
      al_set_clipping_rectangle(ox, oy, ar * bw, ar * bh);
   }
}
