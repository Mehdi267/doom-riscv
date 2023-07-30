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
  #define CURR_DIR_SIZE 50
  char current_dir[CURR_DIR_SIZE];
  int pid;
  char mkdirprog[] = "mkdir";
  char lsprog[] = "ls";
  char cdprog[] = "cd";
  while (1) {
    printf("shell$");
    if (getcwd(current_dir, CURR_DIR_SIZE) != 0){
      printf("%s", current_dir);
    }
    printf("#");
    cons_read(cmd, 20);
    if (memcmp(cmd, "ls", strlen(lsprog)) == 0){
      char curr_path[] = ".";
      print_dir_elements(curr_path);
    }
    else if (memcmp(cmd, "mkdir", strlen(mkdirprog)) == 0){
      char* curr_path = cmd + strlen(mkdirprog) + 1;
      printf("curr_path = %s\n", curr_path);
      mkdir(curr_path, 0);
    }
    else if (memcmp(cmd, "cd", strlen(cdprog)) == 0){
      char* curr_path = cmd + strlen(cdprog) + 1;
      printf("curr_path = %s\n", curr_path);
      chdir(curr_path);
    }
    else if (builtin_cmd(cmd) != 0) {
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
