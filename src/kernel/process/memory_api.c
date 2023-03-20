#include "process.h"
#include "process_memory.h"
#include "hash.h"
#include "stddef.h"
#include "string.h"
#include "mem.h"
#include "stdlib.h"
#include "helperfunc.h"
#include "stdio.h"
#include "memory_api.h"
#include <stdint.h>


uint64_t page_id_counter = 0; 
hash_t *shared_memory_hash_table =  NULL;

/**
 * @brief Get the shared page using the global shared pages hash table
 * 
 * @param page_string the char related to the page that we are looking for
 * @return shared_page_t* the address of a shared_page_t struct that holds information related to the page or 
 * NUll if it can't find the page that we are looking for
 */
shared_page_t* get_shared_page(char* page_string){
    shared_page_t* page_struct = ((shared_page_t*) hash_get(get_shared_pages_hash_table(),
                                                 cast_char_star_into_pointer(page_string),
                                                NULL));
    return page_struct;
}



/**
 * @brief Get a shared page that is mapped in the process given as function argument.
 * if the process is null then we return null, and if the process table is null we return null
 * and if the share page is null we return null 
 * @param proc the process at which we lookup the shared page
 * @param page_string page identifier
 * @return shared_pages_proc_t* NUll if the page cannot be found or if there were an error 
 * or the shared_pages_proc_t which indicates information about the page
 */
shared_pages_proc_t* get_shared_page_proc(process* proc, char* page_string){
    if (proc==NULL){
        return NULL;
    }
    hash_t* table_proc = proc->proc_shared_hash_table;
    if (table_proc ==NULL){
        return NULL;
    }
    shared_pages_proc_t* page_struct = ((shared_pages_proc_t*) hash_get(table_proc,
                                                 cast_char_star_into_pointer(page_string),
                                                NULL));
    return page_struct;
}

void* map_address_for_process(   uint16_t lvl2_index,
   uint16_t lvl1_index,
   uint16_t lvl0_index){
    return (void*)((long) lvl2_index*GIGA_SIZE+lvl1_index*MEGA_SIZE+lvl0_index*KILO_SIZE);
}

int initialize_share_pages_table(){
    shared_memory_hash_table = (hash_t*) malloc(sizeof(hash_t));
    if (shared_memory_hash_table == NULL){
        return -1;
    }
    return 0;
}

int initialize_process_shared_hash_table(process* proc_conf){
    if (proc_conf == NULL){
        return -1;
    }
    proc_conf->proc_shared_hash_table = (hash_t*) malloc(sizeof(hash_t));
    if (proc_conf->proc_shared_hash_table == NULL){
        return -1;
    }
    return 0;
}


static shared_page_t* create_shared_page(char* key){
    void* page_frame = get_frame();
    shared_page_t* page_info = (shared_page_t*)malloc(sizeof(shared_page_t));
    page_info->page_address = page_frame;
    page_info->usage = 0;
    page_info->page_id = increment_shared_page_counter();
    if (hash_set(get_shared_pages_hash_table(), cast_char_star_into_pointer(key), page_info)<0){
        release_frame(page_frame);
        return NULL;
    }
    return page_info;
}

static int link_shared_page_to_process(process* proc_conf, shared_page_t* page_info, char* page_name){
    if (proc_conf == NULL || page_info == NULL || page_name == NULL || proc_conf->shared_pages ==NULL){
        return -1;
    }
    //------------We create the shared page node and add it to the process
    shared_pages_proc_t* shared_page_proc = (shared_pages_proc_t*) malloc(sizeof(shared_pages_proc_t));
    shared_page_proc->key = (char*) malloc(strlen(page_name));
    if (shared_page_proc->key == NULL){
        return -1;
    }
    strcpy(shared_page_proc->key, page_name);
    shared_page_proc->next_shared_page = NULL;
    printf("------\n string is equal to %s \n", shared_page_proc->key);
    if (shared_page_proc==NULL){
        return -1;
    }
    if (proc_conf->shared_pages->head_shared_page == NULL){
        proc_conf->shared_pages->head_shared_page = shared_page_proc;
        proc_conf->shared_pages->tail_shared_page = shared_page_proc;
    }
    else {
        proc_conf->shared_pages->tail_shared_page->next_shared_page = shared_page_proc;
        proc_conf->shared_pages->tail_shared_page = shared_page_proc;
    }
    //------------Attach the page to process----------
    //There are two possibilities in here 
    //Either we add a new page or we exploit a hole left by the other released page
    if (proc_conf->released_pages_list != NULL){
        //We found a released page location that we can exploit
        released_pages_t* released_page = proc_conf->released_pages_list;
        proc_conf->released_pages_list = proc_conf->released_pages_list->next_released_page;
        //The tail coorespands to the new shared page struct related to the process
        //that we will configure manually at this point
        proc_conf->shared_pages->tail_shared_page->lvl0_index = released_page->lvl0_index;
        proc_conf->shared_pages->tail_shared_page->lvl1_index = released_page->lvl1_index;
        proc_conf->shared_pages->tail_shared_page->lvl2_index = released_page->lvl2_index;
        proc_conf->shared_pages->tail_shared_page->page_table = released_page->page_table;
    }
    else{
        //We can't find any released pages so we create a new space 
        //This method will configure the shared page node and place the location that find in a table 
        //in it and it will also add the table to node this will allow us to easily configure the page later
        if (add_frame_to_process(proc_conf, SHARED_PAGE)<0){
            return -1;
        }
    }
    //We link the shared page to the hash table that maps keys to shared pages
     
    hash_set(proc_conf->proc_shared_hash_table,
         cast_char_star_into_pointer(page_name),
         proc_conf->shared_pages->tail_shared_page->page_table);
    
    //At this point we only need to map the page
    //We increase the usage of the shared page
    //(this method will be called when we create the page of when we acquire it)
    page_info->usage++;
    //The level 1 to level 0 linking get done automatically
    print_shared_page_node("shm_create",proc_conf->shared_pages->tail_shared_page);
    uint16_t lvl0_index = proc_conf->shared_pages->tail_shared_page->lvl0_index;
    page_table_entry* shared_page_entry =  proc_conf->shared_pages->tail_shared_page->page_table->pte_list+lvl0_index;
    configure_page_entry(shared_page_entry,
                            (long unsigned int) page_info->page_address, 
                            true,
                            true,
                            true,
                            false, KILO);
    return 0;
}

void shm_release(const char *key){
    print_memory_api_no_arg("Inside Frame release function\n");
    if (key ==NULL){
        return ;
    }
    char* key_no_c = (char*) key;
    //We check if the page exists or no
    shared_page_t* page_info = get_shared_page(key_no_c);
    if (page_info ==NULL){
        return;
    }
    process* current_proc = get_process_struct_of_pid(getpid());
    if (current_proc ==NULL){
        return;
    }
    //We check if the process even has mapped element (proven by the existance of a hash table)
    if (current_proc->proc_shared_hash_table == NULL){
        return;
    }
    //We check if the process is even sharing this page
    shared_pages_proc_t* proc_page_shared = get_shared_page_proc(current_proc, key_no_c);
    if (proc_page_shared == NULL){
        return;
    }
    //If we are here that mean the process is being shared by the process that we are dealing this
    //We must in this case remove the shared process, inform the process that there will be a hole in the shared pages
    //by exploiting the releasead pages struct

    //-----------------We start by creating a released page struct and we save related the hole---------
    released_pages_t* released_page_p = (released_pages_t*) malloc(sizeof(released_pages_t));
    released_page_p->lvl0_index = proc_page_shared->lvl0_index;
    released_page_p->lvl1_index = proc_page_shared->lvl1_index;
    released_page_p->lvl2_index = proc_page_shared->lvl2_index;
    released_page_p->page_table = proc_page_shared->page_table;
    if (current_proc->released_pages_list == NULL){
        released_page_p->next_released_page = NULL;
        current_proc->released_pages_list = released_page_p;
    }
    else{    
        released_page_p->next_released_page = current_proc->released_pages_list;
        current_proc->released_pages_list = released_page_p;
    }
    //----------------We now free shared_pages_proc_t from the current process---------------
    //We free the string first
    // free(proc_page_shared->key);//TODO CHECK IF THIS LEAKS MEMORY
    //We handle the linking of the process
    if (current_proc->shared_pages ==NULL){
        return;
    }
    printf("I am no longer in the free method \n");
    printf("%p\n",current_proc->shared_pages->tail_shared_page);
    printf("%p\n",current_proc->shared_pages->head_shared_page);
    //In this case the shared page is head and tail of the link wrapper
    if (proc_page_shared == current_proc->shared_pages->head_shared_page &&
        proc_page_shared == current_proc->shared_pages->tail_shared_page){
        //In this case we can free it directly
        free(proc_page_shared);
        current_proc->shared_pages->head_shared_page = NULL;
        current_proc->shared_pages->tail_shared_page = NULL;
    }
    else if (proc_page_shared == current_proc->shared_pages->head_shared_page){
        //We now check if it only equal to head 
        current_proc->shared_pages->head_shared_page = proc_page_shared->next_shared_page;
        free(proc_page_shared);
    }
    else if (proc_page_shared == current_proc->shared_pages->tail_shared_page){
        //We now check if it only equal to tail
        //In this case we have to through all of the list and reach the element behind this list
        //This problem can be solved if we have a double linked list TODO!!!
        shared_pages_proc_t* list_iter = current_proc->shared_pages->head_shared_page;
        //Should not cause problems if the list weel linked
        while(list_iter->next_shared_page != proc_page_shared){
            list_iter=list_iter->next_shared_page;
        }
        current_proc->shared_pages->tail_shared_page = list_iter;
        list_iter->next_shared_page = NULL;
        free(proc_page_shared);
    }
    else {
        //In this case the list node is in the middle 
        shared_pages_proc_t* list_iter = current_proc->shared_pages->head_shared_page;
        //Should not cause problems if the list weel linked
        while(list_iter->next_shared_page != proc_page_shared){
            list_iter=list_iter->next_shared_page;
        }
        list_iter->next_shared_page = proc_page_shared->next_shared_page;
        free(proc_page_shared);
    }
    //Now we just have to remove the memory mapping
    uint16_t lvl0_index = released_page_p->lvl0_index;
    page_table_entry* shared_page_entry =  released_page_p->page_table->pte_list+lvl0_index;
    set_invalid(shared_page_entry);
    //We remove the page from the shared pages hash table related to the process
    if (hash_del(current_proc->proc_shared_hash_table, cast_char_star_into_pointer(key_no_c))<0){
        return;
    }
    //We check if there any more users of the pages
    page_info->usage--;
    if (page_info->usage == 0){
        //If there are no more users we then:
        // -free the page information
        // -release the frame that was holding the process
        // -delete the process from the hash table
        release_frame(page_info->page_address);
        hash_del(get_shared_pages_hash_table(), cast_char_star_into_pointer(key_no_c));
        free(page_info);
    }
    print_memory_api_no_arg("Shared page has been released successfully\n");
    return;
}

/**
 * @brief Create a hash table and shared pages wrapper object if needed, this function will
 * do nothing if the process already has the elements that we want to create
 * @param current_proc the process the we will create the elements for 
 * @return a positive value if successful and negative value otherwise 
 */
static int create_hash_table_and_shared_pages_wrapper(process* current_proc){
    if (current_proc->proc_shared_hash_table == NULL){
        if (initialize_process_shared_hash_table(current_proc)<0){
            return -1;
        }
        if (hash_init_string(current_proc->proc_shared_hash_table)){
            return -1;
        }
    }
    if (current_proc->shared_pages == NULL){
        current_proc->shared_pages = (shared_pages_wrap_t*) malloc(sizeof(shared_pages_wrap_t));
        if (current_proc->shared_pages == NULL){
            return -1;
        }
        current_proc->shared_pages->head_shared_page = NULL;
        current_proc->shared_pages->tail_shared_page = NULL;
    }
    return 0;
}

void *shm_acquire(const char *key){
    print_memory_api_no_arg("Inside shm acquire function \n");
    if (key ==NULL){
        return NULL;
    }
    char* key_no_c = (char*) key;
    //We check if the page exists or no
    shared_page_t* page_info = get_shared_page(key_no_c);
    if  (page_info == NULL){
        return NULL;
    }
    process* current_proc = get_process_struct_of_pid(getpid());
    if (current_proc ==NULL){
        return NULL;
    }
    //We create the hash table
    if (create_hash_table_and_shared_pages_wrapper(current_proc)<0){
        return NULL;
    }
    //We check if the page is aleardy mapped in the current process or not
    if (get_shared_page_proc(current_proc, key_no_c) != NULL){
        return NULL;
    }
    if (link_shared_page_to_process(current_proc, page_info, key_no_c)<0){
        return NULL;
    }
    print_memory_api_no_arg("Aquired shared frame successfully \n");
    void* mapped_address =  map_address_for_process(
        current_proc->shared_pages->tail_shared_page->lvl2_index,
        current_proc->shared_pages->tail_shared_page->lvl1_index,
        current_proc->shared_pages->tail_shared_page->lvl0_index
    );
    return mapped_address;
}


void *shm_create(const char *key){
    print_memory_api_no_arg("In the shm create function \n");
    if (key ==NULL){
        return NULL;
    }
    char* key_no_c = (char*) key;
    //We check if the page exists 
    if (get_shared_page(key_no_c) !=NULL){
        //If the page exits we leave directly
        return NULL;
    }
    shared_page_t* page_info = create_shared_page(key_no_c);
    if( page_info == NULL){
        //No space for the frame or the malloc probably
        return NULL;
    }
    process* current_proc = get_process_struct_of_pid(getpid());
    if (current_proc ==NULL){
        //get pid is not working probably
        return NULL;
    }
    //We create he hash table and shared pages wrapper
    if (create_hash_table_and_shared_pages_wrapper(current_proc)<0){
        return NULL;
    }
    if (link_shared_page_to_process(current_proc, page_info, key_no_c)<0){
        return NULL;
    }
    print_memory_api_no_arg("Created shared page successfully \n");
    //The address returned must correspand to the address mapped to the process
    //The node taht was created is the tail_sahred page which holds the page that we are trying to 
    //allocate memory to
    void* mapped_address =  map_address_for_process(
        current_proc->shared_pages->tail_shared_page->lvl2_index,
        current_proc->shared_pages->tail_shared_page->lvl1_index,
        current_proc->shared_pages->tail_shared_page->lvl0_index
    );
    return mapped_address;
}