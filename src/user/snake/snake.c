#include "../ulib/syscall.h"
#include "stdio.h"
#include "string.h"
#define LINES 10
#define COLUMNS 50
#define MEMORY_SIZE 1000

typedef enum bool { false, true } bool;
typedef enum direction { NORTH, EAST, WEST, SOUTH } direction_t;

char memory_pool[MEMORY_SIZE];
size_t offset = 0;
void * malloc(size_t size) {
  printf("size %lu\n", offset + size);
  if (offset + size > MEMORY_SIZE) {
    printf("segmetation fault \n");
    exit(-1);
  }
  char * res = memory_pool + offset;
  offset += size;
  return res;
}

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
  node_t * queue = malloc(sizeof(node_t));
  node_t * head = malloc(sizeof(node_t));
  queue->i = 5; queue->j = 5; queue->next = NULL;
  head->i = 5; head->j = 6; head->next = queue;
  snake_t * snake = malloc(sizeof(snake_t));
  snake->head = head;
  snake->direction = WEST;
  return snake;
}

static int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
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
      snake->head->i = mod((snake->head->i - 1), LINES);
      break;
    case SOUTH:
      snake->head->i = mod((snake->head->i + 1), LINES);
      break;
    case EAST:
      snake->head->j = mod((snake->head->j - 1), COLUMNS);
      break;
    case WEST:
      snake->head->j = mod((snake->head->j + 1), COLUMNS);
      break;
  }
}

int main(void) {
  offset = 0;
  snake_t * snake = create_snake();
  printf("snake %p\n", snake);
  /* int * t = malloc(1); */
  /* (void)t; */
  /* node_t queue = {5, 5, NULL}; */
  /* node_t head = {5, 6, &queue}; */
  /* snake_t s = {&head, WEST}; */
  /* snake_t * snake = &s; */
  do {
    move_snake(snake);
    print_game(snake);
    sleep(1);
    clear_terminal(LINES);
  } while (1);
  print_game(snake);
}
