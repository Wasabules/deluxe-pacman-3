#pragma once
#include <stdbool.h>

// You can alter this so it matches your data requirements, just keep it named STACK_ELEMENT
typedef struct {
   int x;
   int y;
} STACK_ELEMENT;

// Use this to create your stack object eg: STACK_TYPE stack;
typedef struct {
   STACK_ELEMENT *data;
   size_t element_size;
   unsigned int max_size;
   unsigned int top;
} STACK_TYPE;

bool stack_init(STACK_TYPE *stack_ptr);                           // Allocates memory for the stack and sets initial stack data
void stack_destroy(STACK_TYPE *stack_ptr);                        // De-allocates the stack memory, and resets stack data
bool stack_push(STACK_TYPE *stack_ptr, const int x, const int y); // Adds the element to the top of the stack
bool stack_pop(STACK_TYPE *stack_ptr, int *x, int *y);            // Removes the last element from the stack
bool stack_is_empty(STACK_TYPE *stack_ptr);                       // Returns true if the stack is empty; used by stack_pop()
bool stack_is_full(STACK_TYPE *stack_ptr);                        // Returns true if the stack is full; used by stack_push()
bool stack_shrink(STACK_TYPE *stack_ptr);                         // Removes unused memory from the stack, shrinking it to fit
unsigned int stack_size(STACK_TYPE *stack_ptr);                   // Returns the size of the stack, good for loops
