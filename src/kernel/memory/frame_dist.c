/*
 * Frame allocator for the kernel
 */

#include "frame_dist.h"
#include "stdint.h"
#include <assert.h>
#include <stdio.h>

// External symbols for memory boundaries
extern char _free_memory_start[];
extern char _memory_end[];

// Pointer to the current free memory block
char *mem_ptr;
uint64_t mem_usage;
uint64_t mem_total;

/**
 * @brief Initializes the frame allocator by setting up the linked list of free memory blocks.
 */
void init_frames() {
    char *curptr = _free_memory_start;

    // Setting up the physical memory as a linked list
    while (curptr + FRAME_SIZE < _memory_end) {
        *((char**)curptr) = (curptr + FRAME_SIZE);
        curptr += FRAME_SIZE;
    }
    
    *((char**)curptr) = 0; // Mark the end of the list
    
    mem_ptr = _free_memory_start;
    mem_usage = 0;
    mem_total = (_memory_end - _free_memory_start)/FRAME_SIZE;
    // Assertions to ensure correctness
    assert(curptr < _memory_end);
    assert(curptr + FRAME_SIZE >= _memory_end);
    assert(*((char**)curptr) == 0);
}

/**
 * @brief Retrieves a free frame from the frame allocator.
 * @return Pointer to the allocated frame, or NULL if all memory has been allocated.
 */
void *get_frame() {
    if (*((char**)mem_ptr) == 0) {
        // All memory has been allocated
        return 0; // Like malloc
    }
    mem_usage++;
    void *ptr = mem_ptr;
    mem_ptr = *(char**)ptr;
    // printf("Getting frame %p\n", ptr);
    return ptr;
}

/**
 * @brief Releases a frame and adds it back to the free memory list.
 * @param frame Pointer to the frame to be released.
 */
void release_frame(void *frame) {
    // LIFO: frame is pointed to by mem_ptr
    // frame points to the previous mem_ptr
    // printf("Releasing frame %p\n", frame);
    // Check if the frame is within the bounds of physical memory
    if ((char*)frame < _free_memory_start || (char*)frame > _memory_end) {
        // The frame is not within the bounds of physical memory
        return;
    }
    
    // Check if the frame is correctly aligned
    if (((unsigned long)frame & (unsigned long)0xFFF) != 0) {
        // The frame is not correctly aligned
        return;
    }
    mem_usage--;
    char *temp = mem_ptr;
    mem_ptr = (char*)frame;
    *(char**)frame = temp;
}

void print_mem_usage(){
  printf("Mem usage = %ld/%ld %ld per cent\n", mem_usage, mem_total, (mem_usage*100)/mem_total);
}