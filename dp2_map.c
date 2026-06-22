#include <stdio.h>
#include "dp2_map.h"
#include "vec2d_stack.h"

// Find the shortest path to mapx, mapy
void get_path(int path_node[MAPY][MAPX], LEVEL *level, MAP *dest)
{
//       Path finding is done thusly:
//       - We first fill in a grid that represents our map with -1s.
//       - Any map locations that have pieces that block movement are set to 999
//       - We then set our destination to zero.
//       - After this we use the floodfill algorithm, which adds +1 to all surrounding nodes of the current node
//         increasing in value until the map is filled, the ghosts can then simply move to the cell with the next
//         lowest value to get the shortest path.  This takes into account screen wrap so the ghosts can move around
//         map off the edges if that is shorter.

   STACK_TYPE open_stack;           // map locations that need to be checked
   STACK_TYPE check_stack;          // adjacent map locations that need to be checked
   STACK_ELEMENT cell;              // each stack element for the open stack
   int closed[MAPY][MAPX];          // a map array to show which cells are closed (already checked)
   int cn = 0;                      // current node value
   STACK_ELEMENT check;             // used for checking cells

   // fill empty map locations with -1
   // set map locations that have pieces that block movement to 999 and close them
   for(int y = 0; y < MAPY; y++) {
      for(int x = 0; x < MAPX; x++) {
         // set all path nodes to -1 except lines which block movement
         if(level->map[y][x].tile == 0 || level->map[y][x].is_pill) {
            path_node[y][x] = -1;
            closed[y][x] = 0;
         }
         else {
            path_node[y][x] = 999;
            closed[y][x] = 1;
         }
      }
   }

   // set destination to zero if it is not a line, otherwise it is an invalid destination.
   if(path_node[dest->y][dest->x] == -1) {
      path_node[dest->y][dest->x] = cn;
      closed[dest->y][dest->x] = 1;
   }
   else {
      path_node[dest->y][dest->x] = 1; // The destination (player location usually) is normally zero, so I set it to 1
      return;                          // and return as a simple way to inidcate this is an invalid location.
   }

   // Initialize our stacks
   if(!stack_init(&open_stack)) {
      printf("Error: Failed to initialize open_stack.\n");
      exit(1);
   }
   if(!stack_init(&check_stack)) {
      printf("Error: Failed to initialize check_stack.\n");
      exit(1);
   }

   // First we add the destination cell to the open stack so it is checked first
   stack_push(&open_stack, dest->x, dest->y);

   // Loop while there are open (unchecked) cells on the open_stack.
   do {
      stack_pop(&open_stack, &check.x, &check.y);  // Read the cell to check.

      // Add the adjacent cells that are not in closed to open (need to be checked), check for wrap as well.
      cn = path_node[check.y][check.x] + 1;              // new cell values for adjacent cells
      // push cells to be checked onto check stack
      stack_push(&check_stack, check.x, check.y - 1);    // North
      stack_push(&check_stack, check.x - 1, check.y);    // West
      stack_push(&check_stack, check.x + 1, check.y);    // East
      stack_push(&check_stack, check.x, check.y + 1);    // South

      do {                                            // Loop through the check stack
         stack_pop(&check_stack, &cell.x, &cell.y);   // Read the next adjacent cell to check.
         if(cell.x < 0) cell.x = MAPX - 1;            // check for left edge wrap
         if(cell.x >= MAPX) cell.x = 0;               // check for right edge wrap
         if(cell.y < 0) cell.y = MAPY - 1;            // check for top edge wrap
         if(cell.y >= MAPY) cell.y = 0;               // check for bottom edge wrap
         if(closed[cell.y][cell.x] == 0) {               // if this cell is not on the closed list
            if(path_node[cell.y][cell.x] == -1) {        // A cell still be -1 if it cannot be reached.
               stack_push(&open_stack, cell.x, cell.y);  // push it on the open stack to be checked later
               path_node[cell.y][cell.x] = cn;           // set the cell value to the current node (cn) value
            }
         }
         else {   // if this cell is on the closed list, check and see if it's current value is greater than the current one.
            if(path_node[cell.y][cell.x] > cn && path_node[cell.y][cell.x] < 999)   // ignore walls
               path_node[cell.y][cell.x] = cn;   // if it is, update this cell's value to the lower one.
            else if(path_node[cell.y][cell.x] + 1 < cn - 1) {
               cn = path_node[cell.y][cell.x] + 1; // If it is much lower than the current node, than update the current node
               path_node[check.y][check.x] = cn;   // as this will result in a shorter path
            }
         }
      }
      while(stack_size(&check_stack) > 0);         // keep looping until all surrounding cells are checked
      closed[check.y][check.x] = 1;                // we're done here, set this cell to closed.
   }
   while(stack_size(&open_stack) > 0);             // keep looping while there are cells need to be checked

   // When we get here, the entire level would be scanned and the shortest path from any location on the map to
   // the player will be set.  No matter where a ghost is, they only need to check the surrounding map cells
   // and the one with the lowest number (that's what the variable "cn" is above) is the path we need to take
   // to get to the player the fastest.  This takes into account wrapping around the level so if the shortest
   // path involves travelling off one edge and onto the other, it will take it.  This means there is no map design
   // you can make that the ghosts cannot navigate around.

   // Free up the memory we allocated for our stacks
   stack_destroy(&open_stack);
   stack_destroy(&check_stack);

   /* Test code that prints out the resuling node data
   for(int y = 0; y < MAPY; y++) {
      for(int x = 0; x < MAPX; x++) {
         printf("%03i ", path_node[y][x]);
      }
      printf("\n");
   }
   */
}


// helps ghosts find the shortest direction to turn to in order
// to reach the map tile location of dest->x/dest->y.
// if random_dir is true, we'll pick a random valid direction to move.
// returns 0 for down, 1 for left, 2 for right, 3 for up
int dir(LEVEL *level, GHOST *ghost, MAP *dest, bool random_dir)
{
   /// TODO: Store last destination and check if it has changed
   /// TODO: save path_node as a static array
   static int path_node[MAPY][MAPX];
   int t, start, v = 999;
   bool valid_dir[4] = {false, false, false, false};
   int invalid = 0; // the opposite direction the ghost is moving = invalid
   MAP g_start;
   static MAP old_dest;
   g_start.x = ghost->map.x;
   g_start.y = ghost->map.y;

   // There are MAPX tiles horizontally (or for x) and MAPY vertically (or for y)
   // make certain we fall within those bounds
   // and adjust to the closest valid value if we fall outside of it.
   if(dest->x < 0) dest->x = 0;
   else if(dest->x >= MAPX) dest->x = MAPX - 1;
   if(dest->y < 0) dest->y = 0;
   else if(dest->y >= MAPY) dest->y = MAPY - 1;

   if(g_start.x < 0) g_start.x = 0;
   else if(g_start.x >= MAPX) g_start.x = MAPX - 1;
   if(g_start.y < 0) g_start.y = 0;
   else if(g_start.y >= MAPY) g_start.y = MAPY - 1;


   //Check if there is only one direction to go here...
   // not only straight aways, but also if a ghost is in a corner
   // For each direction we can move, we'll increment the dir_count
   // if there is only one direction, there is no reason to waste time
   // calling the path function and doing anything else, just return
   // the valid path we can go and be done with it.
   int dir_count = 0;
   int valid;
   switch(ghost->dx) {
      case -1:
         invalid = 2;
         if(g_start.x == 0) { // ghost is on the edge of the map, so the wrap is guaranteed, no need to test
            dir_count++;      // The level editor guarantees all wraps, or it won't validate it.
            valid = 1;
         }
         else if(level->map[g_start.y][g_start.x - 1].tile == 0 || level->map[g_start.y][g_start.x - 1].is_pill) {
            dir_count++;
            valid = 1;
         }
         break;
      case  0:
         if(g_start.x == 0) { // ghost is on the edge of the map, so the wrap is guaranteed, no need to test
            dir_count++;      // The level editor guarantees all wraps, or it won't validate it.
            valid = 1;
         }
         else if(level->map[g_start.y][g_start.x - 1].tile == 0 || level->map[g_start.y][g_start.x - 1].is_pill) {
            dir_count++;
            valid = 1;
         }
         if(g_start.x == MAPX - 1) { // ghost is on the edge of the map, so the wrap is guaranteed, no need to test
            dir_count++;           // The level editor guarantees all wraps, or it won't validate it.
            valid = 2;
         }
         else if(level->map[g_start.y][g_start.x + 1].tile == 0 || level->map[g_start.y][g_start.x + 1].is_pill) {
            dir_count++;
            valid = 2;
         }
         break;
      case  1:
         invalid = 1;
         if(g_start.x == MAPX - 1) { // ghost is on the edge of the map, so the wrap is guaranteed, no need to test
            dir_count++;         // The level editor guarantees all wraps, or it won't validate it.
            valid = 2;
         }
         else if(level->map[g_start.y][g_start.x + 1].tile == 0 || level->map[g_start.y][g_start.x + 1].is_pill) {
            dir_count++;
            valid = 2;
         }
   }
   switch(ghost->dy) {
      case -1:
         invalid = 0;
         if(g_start.y == 0) { // ghost is on the edge of the map, so the wrap is guaranteed, no need to test
            dir_count++;      // The level editor guarantees all wraps, or it won't validate it.
            valid = 3;
         }
         else if(level->map[g_start.y - 1][g_start.x].tile == 0 || level->map[g_start.y - 1][g_start.x].is_pill) {
            dir_count++;
            valid = 3;
         }
         break;
      case  0:
         if(g_start.y == 0) { // ghost is on the edge of the map, so the wrap is guaranteed, no need to test
            dir_count++;      // The level editor guarantees all wraps, or it won't validate it.
            valid = 3;
         }
         else if(level->map[g_start.y - 1][g_start.x].tile == 0 || level->map[g_start.y - 1][g_start.x].is_pill) {
            dir_count++;
            valid = 3;
         }
         if(g_start.y == MAPY - 1) { // ghost is on the edge of the map, so the wrap is guaranteed, no need to test
            dir_count++;         // The level editor guarantees all wraps, or it won't validate it.
            valid = 0;
         }
         else if(level->map[g_start.y + 1][g_start.x].tile == 0 || level->map[g_start.y + 1][g_start.x].is_pill) {
            dir_count++;
            valid = 0;
         }
         break;
      case  1:
         invalid = 3;
         if(g_start.y == MAPY - 1) { // ghost is on the edge of the map, so the wrap is guaranteed, no need to test
            dir_count++;         // The level editor guarantees all wraps, or it won't validate it.
            valid = 0;
         }
         else if(level->map[g_start.y + 1][g_start.x].tile == 0 || level->map[g_start.y + 1][g_start.x].is_pill) {
            dir_count++;
            valid = 0;
         }
   }
   if(dir_count == 1) return valid; // Can only move one direction, so just return that direction.
   if(dir_count == 0) return invalid; // dead end, turn around.

   // We have more than one direction we can choose from, so lets get a path...
   if(dest->x != old_dest.x && dest->y != old_dest.y) { // only get a new path if the destination has changed
      get_path(path_node, level, dest);

      // Destination changed, so lets store it for the next time we're here.
      old_dest.x = dest->x;
      old_dest.y = dest->y;
   }

   if(path_node[dest->y][dest->x] == 1) random_dir = true;

   // check for the shortest path to pacman and start out ghost
   // facing that direction, this helps keep the ghost from starting
   // out facing a wall or other such invalid direction.

   // check down
   if(g_start.y < MAPY - 1) t = path_node[g_start.y + 1][g_start.x];
   else t = path_node[0][g_start.x];   // ghost is at bottom, check top (wrap)
   if(t >= 0 && t < 999) {
      valid_dir[0] = true; // this is a valid direction to move
      if(t < v) {
         v = t;
         start = 0;
      }
   }
   else if(t == -1) valid_dir[0] = true; // not a valid PATH, but a valid direction for random movement

   // check left
   if(g_start.x >= 1) t = path_node[g_start.y][g_start.x - 1];
   else t = path_node[g_start.y][MAPX - 1];   // ghost is at left, check right side
   if(t >= 0 && t < 999) {
      valid_dir[1] = true; // this is a valid direction to move
      if(t < v) { // but is it the shortest path?
         v = t;
         start = 1;
      }
   }
   else if(t == -1) valid_dir[1] = true; // not a valid PATH, but a valid direction for random movement

   // check right
   if(g_start.x < MAPX - 1) t = path_node[g_start.y][g_start.x + 1];
   else t = path_node[g_start.y][0];
   if(t >= 0 && t < 999) {
      valid_dir[2] = true; // this is a valid direction to move
      if(t < v) { // but is it the shortest path?
         v = t;
         start = 2;
      }
   }
   else if(t == -1) valid_dir[2] = true; // not a valid PATH, but a valid direction for random movement

   // check up
   if(g_start.y >= 1) t = path_node[g_start.y - 1][g_start.x];
   else t = path_node[MAPY - 1][g_start.x];
   if(t >= 0 && t < 999) {
      valid_dir[3] = true; // this is a valid direction to move
      if(t < v) { // but is it the shortest path?
         v = t;
         start = 3;
      }
   }
   else if(t == -1) valid_dir[3] = true; // not a valid PATH, but a valid direction for random movement

   // we'll check valid directions starting at a random point
   // this way we don't get stuck in a random loop that MIGHT
   // check all the directions, this way we check all directions
   // taking the same amount of time to do so each time we check
   // but starting at a random point.  No loops to get stuck in.
   if(random_dir) start = rand() % 4;

   for(int i = start; i < 4; i++) if(i != invalid && valid_dir[i]) return i;
   for(int i = 0; i < start; i++) if(i != invalid && valid_dir[i]) return i;

   // couldn't find a valid direction, must be at a dead end, we'll reverse direction
   return invalid;
}
