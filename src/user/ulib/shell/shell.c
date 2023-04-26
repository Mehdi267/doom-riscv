#include "../syscall.h"
#include "stdio.h"
#include "string.h"

long int ret;

/**
 * @brief if cmd is a builtin, executes the builtin and returns 0, returns 1 if not
 */
int builtin_cmd(char *cmd) {
  if (strcmp(cmd, "echo $?")) {
    printf("%li\n", ret);
    return 0;
  }
  return 1;
}

int main(void) {
  char cmd[20];
  int pid;
  while (1) {
    cons_read(&cmd, 20);
    if (builtin_cmd(cmd) != 0) {
      pid = start(cmd, 4000, 128, NULL);
      waitpid(pid, &ret);
    }
  }
}
