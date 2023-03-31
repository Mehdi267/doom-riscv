/*
 * Frame allocator for the kernel
 */

#include "frame_dist.h"
#include <assert.h>
#include <stdio.h>

extern char _free_memory_start[];
extern char _memory_end[];
//static char *ptr_end = _memory_end; 

char *mem_ptr;

void init_frames(){
    /*setting up of the physical *mem as a
    linked list. Each block of free mem
    contains a ptr to the next one */
    char *curptr = _free_memory_start;
    while(curptr + FRAME_SIZE < _memory_end){
        *((char**)curptr) = (curptr + FRAME_SIZE);
        curptr += FRAME_SIZE;
    }
    *((char**)curptr) = 0;
    mem_ptr = _free_memory_start;
    // printf("init done");
    assert(curptr < _memory_end);
    assert(curptr + FRAME_SIZE >= _memory_end);
    assert(*((char**)curptr) == 0);
}

void *get_frame(){
    if (*((char**)mem_ptr) == 0){
        //all the mem has been allocated
        return 0; //like malloc
    }
    void *ptr = mem_ptr;
    mem_ptr = *(char**)ptr;
    return ptr;
}


void release_frame(void *frame){
    // printf("[release_frame] Releasing frame with address %p \n", frame);
    // return;
    //LIFO : frame est pointée par mem_ptr
    //frame pointe vers l'ancien mem_ptr
    //checks : la frame est comprise dans la memoire accessible
    if((char *)frame < _free_memory_start || (char *)frame > _memory_end){
        //the frame is not within bounds of physical mem
        return ;
    }

    if(((unsigned long)frame & (unsigned long)0xFFF) != 0){
        //the frame is not correctly aligned
        return ;
    }
    char *temp = mem_ptr;
    mem_ptr = (char*)frame;
    *(char**)frame = temp;
}