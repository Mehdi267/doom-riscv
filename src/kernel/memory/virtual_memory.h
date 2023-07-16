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
 * @brief The following variables are used to determine 
 * the gigabytes at which some memory elements are stored.
 * @param KERNEL_SPACE: Represents the gigabyte range for kernel space (0->1).
 * @param USERSPACE: Represents the gigabyte range for user space (1->2).
 * @param VRAMSPACE: Represents the gigabyte range for VRAM space (2->4).
*/

/* These codes represent the memory structure and 
*  locations of different code positions.
*  This is used for CEP machine only.
*/
#define KERNEL_SPACE 0
#define USERSPACE 1
#define VRAM_SPACE_1 2
#define VRAM_SPACE_2 3
#define SHARED_PAGES 4

// Base addresses for each memory space
#define KERNEL_SPACE_ADDRESS 0x00000000
#define USERSPACE_ADDRESS 0x40000000
#define VRAM_SPACE_1_ADDRESS 0x80000000
#define VRAM_SPACE_2_ADDRESS 0xC0000000

//Constants
#define SATP_CONF_BASE 0x8000000000000000

// Kernel's base page table
extern page_table *kernel_base_page_table;

/**
 * @brief Initializes the page table that will be used by the kernel.
 * @returns A page_table pointer that holds one entry, which is a gigabyte page.
 * @note This function will be called when allocating memory for the kernel and might be used in other contexts.
*/
extern page_table *init_directory();


/**
 * @brief Creates a page table that will be used by the kernel and processes.
 * @returns A negative value if an error was detected, and a positive value if there weren't any.
 * 
 * This function sets up the virtual memory by creating and configuring the necessary page tables. It allocates:
 * - A gigabyte page that is used to store all kernel memory for scheduling, process management, interrupt handling, and other system-level operations.
 * - A page that is used for user mode, allowing processes to execute in a separate memory space.
 * - Two pages for VRAM management, providing memory regions for video memory.
*/
extern int set_up_virtual_memory();


/**
 * @brief Validates that the virtual memory was set correctly by testing for memory overlap.
 * 
 * This function checks if the virtual memory setup has been done properly and ensures that there are no overlapping memory regions.
 */
extern void debug_memory_overlap();

/**
 * @brief A linked list for page tables that will be used for level 1 and level 0.
 * 
 * This structure represents a linked list of page tables used in the virtual memory system. It contains various fields to manage the page tables and track their usage.
 */
typedef struct page_table_link_list {
    page_table* table;
    struct page_table_link_list* parent_page;
    struct page_table_link_list* head_page;
    struct page_table_link_list* tail_page;
    struct page_table_link_list* next_page;
    uint16_t usage; // Used for total usage
    uint16_t stack_usage; // Used for stack usage. Associated with level 1 
    uint16_t heap_usage; // Used for heap usage. Associated with level 1
    uint16_t shared_memory_usage; // Used for shared memory usage
    int index;
} page_table_link_list_t;

extern int print_mem_symbols();

#endif
