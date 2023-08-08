#include "pipe.h"
#include "inode.h"
#include "../sync/semaphore_api.h"
#include "string.h"
#include "assert.h"

pipe* create_pipe(){
  pipe* new_pipe = (pipe*) malloc(sizeof(pipe));
  if (new_pipe == 0){
    goto function_fail;
  }
  memset(new_pipe, 0, sizeof(pipe));
  new_pipe->can_write = true;
  new_pipe->can_read = true;
  new_pipe-> semaphore_id = screate(1);
  new_pipe->lock_buf = false;
  if (new_pipe->semaphore_id == 0){
    goto function_fail;
  }
  return new_pipe;
  function_fail:
    if (new_pipe){
      free(new_pipe);
    }
    return 0;
}

int close_pipe(pipe* pipe, close_type type){
  if (pipe == 0){
    //Pipe is empty didnot fail
    return 0;
  }
  switch (type){
    case (CLOSE_ALL):
      pipe->can_read = false;
      pipe->can_write = false;
      break;
    case (CLOSE_READ):
      pipe->can_read = false;
      break;
    case (CLOSE_WRITE):
      pipe->can_write = false;
      break;
    default:
      break;
  }
  if (pipe->can_write || pipe->can_write){
    //The pipe is still accesible and should be closed
    //by all file descriptor for it to be used
    return -1;
  }
  sdelete(pipe->semaphore_id);
  free(pipe);
  return 0;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int write_pipe(pipe* pipe, const char* data, int length){
  if (!(pipe && data && pipe->can_write)){
    return -1;
  }
  if (length < 0){
    return -1;
  }
  int written_length = 0;
  if (wait(pipe->semaphore_id)>=0){
    while (written_length != length){
      if (pipe->cur_buf_cap == PIPE_BUFFER_SIZE){
        //The other process must read the data 
        //before we write more into it
        signal(pipe->semaphore_id);
      } else{
        //might cause race condition in a multi processor
        if (!pipe->lock_buf){
          pipe->lock_buf = true;
          int current_write = MIN(PIPE_BUFFER_SIZE -
                              pipe->cur_buf_cap, length - written_length);
          //if this is negative something has went terriblaly wrong
          assert(current_write >= 0);
          memcpy(pipe->buffer, data + written_length, current_write);
          written_length += current_write; 
          pipe->cur_buf_cap += current_write;
          pipe->lock_buf = false;
          signal(pipe->semaphore_id);
        }
      }
      if (written_length != length || pipe->cur_buf_cap == PIPE_BUFFER_SIZE){
        wait(pipe->semaphore_id);
      }
    }
  }
  return written_length;
}

int read_pipe(pipe* pipe, const char* data, int length){
  if (!(pipe && data && pipe->can_read)){
    return -1;
  }
  if (length < 0){
    return -1;
  }
  int read_length = 0;
  if (wait(pipe->semaphore_id)>=0){
    while (read_length != length){
      if (pipe->cur_buf_cap == 0){
        //The other process must read the data 
        //before we write more into it
        signal(pipe->semaphore_id);
      } else{
        //might cause race condition in a multi processor
        if (!pipe->lock_buf){
          pipe->lock_buf = true;
          int current_read = MIN(pipe->cur_buf_cap,
                              length - read_length);
          //if this is negative something has went terriblaly wrong
          assert(current_read >= 0);
          //wrong
          memcpy((char*)(data + read_length), pipe->buffer, current_read);
          read_length += current_read; 
          pipe->cur_buf_cap -= current_read;
          pipe->lock_buf = false;
          signal(pipe->semaphore_id);
        }
      }
      if (read_length != length || pipe->cur_buf_cap == 0){
        wait(pipe->semaphore_id);
      }
    }
  }
  return read_length;
}