/*
 * Frame allocator for the kernel
 */

#ifndef _FRAME_DIST_H
#define _FRAME_DIST_H

#define PPN0_SIZE 9
#define PPN1_SIZE 9
#define PPN2_SIZE 26

#define PPN0_MASK 0x1FF
#define PPN1_MASK 0x1FF
#define PPN2_MASK 0x3FFFFFF

#define MEGAPAGE_SIZE_EXP 21
#define MEGAPAGE_SIZE ((unsigned long)1<<MEGAPAGE_SIZE_EXP)
#define GIGAPAGE_SIZE_EXP 30
#define GIGAPAGE_SIZE ((unsigned long)1<<GIGAPAGE_SIZE_EXP)
#define PT_SIZE 512 //nb of ptes per pt
#define FRAME_SIZE_EXP 12
#define FRAME_SIZE ((unsigned long)1<<FRAME_SIZE_EXP)

void init_frames();
void *get_frame();
void release_frame(void *frame_ptr);

#endif