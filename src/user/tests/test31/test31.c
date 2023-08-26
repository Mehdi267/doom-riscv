/*******************************************************************************
 * Ensimag - Projet Systeme 
 * Test 31
 *
 * Test 31 - fork test
 *
 ******************************************************************************/

#include "sysapi.h"
#include "test31.h"

int test_fork() {
  printf("Inside test31\n");
  int file_fd = open("fork.txt", O_CREAT | O_TRUNC | O_RDWR, 0);
  printf("file_fd = %d\n",file_fd);
  char msg[] = "Hello from the child process\n";
  if (file_fd == -1) {
    printf("Error opening file");
    return 1;
  }
  //To fix small shared pages bug
  // void *test_mappings = shm_create("test31_fork");
  // if (test_mappings == 0){
  //   test_mappings = shm_acquire("test31_fork");
  //   if (test_mappings == 0){
  //     printf("Open shared page failed\n");
  //     return -1;
  //   }
  // }
  // strncpy((char*) test_mappings, msg, strlen(msg));
  pid_t child_pid = fork();
  if (child_pid == -1) {
    printf("fork failed");
    return 1;
  }
  if (child_pid == 0) {
    // This code runs in the child process.
    // assert(memcmp(test_mappings, msg, strlen(msg)) == 0);
    printf("Child process: My PID is %d\n", getpid(), msg);
    printf("msg = %s", msg);
    assert(write(file_fd, msg, strlen(msg)) == (long)strlen(msg));
    close(file_fd);
    exit(0);
  } else {
    // This code runs in the parent process.
    printf("Parent process: My PID is %d, Child PID is %d\n", getpid(), child_pid);

    // Wait for the child process to finish
    long status;
    waitpid_old(child_pid, &status);

    // Rewind the file descriptor to the beginning
    lseek(file_fd, 0, SEEK_SET);

    // Read and print the contents of the file using the file descriptor
    char buffer[256];
    read(file_fd, buffer, strlen(msg));
    printf("buffer = %s\n", buffer);
    assert(memcmp(buffer, msg, strlen(msg)) == 0);
    // Close the file descriptor
    close(file_fd);
  }
  if (unlink("fork.txt")<0){
    printf("delete file failed");
    return -1;
  }
  // shm_release("test31_fork");
  return 0;
}

int main() {
  assert(test_fork() == 0);
  printf("Fork test success\n");
  return 0;
}
