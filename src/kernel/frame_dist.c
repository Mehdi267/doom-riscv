/*
 * Frame allocator for the kernel
 */

#include "frame_dist.h"
#include <assert.h>
#include <stdio.h>

extern char _free_memory_start[];
extern char _memory_end[];
//static char *ptr_end = _memory_end; 

void init_frames(){
    char *curptr = _free_memory_start;
    while(curptr + FRAME_SIZE < _memory_end){
        *((char**)curptr) = (curptr + FRAME_SIZE);
        curptr += FRAME_SIZE;
    }
    *((char**)curptr) = 0;
    printf("init done");
    assert(curptr < _memory_end);
    assert(curptr + FRAME_SIZE >= _memory_end);
    assert(*((char**)curptr) == 0);
}