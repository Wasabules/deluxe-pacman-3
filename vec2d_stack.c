/*    Dynamic stack for C.
 *    by Neil Roy
 *    January 23, 2016
 *
 *    This will create a stack (based on the STACK_ELEMENT type) which starts at 1 element
 *    and dynamically resizes itself as needed.  When the current stack limit is reached
 *    it will double the size of it and then continue to add to it.
 *    You can pop data from the stack and push to it.
 *    Edit stack.h and change the STACK_ELEMENT to whatever you need.
 *    See stack.h for more details on individual functions.
 */

#include <stdio.h>   // printf
#include <stdlib.h>  // realloc, free, exit, NULL
#include <string.h>  // memset
#include "vec2d_stack.h"


bool stack_init(STACK_TYPE *stack_ptr)
{
   STACK_ELEMENT *new_data = NULL;
   stack_ptr->element_size = sizeof(STACK_ELEMENT);

   // Allocate a memory for stack data
   new_data = (STACK_ELEMENT *)malloc(stack_ptr->element_size);

   if(new_data == NULL) return false;  // failed to initialize

   stack_ptr->data = new_data;
   stack_ptr->max_size = 1; // starts out at 1, increases as needed
   stack_ptr->top = 0;  // IE, empty.

   return true;
}


void stack_destroy(STACK_TYPE *stack_ptr)
{
   // De-allocate memory for stack
   free(stack_ptr->data);

   stack_ptr->data = NULL;  // make certain we point to NULL
   stack_ptr->max_size = 0;
   stack_ptr->top = 0;  // IE, empty.
}


// custom stack push function specifically for Deluxe Pacman 2
bool stack_push(STACK_TYPE *stack_ptr, const int x, const int y)
{
   STACK_ELEMENT *new_data = NULL;
   STACK_ELEMENT element;

   element.y = y;
   element.x = x;

   // If the stack is full, we will reallocate more memory
   if(stack_is_full(stack_ptr)) {
      new_data = (STACK_ELEMENT *)realloc(stack_ptr->data, stack_ptr->max_size * stack_ptr->element_size * 2);   // double the size
      if(new_data == NULL) return false;  // reallocation failed
      stack_ptr->data = new_data;         // point to the new data
      stack_ptr->max_size *= 2;           // set the new maximum size
   }

   // Put new data on top of stack and increment top.
   // note: stack_ptr->top-1 is the last element, so stack_ptr->top = next one
   //       so we place the next element on stack_ptr->top then increment it.
   //stack_ptr->data[stack_ptr->top++] = element;

   for(unsigned int i = stack_ptr->top; i > 0; i--) stack_ptr->data[i] = stack_ptr->data[i - 1];
   stack_ptr->data[0] = element;
   stack_ptr->top++;

   return true;
}


bool stack_pop(STACK_TYPE *stack_ptr, int *x, int *y)
{
   if(stack_is_empty(stack_ptr)) return false;

   stack_ptr->top--;

   *x = stack_ptr->data[stack_ptr->top].x;
   *y = stack_ptr->data[stack_ptr->top].y;

   return true;
}


bool stack_is_empty(STACK_TYPE *stack_ptr)
{
   return stack_ptr->top == 0 ? true : false;
}


bool stack_is_full(STACK_TYPE *stack_ptr)
{
   return stack_ptr->top == stack_ptr->max_size ? true : false;
}


bool stack_shrink(STACK_TYPE *stack_ptr)
{
   STACK_ELEMENT *new_data = NULL;

   // If the stack is empty we'll set it to 1 element in size
   if(stack_is_empty(stack_ptr)) {
      new_data = (STACK_ELEMENT *)realloc(stack_ptr->data, stack_ptr->element_size);
   }
   else {   // stack is not empty, set the new size to the last element
      new_data = (STACK_ELEMENT *)realloc(stack_ptr->data, stack_ptr->top * stack_ptr->element_size);
   }

   if(new_data != NULL) {
      stack_ptr->data = new_data;            // point to reallocated memory
      stack_ptr->max_size = stack_ptr->top;  // reset maximum size
      return true;
   }
   else return false;
}


size_t stack_size(STACK_TYPE *stack_ptr)
{
   return stack_ptr->top;
}
