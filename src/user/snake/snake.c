#include "../ulib/syscall.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#define LINES 10
#define COLUMNS 50

typedef enum bool { false, true } bool;
typedef enum direction { NORTH, EAST, WEST, SOUTH } direction_t;

typedef struct node_t {
  int i, j;
  struct node_t * next;
} node_t;

typedef struct snake {
  node_t * head;
  direction_t direction;
} snake_t;

bool snake_in_case(snake_t* snake, int i, int j) {
  node_t * curr = snake->head;
  while (curr != NULL) {
    if (curr->i == i && curr->j == j) {
      return true;
    }
    curr = curr->next;
  }
  return false;
}

void print_game(snake_t * snake) {
  for (int i = 0; i < LINES; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      if (snake_in_case(snake, i, j))
        printf("%c", '#');
      else
        printf("%c", '.');
    }
    printf("\n");
  }

}

void clear_terminal(int num_lines) {
    // Move the cursor up 'num_lines' times
    printf("\033[%dA", num_lines);
    // Clear the lines
    printf("\033[J");
}

snake_t * create_snake() {
  printf("ok");
  node_t * queue = malloc(sizeof(node_t));
  printf("ok");
  node_t * head = malloc(sizeof(node_t));
  queue->i = 5; queue->j = 5; queue->next = NULL;
  head->i = 5; head->j = 6; head->next = queue;
  snake_t * snake = malloc(sizeof(snake_t));
  snake->head = head;
  snake->direction = WEST;
  return NULL;
}

void move_snake(snake_t * snake) {
  int prev_i = snake->head->i;
  int prev_j = snake->head->j;
  /* int prev_i = head_i; */
  /* int prev_j = head_j; */
  node_t * curr = snake->head->next;
  while (curr != NULL) {
    int tmp_i = curr->i;
    int tmp_j = curr->j;
    curr->i = prev_i;
    curr->j = prev_j;
    prev_i = tmp_i;
    prev_j = tmp_j;
    curr = curr->next;
  }
  switch (snake->direction) {
    case NORTH:
      snake->head->i -= 1;
      break;
    case SOUTH:
      snake->head->i += 1;
      break;
    case EAST:
      snake->head->j -= 1;
      break;
    case WEST:
      snake->head->j += 1;
      break;
  }
}

int main(void) {
  /* snake_t * snake = create_snake(); */
  node_t queue = {5, 5, NULL};
  node_t head = {5, 6, &queue};
  snake_t s = {&head, WEST};
  snake_t * snake = &s;
  do {
    move_snake(snake);
    print_game(snake);
    sleep(1);
    clear_terminal(LINES);
  } while (1);
  print_game(snake);
}
