/*
    Handler of virtual memory
*/

#include <stdbool.h>
#include <stdint.h>
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
#define SHARED_PAGES 4

#define KERNEL_SPACE_ADDRESS 0x0000000
#define USERSPACE_ADDRESS 0x40000000
#define VRAM_SPACE_1_ADDRESS 0x80000000
#define VRAM_SPACE_2_ADDRESS 0xc0000000


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

/**
 * @brief A linked list for page table that will  be used for the level 1 and level 0 
 * 
 */
typedef struct page_table_link_list{
    page_table* table;
    struct page_table_link_list* parent_page;
    struct page_table_link_list* head_page;
    struct page_table_link_list* tail_page;
    struct page_table_link_list* next_page;
    uint16_t usage; //Used for total usage
    uint16_t stack_usage;//Used for stack usage. Associated with level 1 
    uint16_t heap_usage;//Used for heap usage. Associated with level 1
    uint16_t shared_memory_usage;//Used for Shared memory usage
    int index;
} page_table_link_list_t;



#endif
