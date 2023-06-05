/*
 * Frame allocator for the kernel
*/

#ifndef _FRAME_DIST_H
#define _FRAME_DIST_H

//Sizes of the different blocks of the ppn memory
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

/**
 * @brief Divides the free memory into chucks, the size of the chucks is depedant on the   
 * size of the frame that specific to the operating system
 */
void init_frames();

/**
 * @brief Returns a frame that is not been used, the size of the frame is equal to the constant FRAMZ_SIZE
 * 
 * @return void* Returns and adress to a frame if it found a free and NULL address 
 * it did not find any avalibale frames
 */
void * get_frame();

/**
 * @brief takes and adress to a frame and frees it(make it avaliable for future use)  
 * 
 * @param frame_ptr the adress of the frame that we want to release 
 */
void release_frame(void *frame_ptr);

#endif