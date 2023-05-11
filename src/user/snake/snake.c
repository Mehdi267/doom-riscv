#include "snake.h"
#include "helper_functions.h"

bool snake_in_case(snake_t* snake, position_t target) {
  node_t * curr = snake->head;
  while (curr != NULL) {
    if (curr->p.i == target.i && curr->p.j == target.j) {
      return true;
    }
    curr = curr->next;
  }
  return false;
}

snake_t * create_snake() {
  node_t * queue = malloc(sizeof(node_t));
  node_t * head = malloc(sizeof(node_t));
  queue->p.i = 5; queue->p.j = 5; queue->next = NULL;
  head->p.i = 5; head->p.j = 6; head->next = queue;
  snake_t * snake = malloc(sizeof(snake_t));
  snake->head = head;
  snake->direction = WEST;
  return snake;
}


void get_next_case(snake_t * snake, position_t * p) {
  p->i = snake->head->p.i;
  p->j = snake->head->p.j;
  switch (snake->direction) {
    case NORTH:
      p->i = mod((snake->head->p.i - 1), LINES);
      break;
    case SOUTH:
      p->i = mod((snake->head->p.i + 1), LINES);
      break;
    case EAST:
      p->j = mod((snake->head->p.j - 1), COLUMNS);
      break;
    case WEST:
      p->j = mod((snake->head->p.j + 1), COLUMNS);
      break;
  }
}
