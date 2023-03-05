/*
 * Frame allocator for the kernel
 */

#ifndef _FRAME_DIST_H
#define _FRAME_DIST_H

#define FRAME_SIZE 4096 //(bytes)

void init_frames();
void *get_frame();
void release_frame(void *frame_ptr);

#endif