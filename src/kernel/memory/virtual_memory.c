#include <stdbool.h>
#include "pages.h"
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



page_table *init_directory(){
  page_table *directory_ptr = create_page_table();
  if (directory_ptr == NULL){return NULL;}

  #ifndef  VIRTMACHINE
    /**
     * @brief Cep machine directory configuration
     * We create four giga pages : 
     * One page for the kernel  
     * One page for the user space, this page is symbolic and it will point to nothing 
     * at this time
     * two pages for the vram for the vga connection  
     */
    //Giga page for kernel that maps to all of the space that we will use for memory
    page_table_entry *pte_kernel = directory_ptr->pte_list+0; 
    //gigapage is RWX, starting from the adress 0  
    configure_page_entry(pte_kernel, KERNEL_SPACE_ADDRESS, true, true, true, false, GIGA); 
    debug_print_memory("page table address = %p \n",directory_ptr);

    //We allocate memory for the user space
    page_table_entry *user_space_giga = directory_ptr->pte_list+USERSPACE; 
    configure_page_entry(user_space_giga, USERSPACE_ADDRESS, false, false, false, true, GIGA);

    //We also need to add the memory related space for graphical setup to the page table
    //These giga pages are read/write only
    page_table_entry *pte_graphic_memory_1 = directory_ptr->pte_list+VRAM_SPACE_1; 
    page_table_entry *pte_graphic_memory_2 = directory_ptr->pte_list+VRAM_SPACE_2; 
    configure_page_entry(pte_graphic_memory_1, VRAM_SPACE_1_ADDRESS, true, true, false, false, GIGA);
    configure_page_entry(pte_graphic_memory_2, VRAM_SPACE_2_ADDRESS, true, true, false, false, GIGA);  
    /**
    * @brief Used for testing memory overlap 
    * don't delete please
    */
    #ifdef TESTING_MEMORY_OVERLAP
      page_table_entry *second_kernel_space_giga = directory_ptr->pte_list+KERNEL_SPACE+4; 
      configure_page_entry(second_kernel_space_giga, KERNEL_SPACE, true, true, true, GIGA);  
    #endif
  #else
    /**
     * @brief Virt machine configuration
     * In this machine the dram is mapped to the address 0x80000000 thus the kernel
     * must be able to access this part of memory directly and the
     * kernel should also access certain memory region that will be used for controling the uart,
     * the plic and the virtio device
     * An other region that might important is the mmio pci that is mapped at the address 0x40000000 
     */
    page_table_entry *pte_kernel = directory_ptr->pte_list+KERNEL_SPACE; 
    //We map the gigapage that will be used for device control that will accessed by the kernel  
    configure_page_entry(pte_kernel, KERNEL_SPACE_ADDRESS, true, true, true, false, GIGA); 

    /*We map kernel memory in here*/
    page_table_entry *pte_graphic_memory_1 = directory_ptr->pte_list+VRAM_SPACE_1; 
    configure_page_entry(pte_graphic_memory_1, VRAM_SPACE_1_ADDRESS, true, true, true, false, GIGA);
  #endif  


  return directory_ptr;
}

int set_up_virtual_memory(){
    kernel_base_page_table = init_directory();
    if (kernel_base_page_table == NULL){
        return -1;
    }
    long satp_value =  0x8000000000000000 | ((long unsigned int) kernel_base_page_table>>12);
    debug_print_memory("Satp is equal to %lx \n", satp_value);
    csr_write(satp, satp_value); //ppn is 24b0000  
    return 0;
}

void debug_memory_overlap(){
    uint8_t variable = 42;
    uint8_t *ptr1 = &variable;
    uint8_t *ptr2 = ptr1 + 0x100000000;

    printf("ptr1 @%p = %u\n", ptr1, *ptr1);
    printf("ptr2 @%p = %u\n", ptr2, *ptr2);	
    if (*ptr1 == *ptr2)
        puts("Test mémoire virtuelle OK");
    else{
        puts("Test mémoire virtuelle FAIL");
    }
}