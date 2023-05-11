#include "../ulib/syscall.h"
#include "stdio.h"
#include "string.h"
#include "snake.h"
#include "helper_functions.h"

typedef struct game {
  snake_t * snake;
  position_t food;
  int speed;
} game_t ;

bool is_food_position(game_t * game, position_t p) {
  return game->food.i == p.i && game->food.j == p.j;
}

void print_game(game_t * game) {
  snake_t * snake = game->snake;
  position_t p;
  for (int i = 0; i < LINES; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      p.i = i; p.j = j;
      if (snake_in_case(snake, p)) {
        printf("%s", "🟢");
      } else if (is_food_position(game, p)) {
        printf("%s", "🍎");
      } else {
        printf("%s", "⬜");
      }
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


void move_snake(game_t * game) {
  snake_t * snake = game->snake;
  position_t next_pos;
  get_next_case(snake, &next_pos);
  if (snake_in_case(snake, next_pos)) {
    printf("\n GAME OVER \n");
    exit(0);
  }

  if (is_food_position(game, next_pos)) {
    node_t * new_head = malloc(sizeof(node_t));
    new_head->p.i = game->food.i; new_head->p.j = game->food.j;
    new_head->next = snake->head;
    snake->head = new_head;
    do {
      random_case(&(game->food));
    } while (snake_in_case(snake, game->food));
    game->speed = game->speed - (game->speed * 5 / 100);
    return;
  }
  position_t prev = snake->head->p;
  position_t tmp;
  node_t * curr = snake->head->next;
  while (curr != NULL) {
    tmp = curr->p;
    curr->p = prev;
    prev = tmp;
    curr = curr->next;
  }
  snake->head->p = next_pos;
}


int main(void) {
  offset = 0;
  snake_t * snake = create_snake();
  long int fid = pcreate(1);
  int pid = start("input_reader", 4096, 129, (void *)fid);
  (void)pid;
  int count = 0;
  int cmd;
  game_t game;
  game.snake = snake;
  game.speed = 500;
  random_case(&(game.food));
  do {
    pcount(fid, &count);
    if (count > 0) {
      preceive(fid, &cmd);
      switch (cmd) {
        case 'k': case 'w':
          if (snake->direction != SOUTH)
            snake->direction = NORTH;
          break;
        case 'j': case 's':
          if (snake->direction != NORTH)
            snake->direction = SOUTH;
          break;
        case 'l': case 'd':
          if (snake->direction != EAST)
            snake->direction = WEST;
          break;
        case 'h': case 'a':
          if (snake->direction != WEST)
            snake->direction = EAST;
          break;
      }
    }
    move_snake(&game);
    print_game(&game);
    sleep(game.speed); // in milisecods
    clear_terminal(LINES);
  } while (1);
}
