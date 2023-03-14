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


page_table *kernel_base_page_table;



page_table *init_directory(){
  page_table *directory_ptr = create_page_table();
  if (directory_ptr == NULL){return NULL;}
  
  page_table_entry *pte_kernel = directory_ptr->pte_list+0; // La 1ère PTE du directory pointera vers la gigapage
  //Giga page for kernel memory // stack
  configure_page_entry(pte_kernel, KERNEL_SPACE, true, true, true, false, GIGA); //gigapage is RWX, starting from the adress 0  
  
  debug_print_memory("page table address = %p \n",directory_ptr);

  //We allocate memory for the user space
  page_table_entry *user_space_giga = directory_ptr->pte_list+USERSPACE; 
  configure_page_entry(user_space_giga, USERSPACE, false, false, false, true, GIGA);

  //We also need to add the memory related space for graphical setup to the page table
  //These giga pages are read/write only
  page_table_entry *pte_graphic_memory_1 = directory_ptr->pte_list+VRAM_SPACE_1; 
  page_table_entry *pte_graphic_memory_2 = directory_ptr->pte_list+VRAM_SPACE_2; 
  configure_page_entry(pte_graphic_memory_1, VRAM_SPACE_1, true, true, false, false, GIGA);
  configure_page_entry(pte_graphic_memory_2, VRAM_SPACE_2, true, true, false, false, GIGA);  
  
  /**
   * @brief Used for testing memory overlap 
   * don't delete please
   */
  #ifdef TESTING_MEMORY_OVERLAP
    //   page_table_entry *second_kernel_space_giga = directory_ptr->pte_list+KERNEL_SPACE+4; 
    //   configure_page_entry(second_kernel_space_giga, KERNEL_SPACE, true, true, true, GIGA);  
  #endif

  return directory_ptr;
}

int set_up_virtual_memory(){
    init_frames();
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