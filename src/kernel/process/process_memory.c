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
#include "process_memory.h"

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
#include "memory_api.h"
#include "fs_bridge.h"

//Saves the information of the empty page that was found
static void save_frame_data(page_table_link_list_t* lvl0_iterator,
     frame_loc* frame_info, page_t page_type){
  if (!(frame_info && lvl0_iterator)){
    return; 
  }
  if (page_type == STACK_PAGE){
    frame_info->lvl0_index = PT_SIZE - 1 - lvl0_iterator->usage;
  }
  else{
    frame_info->lvl0_index = lvl0_iterator->usage;
  }
  frame_info->lvl1_index = lvl0_iterator->index;
  frame_info->lvl2_index = USERSPACE;//The only used page location at the moment
  frame_info->page_table = lvl0_iterator->table;
}

/**
 * @brief Configure a linked page table node using the parameters taken from the function's arguments
 * and set the index to the parent's usage and increment parent usage by one.
 * @param link_to_configure Pointer to a pointer where the configured node will be stored.
 * @param table Page table associated with the node.
 * @param parent_page The parent node of the current node.
 * @param head_page Child head of the node.
 * @param tail_page Child tail of the node.
 * @param next_page Node's brother.
 * @param page_type Using the page type we will determine at which part of the block the page should 
 * be placed in the parent node.
 * @param custom_index The custom index is used in case we want to exploit the index of the level 1 page, 
 * for example, the user gigapage has the index equal to one.
 * @return int A positive value if successful and a negative value otherwise.
 */
static int configure_page_table_linked_list_entry(page_table_link_list_t** link_to_configure, 
                        page_table* table,
                        page_table_link_list_t* parent_page,
                        page_table_link_list_t* head_page,
                        page_table_link_list_t* tail_page,
                        page_table_link_list_t* next_page,
                        page_table_link_list_t* previous_page,
                        page_t page_type,
                        int custom_index
                        ){
  if (table == NULL){return -1;}
  *link_to_configure = (page_table_link_list_t*) malloc(sizeof(page_table_link_list_t));
  if (*link_to_configure == NULL){return -1;}
  page_table_link_list_t* node_conf = *link_to_configure;
  memset(node_conf, 0, sizeof(page_table_link_list_t));
  node_conf->page_type = page_type; 
  node_conf->table = table;
  node_conf->parent_page = parent_page;
  node_conf->head_page = head_page;
  node_conf->tail_page = tail_page;
  node_conf->next_page = next_page;
  node_conf->previous_page = previous_page;
  node_conf->link_index_table = 0;
  if (parent_page != NULL){
    //Used for lvl 0 pages
    switch (page_type){
      case CODE_PAGE:
        node_conf->index = STACK_CODE_SPACE_START + parent_page->code_usage;
        parent_page->code_usage++;
        break;
      case STACK_PAGE:
        node_conf->index = STACK_CODE_SPACE_END - parent_page->stack_usage;
        parent_page->stack_usage++;
        break;
      case HEAP_PAGE:
        node_conf->index = HEAP_SPACE_START + parent_page->heap_usage;
        parent_page->heap_usage++;
        break;
      case SHARED_PAGE:
        node_conf->index = SHARED_MEMORY_START + parent_page->shared_memory_usage;
        parent_page->shared_memory_usage++;
        break;
      default:
        break;
    }
    // printf("Setting node conf index %d\n", node_conf->index);
    hash_set(parent_page->link_index_table, 
          (void *)((uint64_t)node_conf->index), link_to_configure);
    parent_page->usage++;
  }
  else {
    if (page_type == NONE_PAGE){
      node_conf->link_index_table = (hash_t *)malloc(sizeof(hash_t));
      if (node_conf->link_index_table == 0){
        free(node_conf->table); 
        free(node_conf); 
        return -1;
      }
      hash_init_direct(node_conf->link_index_table);
    }
    node_conf->index = custom_index;
  }
  return 0;
}

/**
 * @brief This functions take a parent linking node and appends a child to it
 * @note will mostly be used to link lvl0 table to lvl1 table
 * @param parent_page_w the parent node that we will append a child to
 * @return a negative int if the operation was not succefully and a postive value other wise 
 */
static int add_child_node_page_table(process* proc_conf, page_table_link_list_t * parent_page_w, page_t page_type){
  //We could go back to the parent and add an other gigabyte pages but we will work with only one gigabyte page in here
  if (parent_page_w->usage >= PT_SIZE ||
    parent_page_w->stack_usage >= STACK_FRAME_SIZE ||
    parent_page_w->heap_usage >= HEAP_FRAME_SIZE ||
    parent_page_w->shared_memory_usage >= SHARED_FRAME_SIZE){
    return -1;
  }
  page_table* user_page_table_level_0 = create_page_table();
  if (user_page_table_level_0 == NULL){
    return -1;
  }
  page_table_link_list_t* new_page_table_node = NULL;
  if (configure_page_table_linked_list_entry(
    &new_page_table_node,
    user_page_table_level_0,//Table associated to the node
    parent_page_w,
    NULL,NULL,NULL,NULL,
    page_type,
    -1//Lvl index pages are configured using the parent
    )<0){
    return -1;
  }
  debug_print_memory("Child page creation with index %d and page table address %p\n", 
            new_page_table_node->index, 
            user_page_table_level_0);
  //We need to link the new page that was created to the parent
  if (parent_page_w->head_page == NULL && parent_page_w->tail_page == NULL){
    debug_print_memory("No children node exist in the lvl 1 table thus linking index page table %d \n",
       new_page_table_node->index);
    //No children
    parent_page_w->head_page = new_page_table_node;
    parent_page_w->tail_page = new_page_table_node;
  }
  else{
    debug_print_memory("Children node index %d \n",
       new_page_table_node->index);
    //Children exist
    parent_page_w->tail_page->next_page = new_page_table_node;
    new_page_table_node->previous_page = parent_page_w->tail_page;
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
static int link_lvl1_table_direct(process* proc_conf, int lvl2_index, int lvl1_index, page_table* lvl0_table){
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
  //     |-------|   |-------|      |-------|   
  //     |       |   |       |      |       |
  //     |       |   |       | ---> |       |   
  //     |       |   |       |      |       |
  //     |-------|   |-------|      |-------|
  mega_table_entry = lvl_1_node->table->pte_list+lvl1_index;
  configure_page_entry(mega_table_entry,
            (long unsigned int) lvl0_table, false, false, false, true, KILO);
  return 0;
}

int add_frame_to_process(process* proc_conf, page_t page_type, frame_loc* frame_info){
  page_table_link_list_t* node_lvl1 = proc_conf->page_tables_lvl_1_list;
  if (node_lvl1 == NULL){
    return -1;
  }
  page_table_link_list_t* lvl0_iterator = node_lvl1->head_page;
  int index_start;
  int index_end;
  switch (page_type){
    case CODE_PAGE:
      proc_conf->mem_info.code_usage++;
      index_start = STACK_CODE_SPACE_START;
      index_end =  STACK_CODE_SPACE_END;
      break;
    case STACK_PAGE:
      proc_conf->mem_info.stack_usage++;
      index_start = STACK_CODE_SPACE_START;
      index_end =  STACK_CODE_SPACE_END;
      break;
    case HEAP_PAGE:
      proc_conf->mem_info.heap_usage++;
      index_start = HEAP_SPACE_START;
      index_end =  HEAP_SPACE_END;
      break;
    case SHARED_PAGE:
      proc_conf->mem_info.shared_pages_usage++;
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
    if (lvl0_iterator->index >= index_start &&
        lvl0_iterator->index<= index_end &&
        lvl0_iterator->page_type == page_type){
      if (lvl0_iterator->usage < PT_SIZE){
        if (frame_info != 0){save_frame_data(lvl0_iterator, frame_info, page_type);}
        //We found a empty page, and we then increase its usage
        lvl0_iterator->usage++;
        debug_print_memory("Found an empty slot index %d, usage %d\n", 
                lvl0_iterator->index, lvl0_iterator->usage);
        return 0;
      }
    }
    lvl0_iterator = lvl0_iterator->next_page;
  }
  //A page was not found, we need to create a new page and increase its usage
  if (add_child_node_page_table(proc_conf, node_lvl1, page_type) < 0){
    return -1;
  }
  if (frame_info != 0){save_frame_data(node_lvl1->tail_page, frame_info, page_type);}
  if (page_type == SHARED_PAGE){
    //We only link shared pages this way due to their dynamic nature,
    //the other pages are linked using memory allocator
    //Could be improved, since its location is not ideal
    //it is possible to place directly in the memory api
    if (link_lvl1_table_direct(proc_conf, USERSPACE, node_lvl1->tail_page->index, 
        node_lvl1->tail_page->table)<0){
      return -1;
    }
  }
  node_lvl1->tail_page->usage++;
  return 0;
}

/**
 * @brief This method should be called only after all the usage values have been set properly 
 * and all of the tree like structures have set up
 * @note this method should only be called when we allocated memory when we declare the process
 * @param proc_conf The process that we will configure its memory
 * @param start_index used to indicate which lvl0 page table we will use, this page table will be pointed at
 * by the lvl1 page table
 * @param end_index 
 * @return int a negatif int value if the allocation was not successful and a positive value otherwise
 */
static int allocate_memory_final(process* proc_conf, int start_index,
       int end_index, void* data, int data_size, page_t page_type){
  //WARNING THIS FUNCTION SHOULD ONLY BE USED AT INITIALIZATION OTHERWISE USE
  //USE THE MANUAL METHODS.
  if (proc_conf->page_tables_lvl_1_list == NULL){
    return -1;
  }
  bool reverse_order = false;
  if (page_type == STACK_PAGE){
    reverse_order = true;
  }
  //These variables are used when we are writing code into the frames
  bool writing_data = false;
  void* data_pointer = NULL;
  int data_left = 0 ;
  if (data != NULL && data_size != 0){
    writing_data = true;
    data_pointer = data;
    data_left = data_size;
  }
  //We take pointer associated with the mega page
  //----------Lvl1------
  page_table_link_list_t* lvl_1_node = proc_conf->page_tables_lvl_1_list;
  page_table_entry* mega_table_entry;
  //----------Lvl0-------
  //We also need to iterate over the kilo page tables since 
  //they also vary in this loop
  page_table_link_list_t* lvl_0_node_iter = lvl_1_node->head_page;
  //Initially the pages are added in order taht is why we can do this
  while (lvl_0_node_iter != NULL){
    //We need to work with page that has the desired start index
    if (lvl_0_node_iter->index == start_index){break;}
    lvl_0_node_iter = lvl_0_node_iter->next_page;
  }
  if(lvl_0_node_iter == NULL){return -1;}
  //-----Values used in the iteration-----
  page_table* kilo_page_table;
  page_table_entry* kilo_table_entry;
  int kilo_page_usage;
  int iter_mega = 0;
  int iter_mega_limit;
  if (reverse_order){
    iter_mega_limit = start_index - end_index;
  } else{
    iter_mega_limit = end_index - start_index;
  }
  debug_print_memory("start_index %d - %d\n", start_index, end_index);
  int mega_usage_iter;
  while(iter_mega < iter_mega_limit){
    //We add a link from the level 1 page table to level 0 page table
    //
    //     |-------|   |-------|      |-------|
    //     |       |   |       |      |       |
    //     |       |   |       | ---> |       |
    //     |       |   |       |      |       |
    //     |-------|   |-------|      |-------|
    if (reverse_order){mega_usage_iter = start_index - iter_mega;}
    else{mega_usage_iter = iter_mega + start_index;}
    debug_print_memory("mega_usage_iter %d\n", mega_usage_iter);
    mega_table_entry = lvl_1_node->table->pte_list+mega_usage_iter;
    assert(lvl_0_node_iter->index == mega_usage_iter);
    kilo_page_table = lvl_0_node_iter->table;
    if (kilo_page_table == NULL){ return -1;}
    //Making the lvl1 table point to the lvl0 table
    configure_page_entry(mega_table_entry,
              (long unsigned int) kilo_page_table, 
              false,false,false,//No R,W,X 
              true,
              KILO);
    #ifdef PTE_PAGES_DEBUG
      print_memory_no_arg("-- Mega page pte --\n");
      print_pte(mega_table_entry);
    #endif
    kilo_page_usage = lvl_0_node_iter->usage;
    //Now we allocated memory to every node in kilo page table 
    for (unsigned int kilo_table_usage = 0; kilo_table_usage < kilo_page_usage; kilo_table_usage++){
      int index_kilo = 0;
      if (reverse_order){
        index_kilo = (PT_SIZE-1-kilo_table_usage);
      } else {index_kilo = kilo_table_usage;}
      kilo_table_entry = lvl_0_node_iter->table->pte_list+index_kilo;
      //Final page level page must in the read/write/exec mode
      debug_print_memory("mega pte index = %d; index_kilo = %d\n", 
                mega_usage_iter, index_kilo);
      void* frame_pointer = get_frame();
      if (frame_pointer == NULL){return -1;}
      memset(frame_pointer, 0, FRAME_SIZE);
      if (writing_data){
        if (data_left<0){
          //We have written all of the data that we need to write and 
          //will go back to setting the data in the frame to zero using the memset 
          //method  
          writing_data = false;
        }else{
          debug_print_memory("Writing data in the frame %d, first value in the data pointer %p\n",
                  data_left, data_pointer);
          memcpy(frame_pointer, data_pointer, FRAME_SIZE);
          data_pointer = (void*)((long) data_pointer + FRAME_SIZE);
          data_left-=FRAME_SIZE;
        }
      }
      debug_print_memory("Created frame add %p\n", frame_pointer);  
      configure_page_entry(kilo_table_entry,
            (long unsigned int )frame_pointer, 
            true, true, writing_data,
            //R,W, and writing_data is active we are writing data, 
            //making the code executable
            true, //This data will be used by the user mode
            KILO);
      #ifdef PTE_PAGES_DEBUG
        print_memory_no_arg("-- Kilo page pte --\n");
        print_pte(kilo_table_entry);
      #endif
    }
    if (reverse_order){
      lvl_0_node_iter = lvl_0_node_iter->previous_page;
    } else{
      lvl_0_node_iter = lvl_0_node_iter->next_page;
    }
    iter_mega++;
  }
  return 0;
}

static int process_frames_alloc(process* process_conf){
  //----------------------------Allocating space -------------------
  debug_print_memory("Allocating space for the process %s // %d \n",
       process_conf->process_name, process_conf->pid);
  //We allocate space for the code
  //At this point we need to copy the code into the frames that were allocated 
  #ifdef USER_PROCESSES_ON
    int code_size = (int) ((long) process_conf->app_pointer->end - (long) process_conf->app_pointer->start);
    //TODO FIX THIS AND A PERMANANT FIX
    //This is used because we need additional space for bss
    //and some other efl related directories
    // page_table_link_list_t* lvl_1_node = process_conf->page_tables_lvl_1_list;
    // lvl_1_node->head_page->usage+= 20;
    // process_conf->mem_info.code_usage+= 20;
    if (allocate_memory_final(process_conf,
              STACK_CODE_SPACE_START, 
              STACK_CODE_SPACE_START +process_conf->page_tables_lvl_1_list->code_usage,
              process_conf->app_pointer->start,
              code_size, CODE_PAGE) < 0){
      print_memory_no_arg("problem with final memory allocator -> code\n");
      return -1;
    }
    if (allocate_memory_final(process_conf,
              STACK_CODE_SPACE_END, 
              STACK_CODE_SPACE_END - process_conf->page_tables_lvl_1_list->stack_usage,
              0,
              0, STACK_PAGE) < 0){
      print_memory_no_arg("problem with final memory allocator -> stack\n");
      return -1;
    }
  #endif
  #ifdef KERNEL_PROCESSES_ON
      if (allocate_memory_final(process_conf,
              STACK_CODE_SPACE_START, 
              STACK_CODE_SPACE_START +process_conf->page_tables_lvl_1_list->stack_usage,
              NULL,
              0) < 0){
      print_memory_no_arg("problem with final memory allocator -> stack\n");
      return -1;
    }
  #endif
  print_memory_no_arg("Stack and code memory has been allocated\n");
  //We allocate space for the heap
  if (allocate_memory_final(process_conf,
    HEAP_SPACE_START,
    HEAP_SPACE_START +process_conf->page_tables_lvl_1_list->heap_usage,
    NULL , 
    0, HEAP_PAGE) < 0){
    print_memory_no_arg("problem with final memory allocator -> heap\n");
    return -1;
  }
  print_memory_no_arg("Heap memory has been allocated\n");
  return 0;
}

/**
 * @brief This method will reserve the space for the process using the parameters given and functions arguments
 * this fonction will not allocated the needed memory it will just change the indicies so that are well configured
 * when we allocate memory 
 * @param process_conf the process that we wish to configure
 * @param temp_code_size the size of the code 
 * @param heap_size the size of the heap
 * @param size_stack the size of the stack
 * @return 0 if successfull and -1 otherwise
 */
static int add_frames_process(process *process_conf, int temp_code_size, int size_stack, int heap_size){
  debug_print_memory("Reserving space for process code size = %d heap size = %d stack size = %d \n",
      temp_code_size, heap_size, size_stack);
  #ifdef USER_PROCESSES_ON
  //We add process' code
    while(temp_code_size > 0){
      if (add_frame_to_process(process_conf, CODE_PAGE, 0)<0){
        print_memory_no_arg("problem with memory allocator :  code space allocator\n");
        return -1;
      }
      temp_code_size -= FRAME_SIZE;
    }
  #endif 
  //We add stack' memory
  while(size_stack > 0){
    if (add_frame_to_process(process_conf, STACK_PAGE, 0)<0){
      print_memory_no_arg("problem with memory allocator :  stack allocator\n");
      return -1;
    }
    size_stack -= FRAME_SIZE;
  }
  //We associate the necessary frames and the page tables for the heap  
  while(heap_size > 0){
    if (add_frame_to_process(process_conf, HEAP_PAGE, 0)<0){
      print_memory_no_arg("problem with memory allocator :  frame allocator heap\n");
      return -1;
    }
    heap_size -= FRAME_SIZE;
  }
  return 0;
}

/**
 * @brief This function copies the basic lvl2 page table for the 
 * kernel and adds a new link to the newly created page. This link 
 * will connect the lvl2 page to a new lvl1 page and it will configure 
 * the basic memory locations of the lvl1 page dividing it into stack 
 * heap and shared pages
 * @param process_conf the process that we wish to configure
 * @return int status 
 */
static int init_mem_proc(process* process_conf){
  //----------------------LEVEL 2 CONF-------------------
  page_table* user_page_table_level_2 = create_page_table();
  if (user_page_table_level_2 == NULL){
    return -1;
  }
  print_memory_no_arg("--------------Starting Process Memory allocation-------------\n");
  debug_print_memory("Lvl2 address %p for process name %s // pid : %d \n",
       user_page_table_level_2, process_conf->process_name, process_conf->pid);
  process_conf->page_table_level_2 = user_page_table_level_2;
  //We copy the kernel page table in order to have the base directory 
  //that possess the links to the kernel space and memory, and the display locations and 
  //so on
  memcpy((void*) user_page_table_level_2, (void *) kernel_base_page_table, FRAME_SIZE);
  
  //-----------------------LEVEL 1/LEVEL 2 LINK-------------------
  page_table* user_page_table_level_1 = create_page_table();
  if (user_page_table_level_1 == NULL){return -1;}
  debug_print_memory("Lvl1 address %p for process name : %s // pid : %d \n",
      user_page_table_level_1, process_conf->process_name, process_conf->pid);

  //We create in here the only page table that will exist at first level since we choose  to limit virtual 
  //space in this os to one gb.
  //this page table will be pointed 
  //at by the second pte in the lvl 2 page table
  if (configure_page_table_linked_list_entry(
    &process_conf->page_tables_lvl_1_list,
    user_page_table_level_1,
    NULL,NULL,NULL,NULL,NULL, //No attachements
    NONE_PAGE,//This value is not relevant for this page table node
    USERSPACE //The user gigapage index is equal to one
    )<0){return -1;}
  //Make level 2 page table point to level 1 page table in the satp chain
  // 
  //     |-------|      |-------|      |-------|   
  //     |       |      |       |      |       |
  //     |       | ---> |       |      |       |   
  //     |       |      |       |      |       |
  //     |-------|      |-------|      |-------|
  configure_page_entry(user_page_table_level_2->pte_list+USERSPACE,
          (long unsigned int )user_page_table_level_1,
          false, false, false,//No R,W,X for this page
          true,//Will be used in user mode
          KILO);//Size of the page
  #ifdef PTE_PAGES_DEBUG
    // debug_print_memory("-----Second level pte kernel/process directory when working with process : %d \n",process_conf->pid);
    // print_pte(user_page_table_level_2->pte_list+USERSPACE);
  #endif
  return 0;
}



int process_memory_allocator(process* process_conf, unsigned long size){
  //We initiate the lvl2 page and we link to the only lvl1 page that will use
  //this page will be linked to lvl0 pages that contain 2Mb of data
  if (init_mem_proc(process_conf)<0){
    return -1;
  }
  //----------------------------Reserving space -------------------
  //The current value of the heap size is set so that we can have at least one frame
  int heap_size = 1;
  debug_print_memory("Reserving space for the process %s // %d \n",
       process_conf->process_name, process_conf->pid);
  #ifdef USER_PROCESSES_ON
    if (process_conf->app_pointer == NULL){return -1;}
    //We reserve the space for the stack and heap and also the code
    //which will be copied from else where
    int code_size = (int) ((long) process_conf->app_pointer->end - (long) process_conf->app_pointer->start);
    debug_print_memory("Code needs to be added to the process // code size =  %d \n", code_size);
    //The + FRAME SIZE is used as a temporary measure because
    //some elf headers are not added to the code size and we need to find 
    //a way to compute their size
    if (add_frames_process(process_conf, code_size, size, heap_size)<0){
      return -1;
    };

  #endif
  #ifdef KERNEL_PROCESSES_ON
    if (add_frames_process(process_conf, 0, size, heap_size)<0){
      return -1;
    };
  #endif
  //After setting up all of the frames in the init process we 
  //finally allocate them and reserve the necessary memory spaces
  if (process_frames_alloc(process_conf)<0){
    return -1;
  }
  return 0;
}

/**
 * @brief Computes the address that page table entry points to 
 * @param pte page table entry that we will apply the action to 
 * @return void* the address that the pte points to 
 */
static void* find_pte_adress(page_table_entry* pte){
  return (void*)((long) pte->ppn2*GIGA_SIZE+pte->ppn1*MEGA_SIZE+pte->ppn0*KILO_SIZE);
}

static int free_frames_indexed(page_table* table, int start_index, int end_index){
  if (table ==NULL){
    return -1;
  }
  page_table_entry* pte_free;
  for (unsigned int pte_index = start_index; pte_index < end_index; pte_index++){
    pte_free = table->pte_list+pte_index;
    // print_pte(pte_free);
    release_frame(find_pte_adress(pte_free));
  }
  return 0;
}

int free_frames_indexed_inverse(page_table* table, int start_index, int end_index){
  if (table ==NULL){
    return -1;
  }
  page_table_entry* pte_free;
  for (unsigned int pte_index = start_index; pte_index > end_index; pte_index--){
    pte_free = table->pte_list+pte_index;
    // print_pte(pte_free);
    release_frame(find_pte_adress(pte_free));
  }
  return 0;
}

int free_frames_page_table(page_table_link_list_t* page_table_link){
  //We take pointer associated with the mega page
  //----------Lvl0-------
  if(page_table_link == NULL){
    return -1;
  }
  //We check the page type(different treatement between lvl1 and lvl0 pages)
  //If all of the below values are null then the page is a static(not a shared page) lvl0 page
  if (page_table_link->stack_usage == 0 &&
      page_table_link->heap_usage ==0 &&
      page_table_link->shared_memory_usage == 0 && 
      page_table_link->code_usage == 0){
    //Lvl0 pages
    debug_print_memory("Freeing lvl0 page table index = %d // usage = %d \n", page_table_link->index, page_table_link->usage);
    //Shared pages are freed differently
    if (page_table_link->index >= SHARED_MEMORY_START){
      return 0;
    }
    print_memory_no_arg("-------------------freeing page-------------");
    //static lvl0 pages
    if (page_table_link->page_type == STACK_PAGE){
      free_frames_indexed_inverse(page_table_link->table, PT_SIZE-1, PT_SIZE-1-page_table_link->usage);
    }else{
      free_frames_indexed(page_table_link->table, 0,page_table_link->usage);
    }
  }else{
    debug_print_memory("Freeing lvl1 page table index %d \n", page_table_link->index);
    //lvl1 pages
    free_frames_indexed(page_table_link->table
    ,STACK_CODE_SPACE_START
    ,STACK_CODE_SPACE_START+ page_table_link->code_usage);

    free_frames_indexed_inverse(page_table_link->table
    ,STACK_CODE_SPACE_END
    ,STACK_CODE_SPACE_END - page_table_link->stack_usage);

    free_frames_indexed(page_table_link->table
    ,HEAP_SPACE_START
    ,HEAP_SPACE_START+ page_table_link->heap_usage);

    free_frames_indexed(page_table_link->table
    ,SHARED_MEMORY_START
    ,SHARED_MEMORY_START+ page_table_link->shared_memory_usage);
  }
  return 0;
}

int free_fs_proc(process* proc, del_t del_type){
  if (close_all_files(proc)<0){
    return -1;
  }
  if (del_type != DELETE_ALL){
    return 0;
  }
  if (proc->root_dir.dir_name != 0){
    free(proc->root_dir.dir_name);
  }
  if (proc->cur_dir.dir_name != 0){
    free(proc->cur_dir.dir_name);
  }
  return 0;
}

int free_process_memory(process* proc, del_t del_type){
  print_memory_no_arg("--------------Free process memory-------------\n");
  if (proc == NULL){
    return -1;
  }
  #ifdef VIRTMACHINE
    if (free_fs_proc(proc, del_type)<0){
      return -1;
    }
  #endif
  proc_mang_g.nb_proc_running--;
  debug_print_memory("--------Inside free_process_memory, current process: %s\n", getname());
  debug_print_memory("--------Freeing memory for the process/ id -> %d -------- : %s\n", 
            proc->pid, proc->process_name);
  //We start by removing the shared pages
  debug_print_memory("--------Releasing shared frames for the process/ id -> %d -------- : %s\n",
              proc->pid, proc->process_name); 
  
  if (proc->proc_shared_hash_table != NULL){
    shared_pages_proc_t* shared_iter = proc->shared_pages->head_shared_page;
    shared_pages_proc_t* shared_iter_prev = proc->shared_pages->head_shared_page;
    while (shared_iter!=NULL){
      shared_iter = shared_iter->next_shared_page;
      set_custom_release_process(proc);
      debug_print_memory("--------Custom shm release / id -> %d -------- : %s\n", proc->pid, shared_iter_prev->key); 
      shm_release(shared_iter_prev->key);
      set_custom_release_process(NULL);
      shared_iter_prev = shared_iter;
    }
    // //We need to also the free the holes that were left by the previous releases
    released_pages_t* released_iter = proc->released_pages_list;
    released_pages_t* released_iter_prev = proc->released_pages_list;
    while (released_iter != NULL){
      released_iter = released_iter->next_released_page;
      free(released_iter_prev);
    }
    if (del_type != DELETE_ALL){
      hash_destroy(proc->proc_shared_hash_table);
      free(proc->proc_shared_hash_table);
    }
  }
  debug_print_memory("--------Shared frames released for the process/ id -> %d -------- : %s\n",
            proc->pid, proc->process_name); 
  //We remove static pages first
  if (proc->page_table_level_2 != NULL){
    //We start by clearing all the lvl0 frames and then we free 
    //all of the level 0 tables and finaly we free the frame that holds the lvl1 table
    //and the level2 table
    page_table_link_list_t* lvl0_iter = proc->page_tables_lvl_1_list->head_page;
    page_table_link_list_t* lvl0_iter_prev = proc->page_tables_lvl_1_list->head_page;
    while(lvl0_iter != NULL){
      lvl0_iter = lvl0_iter->next_page;
      free_frames_page_table(lvl0_iter_prev);
      free(lvl0_iter_prev);
      lvl0_iter_prev = lvl0_iter; 
    }
    free_frames_page_table(proc->page_tables_lvl_1_list); //We only free one page
    release_frame(proc->page_tables_lvl_1_list->table);
    if (del_type == DELETE_ALL){
      release_frame(proc->page_table_level_2);
      release_frame(proc->sscratch_frame);
    }
    free(proc->page_tables_lvl_1_list);
  }
  if (del_type != DELETE_ALL){
    return 0;
  }
  //We remove share page and all the memory associated to them
  if (hash_del(get_process_hash_table(),
         cast_int_to_pointer(proc->pid)) < 0) {
    return -1;
  }
  if (proc->context_process != NULL){
    free(proc->context_process);
    proc->context_process = 0;
  }
  if (proc->process_name != NULL){
    free(proc->process_name);
    proc->process_name = 0;
  }
  debug_print_memory("free_process_memory ENDED, current process: %s\n", getname());
  save_pid(proc->pid);
  free(proc);
  proc = 0;
  return 0;
}


int copy_process_memory(process* new_proc, process* old_proc){
  //We initiate the basic structure for the memory setup
  if (init_mem_proc(new_proc)<0){
    return -1;
  }
  //We create the necessary links for pages and we allocate them
  if (add_frames_process(new_proc, 
        FRAME_SIZE*old_proc->mem_info.code_usage,
        FRAME_SIZE*old_proc->mem_info.stack_usage,
        FRAME_SIZE*old_proc->mem_info.heap_usage)<0){
    return -1;
  }
  // printf("code_usage dest %d, src %d \n", new_proc->mem_info.code_usage,
  //              old_proc->mem_info.code_usage);
  // printf("stack_usage dest %d, src %d \n", new_proc->mem_info.stack_usage,
  //              old_proc->mem_info.stack_usage);
  // printf("heap_usage dest %d, src %d \n", new_proc->mem_info.heap_usage,
  //              old_proc->mem_info.heap_usage);
  
  assert(new_proc->mem_info.code_usage == old_proc->mem_info.code_usage);
  assert(new_proc->mem_info.stack_usage == old_proc->mem_info.stack_usage);
  assert(new_proc->mem_info.heap_usage == old_proc->mem_info.heap_usage);
  new_proc->mem_info.sbrk_pointer = old_proc->mem_info.sbrk_pointer;
  new_proc->mem_info.start_heap_add = old_proc->mem_info.start_heap_add;
  
  // page_table_link_list_t* lvl0_parent = old_proc->page_tables_lvl_1_list->head_page;
  // while(lvl0_parent != NULL){
  //   debug_print_memory("[]Parent lvl0 page, page usage %d, index %d, type = %d\n",
  //       lvl0_parent->usage, lvl0_parent->index, lvl0_parent->page_type);
  //  page_table_link_list_t* lvl0_src = hash_get(old_proc->page_tables_lvl_1_list->link_index_table,
  //                            (void*)((uint64_t)lvl0_parent->index), NULL);
  //   debug_print_memory("[Src]parent by index, page usage %d, index %d type = %d\n",
  //       lvl0_src->usage, lvl0_src->index, lvl0_src->page_type);
  //   lvl0_parent = lvl0_parent->next_page;
  // }

  page_table_link_list_t* lvl0_iter = new_proc->page_tables_lvl_1_list->head_page;
  while(lvl0_iter != NULL){
    debug_print_memory("[Dest]Copying lvl0 page, page usage %d, index %d, type = %d\n",
        lvl0_iter->usage, lvl0_iter->index, lvl0_iter->page_type);
    bool reverse_order = false;
    bool exec_perm = false;
    if (lvl0_iter->page_type == STACK_PAGE){
      reverse_order = true;
    } 
    if (lvl0_iter->page_type == CODE_PAGE){
      exec_perm = true;
    }
    (void)exec_perm;
    page_table_link_list_t* lvl0_src = 0;
    page_table_link_list_t* lvl0_parent = old_proc->page_tables_lvl_1_list->head_page;
    while(lvl0_parent != NULL){
      if (lvl0_parent->index == lvl0_iter->index){
        lvl0_src = lvl0_parent;
        break;
      }
      lvl0_parent = lvl0_parent->next_page;
    }
    // page_table_link_list_t* lvl0_src = 
    // hash_get(old_proc->page_tables_lvl_1_list->link_index_table,
                            //  (void*)((uint64_t)lvl0_iter->index), NULL);
    debug_print_memory("[Src]Copying lvl0 page, page usage %d, index %d type = %d\n",
        lvl0_src->usage, lvl0_src->index, lvl0_src->page_type);
    if (lvl0_src == NULL){
      assert(0);
      return -1;
    }
    //We make sure everything is well configured
    assert(lvl0_src->usage == lvl0_iter->usage);
    page_table_entry* mega_entry = new_proc->page_tables_lvl_1_list->table->pte_list+(lvl0_iter->index);
    configure_page_entry(mega_entry,
          (long unsigned int) lvl0_iter->table, 
          false,false,false,//No R,W,X 
          true,
          KILO);
    for(int tab_iter = 0; tab_iter<lvl0_iter->usage; tab_iter++){
      void* frame_pointer = get_frame();
      if (frame_pointer == 0){return -1;}
      memset(frame_pointer, 0, FRAME_SIZE);
      int index_kilo = 0;
      if (reverse_order){
        index_kilo = (PT_SIZE-1-tab_iter);
      } else {index_kilo = tab_iter;}
      memcpy(frame_pointer, find_pte_adress(lvl0_src->table->pte_list+index_kilo),
                      FRAME_SIZE);
      // assert(memcmp(frame_pointer, find_app("test31")->start, FRAME_SIZE) == 0); 
      configure_page_entry(lvl0_iter->table->pte_list+index_kilo,
        (long unsigned int) frame_pointer, 
        true,true, exec_perm,
        true,
        KILO);
    }
    lvl0_iter = lvl0_iter->next_page;
  }
  if (old_proc->shared_pages != NULL){
    shared_pages_proc_t* shared_iter = old_proc->shared_pages->head_shared_page;
      while (shared_iter!=NULL){
        shm_acquire(new_proc, shared_iter->key);
        shared_iter = shared_iter->next_shared_page;
      }
  } else {new_proc->shared_pages = NULL;}
  return 0;
}

// static int free_top_pages(process* proc, uint32_t page_nb, page_t page_type){
//   if (proc == 0){
//     return -1;
//   }
//   page_table_link_list_t* lvl1 = proc->page_tables_lvl_1_list->head_page;
//   int start_index = 0;
//   int end_index = 0;
//   switch (page_type){
//     case CODE_PAGE:
//       start_index = STACK_CODE_SPACE_START;
//       end_index = STACK_CODE_SPACE_START + lvl1->code_usage;
//       break;
//     case STACK_PAGE:
//       start_index = STACK_CODE_SPACE_END;
//       end_index = STACK_CODE_SPACE_END - lvl1->code_usage;
//       break;
//     case HEAP_PAGE:
//       start_index = HEAP_SPACE_START;
//       end_index = HEAP_SPACE_START + lvl1->heap_usage;
//       break;
//     case SHARED_PAGE:
//       start_index = SHARED_MEMORY_START;
//       end_index = SHARED_MEMORY_START + lvl1->shared_memory_usage;
//       break;
//     default:
//       break;
//   }
//   lvl1->link_index_table
// }

static int add_and_allocate_frames(process* proc, uint32_t page_nb, page_t type){
  if (proc == 0 || page_nb > get_remaining_frames()){
    return -1;
  }
  // printf("Trying to add to mem to, proc pid = %d pid = %d, name = %s\n", proc->pid, getpid(), getname());
  for (uint64_t page_iter = 0; page_iter < page_nb; page_iter++){
    frame_loc frame;
    memset(&frame, 0,sizeof(frame_loc));
    if (add_frame_to_process(proc, type, &frame)<-1){
      return -1;
    }
    //New lvl0 table
    if (frame.lvl0_index == 0){
      //Works only for on lvl1 page
      assert(proc->page_tables_lvl_1_list->tail_page->page_type 
          == type);
      if (link_lvl1_table_direct(proc, USERSPACE, 
              proc->page_tables_lvl_1_list->tail_page->index, 
              proc->page_tables_lvl_1_list->tail_page->table)<0){
        return -1;
      }
    }
    debug_print_memory("Allo details frame lvl2_index = %d \n", frame.lvl2_index);
    debug_print_memory("Allo details frame lvl1_index = %d \n", frame.lvl1_index);
    debug_print_memory("Allo details frame lvl0_index = %d \n", frame.lvl0_index);
    debug_print_memory("Allo details frame page_table = %p \n", frame.page_table); 
    void* frame_pointer = get_frame();
    if (frame_pointer == NULL){return -1;}
    memset(frame_pointer, 0, FRAME_SIZE);
    configure_page_entry(frame.page_table->pte_list+frame.lvl0_index,
      (long unsigned int) frame_pointer, 
      true,true, false,
      true,
      KILO);
  }
  return 0;
}

void* get_first_stack_page(process* proc){
  page_table_link_list_t* lvl0_iter = proc->page_tables_lvl_1_list->head_page;
  while(lvl0_iter != NULL){
    if (lvl0_iter->page_type == STACK_PAGE 
        && lvl0_iter->index == STACK_CODE_SPACE_END){
      return find_pte_adress(lvl0_iter->table->pte_list+PT_SIZE-1); 
    }
    lvl0_iter = lvl0_iter->next_page;
  }
  return 0;
}

int check_expansion_mem(process* proc, struct trap_frame* frame){
  //We check if need to expand the stack
  {
    debug_print_memory("Checking if we need to expand the stack proc name %s\n", 
        proc->process_name);
    uint64_t sp_limit = (uint64_t)0x40000000 +
           FRAME_SIZE * proc->stack_shift;
    uint64_t current_sp = (uint64_t)frame->sp;
    uint64_t used_pages = (sp_limit-current_sp)/FRAME_SIZE + 1;
    long page_diff = used_pages - proc->mem_info.stack_usage;
    if (page_diff>0 && 
        (proc->mem_info.stack_usage + page_diff)<STACK_FRAME_SIZE*PT_SIZE){
      printf("Trying to add to stack size, pid = %d, name = %s\n", getpid(), getname());
      debug_print_memory("Expanding memory of proc : %s by %ld \n", 
        proc->process_name, page_diff);
      add_and_allocate_frames(proc, page_diff, STACK_PAGE);
    }
  }
  return -1;
}



void *sys_sbrk(long int increment){
  // printf("SBRK CALLED %ld \n", increment);
  process* proc = get_current_process();
  if (proc == 0){
    return 0;
  }
  // printf("sysbrk called arg = %ld \n", increment);
	char *current = (char *) proc->mem_info.sbrk_pointer;
  // printf("current = %p \n", proc->mem_info.sbrk_pointer);
  char *s = current;
	char *c = s + increment;
  uint64_t abs_inc = ABS(increment);
  uint64_t nb_pages = CUSTOM_MOD_DIV(abs_inc, FRAME_SIZE);
  if (increment>0){
    if (add_and_allocate_frames(proc, nb_pages, HEAP_PAGE)<0){
      PRINT_RED("SBRK FAILED\n");
      return ((void *) (-1));
    }
  }
	if (((uint64_t) c < (uint64_t) proc->mem_info.start_heap_add)) {
		/* We cannot grow the heap anymore */
    PRINT_RED("BAD\n");
    return ((void *) (-1));
	}
	/* The heap grown */
	proc->mem_info.sbrk_pointer = c;
  // printf("return current = %p \n", s);
  return s;
}
