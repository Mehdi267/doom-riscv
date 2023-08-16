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
  if (pipe->can_write || pipe->can_read){
    //The pipe is still accesible and should be closed
    //by all file descriptor for it to be used
    return -1;
  }else{
    sdelete(pipe->semaphore_id);
    free(pipe);
  }
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
    while (written_length != length&& (pipe->can_read)){
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
          assert(current_write >= 0);
          if ((pipe->nb_written+current_write)/PIPE_BUFFER_SIZE > 0){
            int remaining_space = PIPE_BUFFER_SIZE-pipe->nb_written;
            /*
                    |ffffrrfff|
              We write here ^(remaining_space)
    then We write here ^
            */
            memcpy(pipe->buffer + pipe->nb_written, 
                  data + written_length, remaining_space);
            written_length += remaining_space;
            memcpy(pipe->buffer, data + written_length, current_write - remaining_space);
            pipe->nb_written = current_write - remaining_space;
          }else{
            memcpy(pipe->buffer + pipe->nb_written,
                data + written_length, current_write);
            written_length += current_write; 
            pipe->nb_written += current_write;
          }
          //if this is negative something has went terribly wrong
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
    while ((read_length != length) && (pipe->can_write)){
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
          assert(current_read >= 0);
          if ((pipe->nb_read+current_read)/PIPE_BUFFER_SIZE > 0){
            int remaining_space = PIPE_BUFFER_SIZE-pipe->nb_read;
            memcpy((char*)(data + read_length), 
                  pipe->buffer+pipe->nb_read,
                  remaining_space);
            read_length += remaining_space; 
            memcpy((char*)(data + read_length), data, current_read-remaining_space);
            pipe->nb_read = current_read-remaining_space;
          } else{
            memcpy((char*)(data + read_length), 
                  pipe->buffer + pipe->nb_read, current_read);
            read_length += current_read; 
            pipe->nb_read += current_read;
          }
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