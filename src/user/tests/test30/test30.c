/*******************************************************************************
 * Ensimag - Projet Systeme 
 * Test 30
 *
 * Test 30 - docs
 *
 ******************************************************************************/

#include "sysapi.h"
#include "test30.h"


int test30() {
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

    printf("Read from pipe: %s\n", read_buffer);
    assert(memcmp(read_buffer, write_data, strlen(write_data)) == 0);
    // Close the read end of the pipe
    close(pipe_fd[0]);
    return 0;
}


int main() {
  assert(test30() == 0);
  return 0;
}