/**
* Projet PCSEA RISC-V
* Mehdi Frikha
* See license for license details.
*/
#include "process.h"
#include "hash.h"
#include "stddef.h"
#include "string.h"
#include "mem.h"
#include "stdlib.h"
#include "helperfunc.h"
#include "stdio.h"
#include "frame_dist.h"
#include "stdbool.h"
#include "queue.h"
#include "assert.h"
#include "scheduler.h"
#include "stdbool.h"
#include "stddef.h"
#include "stddef.h"
#include <stdint.h>
#include "../sync.h"


LIST_HEAD(activatable_process_queue);
LIST_HEAD(asleep_process_queue);
// initially user process are not on, this will become true when we first visit the scheduler
bool started_user_process = false; 
scheduler_struct *scheduler_main;

int init_scheduling_process_queue(){
    return 0;
}


void add_process_to_queue_wrapper(process* process_to_add, queue_process_type type){
    if (type == ACTIVATABLE_QUEUE){
        queue_add(process_to_add, &activatable_process_queue, process, next_prev, prio);
    }
    else if (type == ASLEEP_QUEUE){
        queue_add(process_to_add, &asleep_process_queue, process, next_prev, sleep_time);
    }
}


void delete_process_from_queue_wrapper(process* process_to_delete, queue_process_type type){
    if (type == ACTIVATABLE_QUEUE){
        queue_del(process_to_delete, next_prev);
    }
    else if (type == ASLEEP_QUEUE){
        queue_del(process_to_delete, next_prev);
    }

}


process* pop_element_queue_wrapper(queue_process_type type){
    if (type == ACTIVATABLE_QUEUE){
        return queue_out(&activatable_process_queue, process, next_prev);
    }
    else if (type == ASLEEP_QUEUE){
        return queue_out(&asleep_process_queue, process, next_prev);
    }
    return NULL;

}

process* get_peek_element_queue_wrapper(queue_process_type type){
    if (type == ACTIVATABLE_QUEUE){
        return queue_top(&activatable_process_queue, process, next_prev);
    }
    else if (type == ASLEEP_QUEUE){
        return queue_top(&asleep_process_queue, process, next_prev);
    }
    return NULL;
}


uint64_t get_smallest_sleep_time() {
  process* p = get_peek_element_queue_wrapper(ASLEEP_QUEUE);
  if (p) {
    assert(-p->sleep_time >= 0);
    return -p->sleep_time;
  }
  return 0;
}


void scheduler(){
    debug_print_scheduler_no_arg("\n-----------------Scheduler--------------------\n");
    debug_print_scheduler("[scheduler -> %d] Inside the scheduler with pid equal to %d \n",
                            getpid(), getpid());

    // awake process and insert the in activable queue;
    uint64_t sleep_time = get_smallest_sleep_time();
    while (sleep_time != 0 && current_clock() > sleep_time) { // several process can be awaken
      process* awake = pop_element_queue_wrapper(ASLEEP_QUEUE);
      add_process_to_queue_wrapper(awake, ACTIVATABLE_QUEUE);
      sleep_time = get_smallest_sleep_time();
    }
    //Process has been called before any execution has started
    if (scheduler_main == NULL){
        return ;
    }
    //Scheduler has been called directly for the first time 
    if (getpid() == -1){ // pid has not been set yet
        process* top_process = get_peek_element_queue_wrapper(ACTIVATABLE_QUEUE);
        if (top_process == NULL){
            return;
        }
        pop_element_queue_wrapper(ACTIVATABLE_QUEUE);
        if (setpid(top_process->pid)<0){
            return;
        }
        /* set_supervisor_interrupts(true); */
        debug_print_scheduler("[scheduler -> %d] Inside the scheduler with no process running, default launch of the peek process with id = %d", getpid(),  getpid());
        debug_print_scheduler("[scheduler -> %d] running process name = %s\n", getpid(), getname());
        top_process->state = ACTIF;
        started_user_process = true;
        context_switch(scheduler_main->main_context, top_process->context_process); // pid is set by the activate method
    }
    //Custom process launch enable // the user must set a valid pid
    if (started_user_process == false){
        /* set_supervisor_interrupts(true); */
        //In this case no process is running and we have called the scheduler for the first time
        // yet in this case, we want to start with a custom process/ that has been set by the user
        // in the pid field and the user has also eliminated the process from the queue
        process* top_process = get_process_struct_of_pid(getpid());
        if (top_process == NULL){
            return;
        }
        started_user_process = true;
        debug_print_scheduler("[scheduler -> %d] Inside the scheduler with no process running, custom launch of the process with id = %d\n", getpid(),  getpid());
        debug_print_scheduler("[scheduler -> %d] running process name = %s\n", getpid(), getname());
        debug_print_scheduler("[scheduler -> %d] function adress of the process = %ld\n", getpid(), (long) get_process_struct_of_pid(getpid())->context_process->s[1]);
        debug_print_scheduler("[scheduler -> %d] idle adress = %ld\n", getpid(), (long) idle);
        context_switch(scheduler_main->main_context, get_process_struct_of_pid(getpid())->context_process); // pid is set by the activate method
    }
    else{
        //In here we treat the normal case 
        //We take the current process struct:
        process* current_process = get_process_struct_of_pid(getpid());
        process* top_process = get_peek_element_queue_wrapper(ACTIVATABLE_QUEUE);
        if (top_process == NULL || current_process == NULL){
            return; 
        }
        debug_print_scheduler("[scheduler -> %d] current process pid = %d, peek pid = %d\n", getpid(), current_process->pid, top_process->pid);
        debug_print_scheduler("[scheduler -> %d] [current process name --> |%s|], [peek name --> |%s|]\n", getpid(), current_process->process_name, top_process->process_name);
        debug_print_scheduler("[scheduler -> %d] current process priority = %d, peek priority = %d\n", getpid(), current_process->prio, top_process->prio);
        
        //If the process was actif when this treatement was called, then 
        //we swap the process if the peek prio is higher then the current prio
        //and we place the old process if the activatable queue
        if (current_process->state == ACTIF){
            if (top_process->prio >= current_process->prio){
                //In this case we switch the process and 
                debug_print_scheduler("[scheduler -> %d] Swapping process current pid = %d ->>>>>>>> peek pid = %d\n", getpid(), current_process->pid, top_process->pid);
                pop_element_queue_wrapper(ACTIVATABLE_QUEUE);
                if (setpid(top_process->pid)<0){ 
                    return;
                }
                current_process->state = ACTIVATABLE;
                /* set_supervisor_interrupts(true); */
                top_process->state = ACTIF;
                add_process_to_queue_wrapper(current_process, ACTIVATABLE_QUEUE);  
                debug_print_scheduler("[scheduler -> %d] Swapping process current pid = %d ->>>>>>>> peek pid = %d\n", top_process->pid , current_process->pid, top_process->pid);
                context_switch(current_process->context_process, top_process->context_process);
            }
            else{
                debug_print_scheduler("[scheduler -> %d] Sticking with the same process id = %d\n", getpid(), current_process->pid);
            }
        }
        //If the process was killed we free its data 
        //and jump directly into an other context
        else if(current_process->state == KILLED){
            debug_print_scheduler("[scheduler -> %d] I am in a killed process id = %d, moving to process = %d\n",
                                    getpid(), current_process->pid, top_process->pid);
            pop_element_queue_wrapper(ACTIVATABLE_QUEUE);
            if (setpid(top_process->pid)<0){
                return;
            }
            /* set_supervisor_interrupts(true); */
            top_process->state = ACTIF;
            if (hash_del(get_process_hash_table(), cast_int_to_pointer(current_process->pid))<0){
                return ;
            }
            free(current_process);
            current_process = 0;
            direct_context_swap(top_process->context_process);
        }
        //if the process was placed in an other state when this was called
        //then we change the context directly and we don't place the process in the 
        //activatable queue
        else{
            debug_print_scheduler("[scheduler -> %d] Current process is not in a actif state= %d swaping to %d\n", 
                                    getpid(), current_process->pid, top_process->pid);
            pop_element_queue_wrapper(ACTIVATABLE_QUEUE);
            if (setpid(top_process->pid)<0){
                return;
            }
            /* set_supervisor_interrupts(true); */
            top_process->state = ACTIF;
            context_switch(current_process->context_process, top_process->context_process);
        }
    }
    debug_print_scheduler("[scheduler -> %d] I managed to return to the scheduler %d\n",getpid(),getpid());
}

/**
 * @brief x86 projet os function used only for reference
 * 
 */
// void ordonnanceLinkedlist()
// {
//     //int pid_running = mon_pid();
//     /*
//     Cette partie rend le process en cours d'execution qui est dans l'état Elu en l'etat Activable  et  inversement pour l'autre process
//     donc il passe de Activable vers ELU
//     */  
//     debug_print_scheduler("name of running process %s \n", mon_nom());
//     emptyAndFreeListeMourant();
//     process* oldRunningProcess = get_running_process();
//     debug_print_scheduler("Check point 2 \n");

//     if (oldRunningProcess->state == ELU){
//         debug_print_scheduler("Check point 2 before prime  \n");
//         oldRunningProcess->state = ACTIVABLE;
//         add_process_activable(oldRunningProcess);
//     }

//     debug_print_scheduler("Check point 2 prime  \n");

//     if (oldRunningProcess->state == MOURANT){
//         debug_print_scheduler("Adding dead process %s \n", mon_nom());
//         add_process_mourant(oldRunningProcess);
//     }

//     debug_print_scheduler("Check point 3 \n");
//     getHeadActivable()->processActivable->state = ELU;
//     update_running_process(getHeadActivable()->processActivable);
//     removeHeadProcessActivableListe();

//     if (getHeadEndormi() != NULL){
//         while (getHeadEndormi()){
//             if (getHeadEndormi() -> timeWakeUp <= getTicCount()){
//                 add_process_activable(getHeadEndormi()->processEndormi);
//                 removeHeadProcessEndormiListe(getHeadEndormi());
//             }
//             break;
//         }
//     }
//     debug_print_scheduler("Ordananceur pid_running = %i and new pid is equal to %i \n", oldRunningProcess->pid, get_running_process()->pid);
//     if (oldRunningProcess->pid != get_running_process()->pid){
//         ctx_sw(oldRunningProcess->table_registers, get_running_process()->table_registers);
//     }
//     return;
// }

