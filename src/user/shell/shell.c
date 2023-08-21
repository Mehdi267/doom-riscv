#include "../ulib/syscall.h"
#include "../ulib/ufunc.h"
#include "stdio.h"
#include "string.h"

long int ret = 0;

#include <stdio.h>
#include <stdlib.h>

int dis(char *prog) {
  const char *filename = prog;
  int fd = open(filename, O_RDONLY, 0);
  if (fd == -1) {
    printf("Error opening file");
    return 1;
  }

  char buffer[4]; // A RISC-V 32-bit instruction is 4 bytes
  char *result;
  int i = 0; 
  
  while ((result = my_fgets(buffer, sizeof(buffer), fd)) != NULL) {
    printf("%08x\n", *(unsigned int *)buffer); // Print 32-bit value as hex
    i++;
  }
  
  printf("Number of instructions: %d\n", i);
  
  close(fd);

  return 0;
}

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

int main() {
  //stdin
  assert(open("/dev/terminal", O_RDONLY, 0) == 0);
  //stdout
  assert(open("/dev/terminal", O_WRONLY, 0) == 1);
  //stderr
  assert(dup2(1, 2) == 2);
  char cmd[20];
  #define CURR_DIR_SIZE 50
  char current_dir[CURR_DIR_SIZE];
  int pid;
  char mkdirprog[] = "mkdir";
  char lsprog[] = "ls";
  char cdprog[] = "cd";
  char rmdirprog[] = "rmdir";
  char unlinkprog[] = "unlink";
  char disprog[] = "dis";
  char exitshell[] = "exit";
  char voidsysprog[] = "voidsys";
  // #ifdef VIRTMACHINE
  //Loads all of the elfs into the disk
  ld_progs_into_disk();
  // #endif
  while (1) {
  printf("shell$");
  if (getcwd(current_dir, CURR_DIR_SIZE) != 0){
    printf("%s", current_dir);
  }
  printf("#");
  // my_fgets(cmd, 20, 0);
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
  else if (memcmp(cmd, "rmdir", strlen(rmdirprog)) == 0){
    char* curr_path = cmd + strlen(rmdirprog) + 1;
    printf("curr_path = %s\n", curr_path);
    rmdir(curr_path);
  }
  else if (memcmp(cmd, "unlink", strlen(unlinkprog)) == 0){
    char* curr_path = cmd + strlen(unlinkprog) + 1;
    printf("curr_path = %s\n", curr_path);
    unlink(curr_path);
  }
  else if (memcmp(cmd, "dis", strlen(disprog)) == 0){
    char* curr_path = cmd + strlen(disprog) + 1;
    dis(curr_path);
  }
  else if (memcmp(cmd, "exit", strlen(exitshell)) == 0){
    exit(0);
  }
  else if (memcmp(cmd, "voidsys", strlen(voidsysprog)) == 0){
    void_call();
  }
  else if (builtin_cmd(cmd) != 0) {
    pid = start(cmd, 8000, 128, NULL);
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
