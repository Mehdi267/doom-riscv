#include "process.h"
#include "hash.h"
#include "stddef.h"
#include "string.h"
#include "mem.h"
#include "stdlib.h"
#include "helperfunc.h"
#include "stdio.h"
#include "stdbool.h"
#include "scheduler.h"
#include <assert.h>
#include "process_memory.h"


#define FLOAT_TO_INT(x) (int)((x)+0.5)

#include "stdlib.h"
#include "assert.h"
#include "riscv.h"

#include "bios/info.h"
#include "traps/trap.h"
#include "timer.h"
#include "drivers/splash.h"
#include "../memory/frame_dist.h"
#include "../memory/pages.h"
#include "encoding.h"
#include "../memory/virtual_memory.h"

/**
 * @brief Configure a linked page table node using the parameters taken from the function's arguments
 * and set the index to the parent's usage and incrmenets parent usage by one 
 * @param link_to_configure    
 * @param table page table associated to the node
 * @param parent_page the parent node of the currect node
 * @param head_page child head of the node
 * @param tail_page child tail of the node
 * @param next_page node's brother 
 * @param page_type using the page type we will determine at which part of the bloc the page should 
 * be placed in the parent node
 * @param custom_index the custom index is used in case we want to exploit the index of the lvl 1 page 
 * for example the user gigapage has the index equal to one
 * @return int  a positive value if successful and a negatif value otherwise 
 */
static int configure_page_table_linked_list_entry(page_table_link_list_t** link_to_configure, 
                                                page_table* table,
                                                page_table_link_list_t* parent_page,
                                                page_table_link_list_t* head_page,
                                                page_table_link_list_t* tail_page,
                                                page_table_link_list_t* next_page,
                                                page_t page_type,
                                                int custom_index
                                                ){
    if (table == NULL){
        return -1;
    }
    *link_to_configure = (page_table_link_list_t*) malloc(sizeof(page_table_link_list_t));
    if (*link_to_configure == NULL){
        return -1;
    }
    page_table_link_list_t* node_conf = *link_to_configure;
    node_conf->table = table;
    node_conf->parent_page = parent_page;
    node_conf->head_page = head_page;
    node_conf->tail_page = tail_page;
    node_conf->next_page = next_page;
    node_conf->usage = 0;
    node_conf->stack_usage = 0;
    node_conf->heap_usage = 0;
    node_conf->shared_memory_usage = 0;
    if (parent_page != NULL){
        switch (page_type){
            case STACK_CODE_PAGE:
                node_conf->index = parent_page->stack_usage;
                parent_page->stack_usage++;
                break;
            case HEAP_PAGE:
                node_conf->index = parent_page->heap_usage;
                parent_page->heap_usage++;
                break;
            case SHARED_PAGE:
                node_conf->index = parent_page->shared_memory_usage;
                parent_page->shared_memory_usage++;
                break;
            default:
                break;
        }
        parent_page->usage++;
    }
    else {
        
    }
    return 0;
}

/**
 * @brief This functions take a parent linking node and appens a child to it
 * 
 * @param parent_page_w the parent node that we will append a child to
 * @return a negative int if the operation was not succefully and a postive value other wise 
 */
static int add_child_node_page_table(page_table_link_list_t * parent_page_w, page_t page_type){
    //We could go back to the parent and add an other gigabytes pages but we will work with only one gigabytes pages in here
    if (parent_page_w->usage >= PT_SIZE ||
        parent_page_w->stack_usage >= STACK_FRAME_SIZE ||
        parent_page_w->heap_usage >= HEAP_FRAME_SIZE ||
        parent_page_w->shared_memory_usage >= SHARED_FRAME_SIZE
         ){
        return -1;
    }
    page_table* user_page_table_level_0 = create_page_table();
    page_table_link_list_t* new_page_table_node = NULL;
    if (configure_page_table_linked_list_entry(
        &new_page_table_node,
        user_page_table_level_0,//Table associated to the node
        parent_page_w,
        NULL,
        NULL,
        NULL,
        page_type,
        -1//Lvl index pages are configured using the parent
        )<0){
        return -1;
    }
    //We need to link the new page that was created to the parent
    if (parent_page_w->head_page == NULL && parent_page_w->tail_page == NULL){
        //No children
        parent_page_w->head_page = new_page_table_node;
        parent_page_w->tail_page = new_page_table_node;
    }
    else{
        //Children exist
        parent_page_w->tail_page->next_page = new_page_table_node;
        parent_page_w->tail_page = new_page_table_node;
    }
    return 0;
}

/**
 * @brief Link the lvl1 page table to the lvl0 page table given as function argument with the param 
 * lvl0_table, 
 * @param proc_conf the process that we will apply the action to
 * @param lvl1_index the index at which the linking will occur between the lvl1 and lvl0 
 * @param lvl0_table the table that will be linked
 * @param lvl2_index the level 2 index(remains experimental because we only use giga page for the user currently)
 * @return int a positive value if successful and negative value otherwise
 */
static int link_lvl1_table_shared_page(process* proc_conf, int lvl2_index, int lvl1_index, page_table* lvl0_table){
    if (proc_conf->page_tables_lvl_1_list == NULL){
        return -1;
    }
    if (lvl0_table == NULL){
        return -1;
    }
    //We take pointer associated with the mega page
    //----------Lvl1------
    page_table_link_list_t* lvl_1_node = proc_conf->page_tables_lvl_1_list;
    while (lvl_1_node != NULL) {
        if (lvl_1_node->index == lvl2_index){
            break;
        }
        lvl_1_node = lvl_1_node->next_page;
    }
    page_table_entry* mega_table_entry;
    //We add a link from the level 1 page table to level 0 page table
    //
    //      |-------|   |-------|       |-------|    
    //      |       |   |       |       |       |
    //      |       |   |       | --->  |       |   
    //      |       |   |       |       |       |
    //      |-------|   |-------|       |-------|
    mega_table_entry = lvl_1_node->table->pte_list+lvl1_index;
    configure_page_entry(mega_table_entry,
                        (long unsigned int) lvl0_table, false, false, false, false, KILO);
    return 0;
}

int add_frame_to_process(process* proc_conf, page_t page_type){
    page_table_link_list_t* node_lvl1 = proc_conf->page_tables_lvl_1_list;
    if (node_lvl1 == NULL){
        return -1;
    }
    page_table_link_list_t* lvl0_iterator = node_lvl1->head_page;
    int index_start;
    int index_end;
    switch (page_type){
        case STACK_CODE_PAGE:
            index_start = STACK_CODE_SPACE_START;
            index_end =  STACK_CODE_SPACE_END;
            break;
        case HEAP_PAGE:
            index_start = HEAP_SPACE_START;
            index_end =  HEAP_SPACE_END;
            break;
        case SHARED_PAGE:
            index_start = SHARED_MEMORY_START;
            index_end =  SHARED_MEMORY_END;
            break;
        default:
            break;
    }
    while (lvl0_iterator != NULL){
        //By checking the index we validate that the table location
        //(lvl0 page table must be in the approriate spot)
        // at which the page is being added is valid
        if (lvl0_iterator->index >=  index_start &&
            lvl0_iterator->index<= index_end ){
            if (lvl0_iterator->usage < PT_SIZE){
                if (page_type == SHARED_PAGE){
                    if ( proc_conf == NULL || proc_conf->shared_pages == NULL || proc_conf->shared_pages->tail_shared_page== NULL){
                        return -1;
                    }
                    proc_conf->shared_pages->tail_shared_page->lvl1_index = lvl0_iterator->index;
                    proc_conf->shared_pages->tail_shared_page->lvl0_index = lvl0_iterator->usage;
                    proc_conf->shared_pages->tail_shared_page->lvl2_index = 1; //This value is constant but we might make depend on which source dictetory we choose 
                    proc_conf->shared_pages->tail_shared_page->page_table = lvl0_iterator->table;
                }
                lvl0_iterator->usage++;
                return 0;
            }
        }
        lvl0_iterator = lvl0_iterator->next_page;
    }
    //A page was not found, we need to create a new page and increase its usage
    if (add_child_node_page_table(node_lvl1, page_type) < 0){
        return -1;
    }
    if (page_type == SHARED_PAGE){
        if ( proc_conf == NULL || proc_conf->shared_pages == NULL || proc_conf->shared_pages->tail_shared_page== NULL){
                        return -1;
        }
        proc_conf->shared_pages->tail_shared_page->lvl0_index = node_lvl1->tail_page->usage;
        proc_conf->shared_pages->tail_shared_page->lvl1_index = node_lvl1->tail_page->index;
        proc_conf->shared_pages->tail_shared_page->lvl2_index = 1; //This value is constant but we might make depend on which source dictetory we choose 
        proc_conf->shared_pages->tail_shared_page->page_table = node_lvl1->tail_page->table;
        //We only link sahred pages this way due to their dynamic nature, the other pages are linked using memory allocator
        if (link_lvl1_table_shared_page(proc_conf, USERSPACE, node_lvl1->tail_page->index, node_lvl1->tail_page->table)<0){
            return -1;
        }
    }
    node_lvl1->tail_page->usage++;
    return 0;
}

/**
 * @brief This method should be called only after all the usage values have been set properly 
 * and all of the tree like structures have set up
 * @param proc_conf The process that we will configure its memory
 * @return int a negatif int value if the allocation was not successful and a positive value otherwise
 */
static int allocate_memory_final(process* proc_conf, int start_index, int end_index){
    if (proc_conf->page_tables_lvl_1_list == NULL){
        return -1;
    }
    //We take pointer associated with the mega page
    //----------Lvl1------
    page_table_link_list_t* lvl_1_node = proc_conf->page_tables_lvl_1_list;
    page_table_entry* mega_table_entry;
    //----------Lvl0-------
    //We also need to iterate over the kilo page tables since they also vary in this loop
    page_table_link_list_t* lvl_0_node_iter = lvl_1_node->head_page;
    if(lvl_0_node_iter == NULL){
        return -1;
    }
    //-----Values used in the iteration-----
    page_table* kilo_page_table;
    page_table_entry* kilo_table_entry;
    int kilo_page_usage;
    for (int mega_usage_iter = start_index; mega_usage_iter < end_index; mega_usage_iter++){
        //We add a link from the level 1 page table to level 0 page table
        //
        //      |-------|   |-------|       |-------|    
        //      |       |   |       |       |       |
        //      |       |   |       | --->  |       |   
        //      |       |   |       |       |       |
        //      |-------|   |-------|       |-------|
        mega_table_entry = lvl_1_node->table->pte_list+mega_usage_iter;
        kilo_page_table = lvl_0_node_iter->table;
        if (kilo_page_table == NULL){
            return -1;
        }
        configure_page_entry(mega_table_entry,
                            (long unsigned int) kilo_page_table, false, false, false, false, KILO);
        #ifdef PTE_PAGES_DEBUG
            print_memory_no_arg("-- Mega page pte --\n");
            print_pte(mega_table_entry);
        #endif
        kilo_page_usage = lvl_0_node_iter->usage;
        //Now we allocated memory to every node in kilo page table 
        for (unsigned int kilo_table_usage = 0; kilo_table_usage < kilo_page_usage; kilo_table_usage++){
            kilo_table_entry = lvl_0_node_iter->table->pte_list+kilo_table_usage;
            //Final page level page must in the read/write/exec mode
            configure_page_entry(kilo_table_entry,
                        (long unsigned int )get_frame(), 
                        true,
                        true,
                        true,
                        false, 
                        KILO);
            #ifdef PTE_PAGES_DEBUG
                print_memory_no_arg("-- Kilo page pte --\n");
                print_pte(kilo_table_entry);
            #endif
        }
    }
    return 0;
}


void *process_memory_allocator(process* process_conf, unsigned long size){
    if (size>GIGAPAGE_SIZE){
        return NULL;
    }
    process_conf->stack_shift = 0;
    int size_left = size;
    //the current value of the heap size is set so that we can have at least one frame
    int heap_size = 1; 
    //----------------------LEVEL 2-------
    page_table* user_page_table_level_2 = create_page_table();
    process_conf->page_table_level_2 = user_page_table_level_2;
    //We copy the kernel page table
    memcpy((void*) user_page_table_level_2, (void *) kernel_base_page_table, FRAME_SIZE);
    //-----------------------LEVEL 1/LEVEL 2 LINK-------------------
    page_table* user_page_table_level_1 = create_page_table();
    //We create in here the only page table that will exist at first level since virtual space in this os
    //is limited to one gb
    configure_page_table_linked_list_entry(
        &process_conf->page_tables_lvl_1_list,
        user_page_table_level_1,
        NULL,
        NULL,
        NULL,
        NULL,
        STACK_CODE_PAGE,
        USERSPACE //The user gigapage index is equal to one
        );
    //Make level 2 page table point to level 1 page table in the satp chain
    // 
    //      |-------|        |-------|       |-------|    
    //      |       |        |       |       |       |
    //      |       | -----> |       |       |       |   
    //      |       |        |       |       |       |
    //      |-------|        |-------|       |-------|
    configure_page_entry(user_page_table_level_2->pte_list+USERSPACE,
                    (long unsigned int )user_page_table_level_1,
                    false,
                    false, 
                    false,
                    false,
                    KILO);
    #ifdef PTE_PAGES_DEBUG
        debug_print_memory("-----Second level pte kernel/process directory when working with process : %d",process_conf->pid);
        print_pte(user_page_table_level_2->pte_list+USERSPACE);
    #endif
    //We associate the necessary frames and page tables for the stack 
    do{
        if (add_frame_to_process(process_conf, STACK_CODE_PAGE)<0){
            puts("problem with memory allocator :  frame allocator stack\n");
        }
        process_conf->stack_shift++;
        size_left -= FRAME_SIZE;
    }
    while(size_left > 0);
    
    //We associate the necessary frames and the page tables for the heap  
    do{
        if (add_frame_to_process(process_conf, HEAP_PAGE)<0){
            puts("problem with memory allocator :  frame allocator heap \n");
        }
        heap_size -= FRAME_SIZE;
    }
    while(heap_size > 0);
    
    if (process_conf->page_tables_lvl_1_list->head_page->table == NULL){
        puts("TABLE IS NULL !!!!! \n");
    }
    //We allocate space for the stack and code 
    if (allocate_memory_final(process_conf, STACK_CODE_SPACE_START, STACK_CODE_SPACE_START +process_conf->page_tables_lvl_1_list->stack_usage) < 0){
        puts("problem with final memory allocator\n");
    }
    //We allocate space for the heap
    if (allocate_memory_final(process_conf, HEAP_SPACE_START, HEAP_SPACE_START +process_conf->page_tables_lvl_1_list->heap_usage) < 0){
        puts("problem with final memory allocator\n");
    }
    return get_frame();
}