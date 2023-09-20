/*******************************************************************************
 * Ensimag - Projet Systeme 
 * Test 30
 *
 * Test 30 - pipe test
 *
 ******************************************************************************/

#include "sysapi.h"
#include "test30.h"


int test_pipe_simple() {
  int pipe_fd[2];
  if (pipe(pipe_fd) == -1) {
    printf("pipe");
    return -1;
  }

  // Write data to the write end of the pipe
  const char *write_data = "Hello, pipe!";
  ssize_t bytes_written = write(pipe_fd[1], write_data, strlen(write_data) + 1);
  if (bytes_written == -1) {
    printf("write");
    return -1;
  }

  // Close the write end of the pipe
  close(pipe_fd[1]);

  // Read data from the read end of the pipe
  char read_buffer[13];
  ssize_t bytes_read = read(pipe_fd[0], read_buffer, sizeof(read_buffer));
  if (bytes_read == -1) {
    printf("read");
    return -1;
  }

  // printf("Bytes read = %ld; Read from pipe: %s\n", 
      // bytes_read, read_buffer);
  assert(memcmp(read_buffer, write_data, strlen(write_data)) == 0);
  // Close the read end of the pipe
  close(pipe_fd[0]);
  return 0;
}

int write_pipe(int pipe_write_end){
  // Write data to the write end of the pipe
  const char *write_data = "good pipe!";
  ssize_t bytes_written = write(pipe_write_end, write_data, strlen(write_data) + 1);
  if (bytes_written == -1) {
    printf("write");
    return -1;
  }
  return 0;
}

int test_pipe_overlap() {
  int pipefd[2];
  char buffer[20000]; // A buffer to hold the data

  // Create the pipe
  if (pipe(pipefd) == -1) {
    printf("pipe");
    return 1;
  }

  // Write 6000 bytes into the pipe
  char data1[6000];
  memset(data1, 255, 6000);
  write(pipefd[1], data1, 6000);

  // Read 4000 bytes from the pipe
  ssize_t bytesRead = read(pipefd[0], buffer, 4000);
  buffer[bytesRead] = '\0'; // Null-terminate the string
  // printf("Read %ld\n", bytesRead);

  // Write an additional 6000 bytes into the pipe
  char data2[6000];
  memset(data2, 255, 6000);
  write(pipefd[1], data2, 6000);

  close(pipefd[1]);
  // Read the remaining data from the pipe
  bytesRead = read(pipefd[0], buffer, sizeof(buffer));
  buffer[bytesRead] = '\0'; // Null-terminate the string

  // Close the pipe
  close(pipefd[0]);
  return 0;
}

int read_pipe(int pipe_read_end){
  char read_buffer[11];
  ssize_t bytes_read = read(pipe_read_end, read_buffer, sizeof(read_buffer));
  if (bytes_read == -1) {
    printf("read");
    return -1;
  }
  const char *write_data = "good pipe!";
  assert(memcmp(read_buffer, write_data, strlen(write_data)) == 0);
  return 0;
}

int test_pipe() {
  int pipe_fd[2];
  if (pipe(pipe_fd) == -1) {
    printf("pipe");
    return -1;
  }
  for (int i = 0; i < 500; i++){
    write_pipe(pipe_fd[1]);
  }
  // printf("Completed write_pipe(pipe_fd[1]\n");
  for (int i = 0; i < 400; i++){
    read_pipe(pipe_fd[0]);
  }
  // printf("Completed read_pipe(pipe_fd[0]\n");
  for (int i = 0; i < 500; i++){
    write_pipe(pipe_fd[1]);
  }
  // printf("Completed write_pipe(pipe_fd[1]\n");
  for (int i = 0; i < 600; i++){
    read_pipe(pipe_fd[0]);
  }
  // printf("Completed read_pipe(pipe_fd[0]\n");
  // Close the read end of the pipe
  // printf("Closing pipes\n");
  close(pipe_fd[1]);
  close(pipe_fd[0]);
  return 0;
}



int main(int argc, char *argv) {
  (void)argv;
  (void)argc;
  for (int i = 0; i < 10; i++){
    assert(test_pipe_simple() == 0);
  }
  assert(test_pipe_overlap() == 0);
  for (int i = 0; i < 10; i++){
    assert(test_pipe() == 0);
  }

  return 0;
}