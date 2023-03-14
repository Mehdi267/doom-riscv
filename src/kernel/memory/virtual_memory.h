/*
    Handler of virtual memory
*/

#include <stdbool.h>
#include "frame_dist.h"
#include "pages.h"

#ifndef _VIRTUAL_MEMORY_
#define _VIRTUAL_MEMORY_

/**
 * @brief The following variables are used to deterine 
 * the gigabytes at which the some memro yelements are stored
 * @param KERNEL_SPACE 0->1
 * @param USERSPACE 0->2
 * @param VRAMSPACE 2->4
*/

#define KERNEL_SPACE 0
#define USERSPACE 1
#define VRAM_SPACE_1 2
#define VRAM_SPACE_2 3

//Kernel's base page table
extern page_table *kernel_base_page_table;

/**
 * @brief init_directory creates the page table that we will be used by the kernel
 * @returns a page_table pointer that holds one entry which is a gigabytes page
 * @note This function will be called when we are trying to allcoate memory for the kernel and might be used in other context
*/
extern page_table *init_directory();


/**
 * @brief creates a page table taht will be used by the kernel and processes
 * and calls
 * @returns a negatif if an error was detected and positif value if there weren't any
 * - a gigabyte page that is used to store all kernel memory that will be used for scheduling 
 * process managment and intterupt handeling and other things (view memory layout at https://gitlab.ensimag.fr/pcserv/documentation-risc-v/-/wikis/Adressage-et-m%C3%A9moire-virtuelle)
 * - a page that is used for the user mode
 * - two pages for vram management
*/
extern int set_up_virtual_memory();


/**
 * @brief debug_memory_overlap is used to validate that the virtual memory was correctly set
 * it test that the overlap has been done properly
 */
extern void debug_memory_overlap();

#endif
