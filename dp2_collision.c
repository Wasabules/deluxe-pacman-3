#include "dp2_collision.h"

// Circular collision detection
bool dp2_collision(PACMAN *p, GHOST *g, bool h)
{
   // If the ghost is already dead, there is no collision
   if(g->dead) return false;

   double gx = (double)g->x + 16.0; // ghost.x is based on tile size (32), so half that is 16.
   double gy = (double)g->y + 16.0; // ghost.y is based on tile size (32), so half that is 16.
   double px = (double)p->x;
   double py = (double)p->y;
   double dist_x = px - gx;
   double dist_y = py - gy;
   double gr = g->r;
   if(h) {
      gr += 5.0;
#ifdef DEBUG
      printf("*** Hack Detected: %d ***\n", __LINE__);
#endif // DEBUG
   }
   double rt = abs(p->r + gr);

   if(abs(dist_x) > rt) return false;
   else if(abs(dist_y) > rt) return false;

    // The distance of the vector between the two
   double dist_squared = (dist_x * dist_x) + (dist_y * dist_y);
  
   // is the distance less than or equal to the absolute sum of the two radiuses?
   return dist_squared <= rt * rt;
}
