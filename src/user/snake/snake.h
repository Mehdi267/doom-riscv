#ifndef  SNAKE_H
#define  SNAKE_H
#define LINES 20
#define COLUMNS 20
#include "stdbool.h"

typedef enum direction { NORTH, EAST, WEST, SOUTH } direction_t;

typedef struct position {
  int i;
  int j;
} position_t;

typedef struct node_t {
  position_t p;
  struct node_t * next;
} node_t;

typedef struct snake {
  node_t * head;
  direction_t direction;
} snake_t;

bool snake_in_case(snake_t* snake, position_t p);
snake_t * create_snake();
void get_next_case(snake_t * snake, position_t * p);
#endif //  SNAKE_H
