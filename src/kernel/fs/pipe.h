#ifndef __FS_PIPE__ 
#define __FS_PIPE__

#include <stdint.h>
#include <stdbool.h>
#define PIPE_BUFFER_SIZE 8192

typedef struct file_system_pipe{
  char buffer[PIPE_BUFFER_SIZE];
  //Indicates how many 
  //elements are in the buffer 
  uint32_t cur_buf_cap;
  uint32_t pos_read;
  uint32_t pos_written;
  bool can_write;
  bool can_read;
  bool lock_buf;
  //This semaphore will be used a mutex
  //in order to make sure that the pipe is
  //written and read to at the same time.
  int semaphore_id;
} pipe;


typedef enum close_type{
  CLOSE_ALL,
  CLOSE_READ,
  CLOSE_WRITE,
} close_type;

pipe* create_pipe();
int close_pipe(pipe* pipe, close_type type);
int read_pipe(pipe* pipe, const char* data, int length);
int write_pipe(pipe* pipe, const char* data, int length);
#endif