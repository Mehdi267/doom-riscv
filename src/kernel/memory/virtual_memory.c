#include <stdbool.h>
#include "pages.h"
#include "frame_dist.h"
#include "../process/helperfunc.h"
#include "process/process.h"
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "riscv.h"
#include "virtual_memory.h"

page_table *kernel_base_page_table;

/**
 * @brief Initializes the page table directory used by the kernel.
 * @return A pointer to the created page table directory, or NULL if an error occurred.
 */
page_table *init_directory() {
    page_table *directory_ptr = create_page_table();
    if (directory_ptr == NULL) {
        return NULL;
    }

    #ifndef VIRTMACHINE
    /**
     * @brief Cep machine directory configuration
     * 
     * In Cep machine, the following configuration is done:
     * - One gigabyte page for the kernel, mapping to all the memory space used by the kernel.
     * - One gigabyte page for user space (symbolic, currently not pointing to anything).
     * - Two gigabyte pages for VRAM (used for VGA connection).
     */
    // Gigabyte page for the kernel, maps to all memory space used by the kernel.
    page_table_entry *pte_kernel = directory_ptr->pte_list + KERNEL_SPACE;
    configure_page_entry(pte_kernel, KERNEL_SPACE_ADDRESS, true, true, true, false, GIGA);
    debug_print_memory("page table address = %p \n", directory_ptr);

    // Allocating memory for user space
    page_table_entry *user_space_giga = directory_ptr->pte_list + USERSPACE;
    configure_page_entry(user_space_giga, USERSPACE_ADDRESS, false, false, false, true, GIGA);

    // Adding memory-related space for graphical setup to the page table
    // These gigabyte pages are read/write only
    page_table_entry *pte_graphic_memory_1 = directory_ptr->pte_list + VRAM_SPACE_1;
    page_table_entry *pte_graphic_memory_2 = directory_ptr->pte_list + VRAM_SPACE_2;
    configure_page_entry(pte_graphic_memory_1, VRAM_SPACE_1_ADDRESS, true, true, false, false, GIGA);
    configure_page_entry(pte_graphic_memory_2, VRAM_SPACE_2_ADDRESS, true, true, false, false, GIGA);

    /**
     * @brief Used for testing memory overlap
     * Please do not delete this section.
     */
    #ifdef TESTING_MEMORY_OVERLAP
    page_table_entry *second_kernel_space_giga = directory_ptr->pte_list + KERNEL_SPACE + 4;
    configure_page_entry(second_kernel_space_giga, KERNEL_SPACE, true, true, true, GIGA);
    #endif

    #else
    /**
     * @brief Virt machine configuration
     * 
     * In this machine, the DRAM is mapped to address 0x80000000. Therefore, the kernel
     * must be able to access this memory directly. The kernel should also access specific memory regions
     * used for controlling UART, PLIC, and VirtIO devices. Another important region is the MMIO PCI,
     * mapped at address 0x40000000.
     */
    page_table_entry *pte_kernel = directory_ptr->pte_list + KERNEL_SPACE;
    // Mapping the gigabyte page used for device control that will be accessed by the kernel
    configure_page_entry(pte_kernel, KERNEL_SPACE_ADDRESS, true, true, true, false, GIGA);

    // Mapping kernel memory
    page_table_entry *pte_graphic_memory_1 = directory_ptr->pte_list + VRAM_SPACE_1;
    configure_page_entry(pte_graphic_memory_1, VRAM_SPACE_1_ADDRESS, true, true, true, false, GIGA);
    #endif

    return directory_ptr;
}

/**
 * @brief Sets up the virtual memory.
 * @return 0 if successful, -1 if an error occurred.
 */
int set_up_virtual_memory() {
    kernel_base_page_table = init_directory();
    if (kernel_base_page_table == NULL) {
        return -1;
    }
    long satp_value = SATP_CONF_BASE | ((long unsigned int)kernel_base_page_table >> 12);
    debug_print_memory("Satp is equal to %lx \n", satp_value);
    csr_write(satp, satp_value); // ppn is 24b0000
    return 0;
}
