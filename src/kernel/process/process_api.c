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


// Hash table that associates to every pid the process struct associated to it
hash_t *pid_process_hash_table = NULL;
// Id of the process that is currently running this value will be changed dynamically by the scheduler
int current_running_process_pid = -1;
// Pid iterator that will be used to associate to every process a unique pid
int pid_iterator = 0;

/**
 * @brief 
 * 
 * @param link_to_configure thel ink that we will configure : allocated memory for it and attach the other paramaters to it
 * @param table page_table attached to list
 * @param page_tables_level_0_linkedlist child page table taht will be used to link level 1 andlevel 0
 * @param usage indicates the the number of entries that we will be created by default in page
 * @param next_page_link the next page table linked list node 
 * @return int a negatif value if an error occured and positf value if there were no problems
 */
int configure_page_table_linked_list_entry(page_table_link_list_t* link_to_configure, 
                                                page_table* table,
                                                page_table_link_list_t* page_tables_level_0_linkedlist,
                                                int usage,
                                                page_table_link_list_t* next_page_link){
    link_to_configure = (page_table_link_list_t*) malloc(sizeof(page_table_link_list_t));
    if (link_to_configure == NULL){
        return -1;
    }
    link_to_configure->table = table;
    link_to_configure->page_tables_level_0_linkedlist = page_tables_level_0_linkedlist;
    link_to_configure->usage = usage;
    link_to_configure->next_page_link = next_page_link;
    return 0;
}

/**
* @brief This function allocates memory for a process, it's current
* form remains very basic and does not follow the project specifications
* and it is only valid for a size that is less than then page size
* @param size corresponds to the size that we want to allocate
* @return the address of the page that we allocated
*/
void *process_memory_allocator(process* process_conf, unsigned long size){
    if (size>=GIGAPAGE_SIZE){
        return NULL;
    }
    //----------------------LEVEL 2-------
    page_table* user_page_table_level_2 = create_page_table();
    process_conf->page_table_level_2 = user_page_table_level_2;
    //We copy the lernel page table
    memcopy((void*) user_page_table_level_2, (void *) kernel_base_page_table, FRAME_SIZE);
    //-----------------------LEVEL 1/LEVEL 2 LINK-------------------
    page_table* user_page_table_level_1 = create_page_table();
    //We add a pointer in an pte in the second page table to the first page table
    configure_page_entry(user_page_table_level_2->pte_list+USERSPACE,//pte number
                    (long unsigned int )user_page_table_level_1, false, false, false, true, KILO);

    configure_page_table_linked_list_entry(
        process_conf->page_tables_level_1_linkedlist,
        user_page_table_level_1,
        NULL,
        0,
        NULL
        );
    //------------------LEVEL 1/LEVEL 0 LINK--------------------
    //In here we create the first frame that will be used for the stack
    page_table* user_page_table_level_0 = create_page_table();
    //The index of pte will depend on the usage of the page
    page_table_entry* mega_table_entry = user_page_table_level_1->pte_list+process_conf->page_tables_level_1_linkedlist->usage;
    configure_page_entry(mega_table_entry,
                    (long unsigned int )user_page_table_level_0, false, false, false, true, KILO);
    process_conf->page_tables_level_1_linkedlist->usage++;
    configure_page_table_linked_list_entry(
        process_conf->page_tables_level_1_linkedlist->page_tables_level_0_linkedlist,
        user_page_table_level_0,//table associated with the node
        NULL,//no child
        0,//usage of the page table
        NULL//the next pte in the page is null so the link must be null
        );
    page_table_entry* first_kilo_pte = user_page_table_level_0->pte_list+process_conf->page_tables_level_1_linkedlist->page_tables_level_0_linkedlist->usage;
    configure_page_entry(first_kilo_pte,
                    (long unsigned int )get_frame(), true, true, true, true, KILO);
    process_conf->page_tables_level_1_linkedlist->page_tables_level_0_linkedlist->usage++;

    return get_frame();
}

/**
 * @brief 
 * 
 * @param process_conf 
 * @return int 
 */
int add_frame_to_process(process* process_conf){
    //We check at this level that the process has been correctly set up
    if (process_conf->page_table_level_2 == NULL){
        return -1;
    }
    if (process_conf->page_tables_level_1_linkedlist == NULL){
        return -1;
    }
    if (process_conf->page_tables_level_1_linkedlist->page_tables_level_0_linkedlist == NULL){
        return -1;
    }
    page_table_link_list_t* level_1_iter = process_conf->page_tables_level_1_linkedlist;
    page_table_link_list_t* level_1_iter_prev = process_conf->page_tables_level_1_linkedlist;
    while (true){
        if (level_1_iter == NULL && ){
            page_table* user_page_table_level_0 = create_page_table();
            //The index of pte will depend on the usage of the page
            page_table_entry* mega_table_entry = user_page_table_level_1->pte_list+process_conf->page_tables_level_1_linkedlist->usage;
            configure_page_entry(mega_table_entry,
                            (long unsigned int )user_page_table_level_0, false, false, false, true, KILO);
            process_conf->page_tables_level_1_linkedlist->usage++;
            configure_page_table_linked_list_entry(
                process_conf->page_tables_level_1_linkedlist->page_tables_level_0_linkedlist,
                user_page_table_level_0,//table associated with the node
                NULL,//no child
                0,//usage of the page table
                NULL//the next pte in the page is null so the link must be null
                );
            page_table_entry* first_kilo_pte = user_page_table_level_0->pte_list+process_conf->page_tables_level_1_linkedlist->page_tables_level_0_linkedlist->usage;
            configure_page_entry(first_kilo_pte,
                            (long unsigned int )get_frame(), true, true, true, true, KILO);
            process_conf->page_tables_level_1_linkedlist->page_tables_level_0_linkedlist->usage++;
            level_1_iter = 
        }
        if (level_1_iter->page_tables_level_0_linkedlist->usage == PT_SIZE){
            level_1_iter_prev = level_1_iter;
            level_1_iter = level_1_iter->next_page_link;
            //If the value of this is NULL, then we must create a new link in linked list
            //But we also have to check if it is possible to create this new link  
        }

    }

}


int setpid(int new_pid){
    // We start by checking that the process exists
    process* process_pid = ((process*) hash_get(get_process_hash_table(), cast_int_to_pointer(new_pid), NULL));
    if (process_pid == NULL){
       return -1;
    }
    current_running_process_pid = new_pid;
    return new_pid;
}


int getpid(void){
   return current_running_process_pid;
}


int leave_queue_process_if_needed(process* process_to_leave){
    if(process_to_leave == NULL){
        return -1;
    }
    debug_print_exit_m("\nTrying to remove %s from a queue id %d process state  = %d\n", 
                process_to_leave->process_name,
                process_to_leave->pid,
                process_to_leave->state);   
    switch(process_to_leave->state) {
        case ACTIVATABLE:
            delete_process_from_queue_wrapper(process_to_leave, ACTIVATABLE_QUEUE);
            break;
        case ASLEEP:
            delete_process_from_queue_wrapper(process_to_leave, ASLEEP_QUEUE);
            break;
        default:
            break;
    }
    return 0;
}

int getprio(int pid){
   process* process_pid = ((process*) hash_get(get_process_hash_table(), cast_int_to_pointer(pid), NULL));
   if (process_pid == NULL){
       return -1;
   }
   if (validate_action_process_valid(process_pid) < 0){
       return -1;
   }
   return process_pid->prio;
}


int check_if_new_prio_is_higher_and_call_scheduler(int newprio, bool current_or_queue, int prio_to_compare_queue){
    if (!(newprio <= MAXPRIO && newprio >= MINPRIO)){
       return -1;
    }
    if (current_or_queue == true){
        int currently_running_prio = getprio(getpid());
        if (currently_running_prio < 0){
            return -1;
        }
        if (currently_running_prio < newprio){
            scheduler();
        }
    }
    else{
        if (prio_to_compare_queue < 0){
            return -1;
        }
        if (prio_to_compare_queue > newprio){
            scheduler();
        }
    }
    return 0;
}

int chprio(int pid, int newprio){
   process* process_pid = ((process*) hash_get(get_process_hash_table(), cast_int_to_pointer(pid), NULL));
   if (process_pid == NULL){
       return -1;
   }
   if (!(newprio <= MAXPRIO && newprio >= MINPRIO)){
       return -1;
   }
   if (validate_action_process_valid(process_pid) < 0){
       return -1;
   }
   int old_prio = process_pid->prio;
   process_pid->prio = newprio;
   process* head_of_queue = get_peek_element_queue_wrapper(ACTIVATABLE_QUEUE);
   if (head_of_queue == NULL){
        //No other activatable process we go back to executing this process
        return old_prio;
   }
   check_if_new_prio_is_higher_and_call_scheduler(newprio, false, head_of_queue->prio);
   // If the process is placed in an execution queue, it must be replaced within that queue
   return old_prio;
}


/**
* @brief this function deletes the process from the hash table
* and frees the data structure of the process
* @param process_to_free the process that we will free that must be a zombie
* @returns the value 0 if the the operation was a success and a negative value otherwise
*/
static int free_child_zombie_process(process* process_to_free){
    if (process_to_free==NULL){
        return -1;
    }
    if (process_to_free->state != ZOMBIE){
        return -1;
    }
    debug_print_exit_m("\nTrying to free with pid = %d and name = %s \n",process_to_free->pid, process_to_free->process_name);
    if (process_to_free->pid == getpid()){
        //If we exit the process from the process it self meaning exit process was called
        //In this case the process will be removed by the scheduler
        process_to_free->state = KILLED;
    }
    else{
        debug_print_exit_m("\nFreeing the process with pid = %d and name = %s \n",process_to_free->pid, process_to_free->process_name);
        //If we killed the process using the kill method then we can removea it directly
        if (hash_del(get_process_hash_table(), cast_int_to_pointer(process_to_free->pid))<0){
            //Something went wrong ....
            return -1;
        }
        free(process_to_free);
        process_to_free = 0;
    }
    return 0;
}


/**
* @brief Free a process and removes it from the parent's children
* @note This function must be called on a process that has a parent
* @param process_to_free the process to free
* @param before_process the siblings that comes before the process, might have the same value
* @returns the value 0 if the the operation was a success and a negative value otherwise
*/
static int free_process_arg_and_fix_tree_link(process* process_to_free, process* before_process){
   if (process_to_free->parent == NULL){
       return -1;
   }
   if (process_to_free == NULL || before_process ==NULL){
       return -1;
   }
   if (process_to_free->parent->children_head == NULL ||
       process_to_free->parent->children_tail == NULL){
       return -1;
   }
   if (process_to_free == NULL || before_process == NULL){
       return -1;
   }
   if (process_to_free == process_to_free->parent->children_head &&
       process_to_free == process_to_free->parent->children_tail ){
       process_to_free->parent->children_tail=NULL;
       process_to_free->parent->children_head=NULL;
       return free_child_zombie_process(process_to_free);
   }
   else if (process_to_free == process_to_free->parent->children_head){
       process_to_free->parent->children_head = process_to_free->parent->children_head->next_sibling;
       return free_child_zombie_process(process_to_free);
   }
   else if (process_to_free == process_to_free->parent->children_tail){
       before_process->next_sibling = NULL;
       process_to_free->parent->children_tail = before_process;
       return free_child_zombie_process(process_to_free);
   }
   else{  
       before_process->next_sibling = process_to_free->next_sibling;
       return free_child_zombie_process(process_to_free);
   }      
}

/**
* @brief this method is called when we exit a process, it will make the
* children of the process that are still alive orphans and it will free all the zombie
* children
* @param parent_process the process that will do apply the action on to
* @returns the value 0 if the the operation was a success and a negative value otherwise
* @note this method will return 0 if the process does not have any children
*/
static int make_children_orphans_and_kill_zombies(process* parent_process){
    if (parent_process == NULL){
        return -1;
    }
    if (parent_process->children_head == NULL && parent_process->children_tail == NULL){
        return 0;
    }
    else{
        process* temp_process = parent_process->children_head;
        while (temp_process != NULL){
            // We free the process in this casse
            if (temp_process->state == ZOMBIE){
                //  We don't need to the fix the links of the elements because their relationship
                //  is not relevant after this call
                process * process_to_free = temp_process;
                temp_process = temp_process->next_sibling;
                if (free_child_zombie_process(process_to_free)<0){
                    return -1;
                }
                continue;
            }
            temp_process=temp_process->next_sibling;
        }
    }
   return 0;
}



/**
* @brief called when we exit a process, it will transform the currently  running process or a custom 
* process into a zombie if the parent is still alive or it will kill the process if the parent is dead
* @param  curent_or_custom indicates if we want to apply the function to the current process or a custom process.
* True for current, false for custom
* @param pid the id of the process that we will apply the action on if we choose to work with a custom process 
* @returns the value 0 if the the operation was a success and a negative value otherwise
*/
static int turn_current_process_into_a_zombie_or_kill_it(bool curent_or_custom, int pid){
    process* current_process = NULL;
    if (curent_or_custom == true){
        //We apply the kill process to the currently running process 
        current_process = ((process*) hash_get(get_process_hash_table(), cast_int_to_pointer(getpid()), NULL));
    }
    else{
        //We do the action on a custom process specified using the pid given in the function argument 
        current_process = ((process*) hash_get(get_process_hash_table(), cast_int_to_pointer(pid), NULL));   
    }
    if (current_process == NULL){
        return -1;
    }
    if (make_children_orphans_and_kill_zombies(current_process)<0){
        return -1;
    }
    current_process->state = ZOMBIE;
    if (current_process->parent == NULL){
        return free_child_zombie_process(current_process);
    }
    else{
        //If the parent is waiting for a child we wake it and see
        //if the child that left correspand to that child that the parent 
        //was waiting for.  
        if (current_process->parent->state == BLOCKEDWAITCHILD){
            current_process->parent->state = ACTIVATABLE;    
            add_process_to_queue_wrapper(current_process->parent, ACTIVATABLE_QUEUE);
        }
    }
    return 0;
}




void exit_process(int retval){
    debug_print_exit_m("I am in exit method with argument/ temp pid = %d \n", retval);
    if (validate_action_process_valid(get_process_struct_of_pid(getpid())) < 0){
        // Something went terribly wrong if we are in here
        exit(-1);
    }
    if (turn_current_process_into_a_zombie_or_kill_it(true, 0)<0){
        // Something went terribly wrong if we are in here
        exit(-1);
    }
    get_process_struct_of_pid(getpid())->return_value = retval;
    while(1){}
}




int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg){
    //-------------------------------Input check--------------  

    // We verify that the process that made this call is a validprocess ie not a zombie 
    if (!(getpid() == -1) && validate_action_process_valid(get_process_struct_of_pid(getpid())) < 0){
        return -1;
    }
    // We check that the function arguments are valid
    if (!(prio<= MAXPRIO && prio>=MINPRIO)){
        return -1;
    }

    // Naif check, we can do a thorough check of memory at this level
    // in here or we can do that use memory api methods  
    if (!(ssize > 0)){
        return -1;
    }

    //----------Process generation-----------

    process *new_process = (process*) malloc(sizeof(process));
    if (new_process == NULL){
        return -1;
    }

    //---------Create a new pid and and new process to hash table----------------

    new_process->pid = increment_pid_and_get_new_pid();
    hash_set(get_process_hash_table(), cast_int_to_pointer(new_process->pid), new_process);

    //--------------Priority config--------------

    new_process->prio = prio;

    //---------------Name config-----------------

    new_process->process_name = (char*) malloc(strlen(name));
    if (new_process->process_name == NULL){
        return -1;
    }
    strcpy(new_process->process_name, name);

    //--------------State config---------------------

    new_process->state = ACTIVATABLE;

    //---------------Memory config-----------------------

    // We add PROCESS_SETUP_SIZE because we need space to call the function
    // and in order to place the exit method in the stack
    new_process->ssize = ssize + PROCESS_SETUP_SIZE;

    void* frame_pointer = process_memory_allocator(new_process->ssize);
    if (frame_pointer == NULL){
        return -1;
    }

    //--------------------Process function config-----------
    new_process->func = pt_func;

    //----------------Context setup-------------------------
    new_process->context_process = (context_t*) malloc(sizeof(context_t));
    if (new_process->context_process == NULL){
        return -1;
    }

    new_process->context_process->sp = (uint64_t) frame_pointer;
    // During the context_switch we will call the process_call_wrapper that has to call
    // the method given as function argument that we placed in s1 also the call has to
    // be made the right argument that is in s2 and it also has to call the exit_process method
    // at the end as this will be important in the case the user uses a return call
    new_process->context_process->ra = (uint64_t) process_call_wrapper;
    new_process->context_process->s1 = (uint64_t) pt_func;
    // debug_print("[start -> %d] function adress funciton adress = %ld\n", new_process->pid, (long) pt_func);
    new_process->context_process->s2 = (uint64_t) arg;
    new_process->context_process->sepc = (uint64_t) process_call_wrapper;

    // We must created a stack that has the size of a frame and place it in the kernel 
    // memory space that will be used to handle interrupts for this process

    void* interrupt_frame_pointer = get_frame();
    if (interrupt_frame_pointer == NULL){
        return -1;
    }
    new_process->context_process->sscratch = (uint64_t) interrupt_frame_pointer;

    //--------------Tree management----------------
    // The parent of the process is the process that called the start method
    new_process->parent = (process*) hash_get(get_process_hash_table(), cast_int_to_pointer(getpid()), NULL);


    // if the parent process is null that means we created the head of the tree thus the new process
    // is not attached to a parent
    if (new_process->parent != NULL){
        // We add the new process as a child to the parent process
        if (new_process->parent->children_tail != NULL){
            new_process->parent->children_tail->next_sibling = new_process;
            new_process->parent->children_tail = new_process;
        }
        else{
            new_process->parent->children_head = new_process;
            new_process->parent->children_tail = new_process;
        }
    }
    new_process->children_head = NULL;
    new_process->children_tail = NULL;
    new_process->next_sibling = NULL;

    //--------------Return value----------------
    new_process->return_value= NULL;

    //------------Add process to the activatable queue
    add_process_to_queue_wrapper(new_process, ACTIVATABLE_QUEUE);

    debug_print("[%s] created process with pid = %d \n", new_process->process_name, new_process->pid);

    //------------We activate this new process if it has a higher priority-----------
    // This function must be called a the very end
    check_if_new_prio_is_higher_and_call_scheduler(new_process->prio, true, 0);
    return new_process->pid;
}


int waitpid(int pid, int *retvalp){
    debug_print_exit_m("[waitpid] Inside waitpid with pid  = %d\n", pid);
    if (get_process_struct_of_pid(getpid())->children_head == NULL &&
        get_process_struct_of_pid(getpid())->children_tail == NULL ){
        return -1;
    }
    process* temp_process = NULL;
    process* temp_process_before = NULL;
    int pid_to_return;
    // negative pid, we find the first zombie and we take its return value and free it
    if (pid<0){
        // temp_process = get_process_struct_of_pid(getpid())->children_head;
        // temp_process_before = temp_process;
        while(true){
            temp_process = get_process_struct_of_pid(getpid())->children_head;
            temp_process_before = temp_process;
            while (temp_process != NULL){
                if (temp_process->state == ZOMBIE){
                    break;
                }
                temp_process_before = temp_process;
                temp_process=temp_process->next_sibling;
            }
            get_process_struct_of_pid(getpid())->state = BLOCKEDWAITCHILD;    
            scheduler();
        }
    }
    // positive pid, we verify pid is a child and then we take its return value and
    // we kill it
    else{
        temp_process = get_process_struct_of_pid(getpid())->children_head;
        temp_process_before = temp_process;
        // We check that the pid is a child to the process that called this method
        while (temp_process != NULL){
            if (temp_process->pid == pid){
                break;
            }
            temp_process_before = temp_process;
            temp_process = temp_process->next_sibling;
        }
        if (temp_process==NULL){
            // pid of the process given as function argument in not a child
            // to current process
            return -1;
        }
        while(true){
            // We wait until the state becomes a zombie
            if (temp_process->state == ZOMBIE){
                break;
            }
            get_process_struct_of_pid(getpid())->state = BLOCKEDWAITCHILD;    
            scheduler();
        }
    }
    // We take the return value of the process and then we kill it
    pid_to_return = temp_process->pid;
    if (retvalp != NULL){
        *retvalp = temp_process->return_value; 
    }
    if (free_process_arg_and_fix_tree_link(temp_process, temp_process_before)<0){
        return -1;
    }
    return pid_to_return;
}


int kill(int pid){
    if (pid == idleId){
        //Idle process cannot be killed
        return -1;
    }
    process* process_pid = get_process_struct_of_pid(pid);
    if (process_pid == NULL){
        return -1;
    }
    if (validate_action_process_valid(process_pid) < 0){
       return -1;
    }
    if (process_pid->pid == getpid()){
        return -1;// We cannot kill the current process from the current process
    }
    if (leave_queue_process_if_needed(process_pid)<0){
        return -1;
    }
    if (turn_current_process_into_a_zombie_or_kill_it(false, pid)<0){
        return -1;
    }
    process_pid->return_value = 0;
    return 0;
}
