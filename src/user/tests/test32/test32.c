/*******************************************************************************
 * Ensimag - Projet Systeme 
 * Test 32
 *
 * Test 32 - execve test
 *
 ******************************************************************************/

#include "sysapi.h"
#include "test32.h"

int test_exec(){
  pid_t child_pid = fork();
  if (child_pid == -1) {
    printf("fork failed");
    return 1;
  }
  if (child_pid == 0) {
    const char *new_argv[] = {"program1", "arg1_modified", 
        "arg2_modified", "arg3_modified", NULL};
    char **new_argv_non_const = (char **)(void*)new_argv;
    char *new_envp[] = {NULL};
    
    if (execve("/bin/test_execve", new_argv_non_const, new_envp) <0) {
      printf("execve failed");
      exit(0);
      return -1;
    }
  } else {
    return 0;
  }
  return 0;
}

int main() {
  assert(test_exec() == 0);
  printf("Exec test success\n");
  return 0;
}
