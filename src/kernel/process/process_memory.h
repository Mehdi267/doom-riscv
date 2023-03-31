
/**
* Projet PCSEA RISC-V
* Mehdi Frikha
* See license for license details.
*/


#include "stdint.h"
#include "process.h"
#include "../memory/pages.h"

#ifndef _PROCESS_MEMORY_H_
#define _PROCESS_MEMORY_H_


typedef enum _page_type {
    STACK_CODE_PAGE,
    HEAP_PAGE,
    SHARED_PAGE
} page_t;

#define STACK_FRAME_SIZE 128
#define HEAP_FRAME_SIZE 128
#define SHARED_FRAME_SIZE 256 


#define STACK_CODE_SPACE_START 0
#define STACK_CODE_SPACE_END 127 
#define HEAP_SPACE_START 128
#define HEAP_SPACE_END 255
#define SHARED_MEMORY_START 256
#define SHARED_MEMORY_END 511

#define GIGA_SIZE 0x40000000
#define MEGA_SIZE 0x200000
#define KILO_SIZE 0x1000

/**
* @brief This function allocates memory for a process, it's current
* form remains very basic and does not follow the project specifications
* and it is only valid for a size that is less than then page size
* @param size corresponds to the size that we want to allocate
* @param process_conf the process at which the memory allocater will work on 
* @return a positve value if successful and negative value otherwise
*/
extern int process_memory_allocator(process* process_conf, unsigned long size);


/**
 * @brief add a frame to the process given as function argument 
 * The adding operation gets done by simply changing the usage of certain parameters
 * 
 * @note This function does not allocate the final usable pages
 * @param proc_conf 
 * @param page_type the type the page that we are adding(When working with shared page
 *  we save the index of the certain process structs) 
 * @return int a positve value if the operation was successful and negative value otherwise 
 */
extern int add_frame_to_process(process* proc_conf, page_t page_type);


#endif