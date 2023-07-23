#include "../ulib/syscall.h"
#include "stdio.h"
#include "string.h"

long int ret = 0;

/**
 * @brief if cmd is a builtin, executes the builtin and returns 0, returns 1 if not
 */
int builtin_cmd(char *cmd) {
  if (!strcmp(cmd, "echo $?")) {
    printf("value: %ld\n", ret);
    return 0;
  }
  return 1;
}

int main(void) {
  char cmd[20];
  int pid;
  print_dir_elements(".");
  while (1) {
    printf("shell$>");
    cons_read(cmd, 20);
    if (builtin_cmd(cmd) != 0) {
      pid = start(cmd, 4000, 128, NULL);
      if (pid == -1) {
        printf("shell: program not found: %s\n", cmd);
        ret = -1;
      } else {
        waitpid(pid, &ret);
      }
    }
    memset(cmd, 0, 20);
  }
}
