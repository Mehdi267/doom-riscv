
/**
* Projet PCSEA RISC-V
* Mehdi Frikha
* See license for license details.
*/



#ifndef _PROCESS_MEMORY_H_
#define _PROCESS_MEMORY_H_

#include "stdint.h"
#include "process.h"
#include "../memory/pages.h"
#include "../memory/virtual_memory.h"


typedef struct frame_info{
  uint16_t lvl2_index;
  uint16_t lvl1_index;
  uint16_t lvl0_index;
  page_table* page_table; 
} frame_loc;


#define STACK_FRAME_SIZE 128U
#define HEAP_FRAME_SIZE 128U
#define SHARED_FRAME_SIZE 256U 

#define CUSTOM_MOD_DIV(number, divisor) \
    ({ \
        int remainder = (number) % (divisor); \
        (remainder > 0) ? ((number) / (divisor) + 1) : ((number) / (divisor)); \
    })
#define ABS(x) ((x) < 0 ? -(x) : (x))

//These indicies will be used to index elements 
//that are present in the lvl 1 page table
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
extern int add_frame_to_process(process* proc_conf, page_t page_type, frame_loc* frame_info);


/**Copy the data used by the src_proc into the dest_proc,
 * copies the code and all of the used
 * pages in the stack and heap 
*/
extern int copy_process_memory(process* dest_proc, process* src_proc);

/**
 * @brief Get the first stack page of the process given 
 * as function argument
 * @param proc the process that we would like to extract its first page
 * @return void* the pointer to the page
 */
void* get_first_stack_page(process* proc);

/**
 * @brief Checks if we need to expand the stack or an other memory part
 * in order to solve the exception, if it not the case or it the no memory space left
 * at that case we just leave, otherwise we return 0 indicating that space 
 * was needed and the problem was solved
 * @param proc the process that crashed
 * @param frame the trap frame of the process from which we extract
 * the necessary information
 * @return int status
 */
int check_expansion_mem(process* proc, struct trap_frame* frame);

/**
 * @brief Increment or decrement the end of the data segment (heap) of the calling process.
 *
 * The `sbrk` system call adjusts the end of the data segment (heap) of the calling
 * process by incrementing it by the specified `increment` bytes. The new end
 * address must be within the range of valid addresses for the process's address space.
 *
 * @param increment The number of bytes to increment the end address by. A negative
 * value can be used to decrement the end address.
 * @return On success, a pointer to the previous end of the data segment (heap) is
 * returned. On failure, (void *)-1 is returned.
 * @see brk
 */
void *sys_sbrk(long int increment);

#endif