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

LIST_HEAD(activatable_process_queue);
LIST_HEAD(asleep_process_queue);


int init_scheduling_process_queue(){
    return 0;
}


void add_process_to_queue_wrapper(process* process_to_add, queue_process_type type){
    if (type == ACTIVATABLE_QUEUE){
        queue_add(process_to_add, &activatable_process_queue, process, link_queue_activable, prio);
    }
    else if (type == ASLEEP_QUEUE){
        queue_add(process_to_add, &asleep_process_queue, process, link_queue_asleep, prio);
    }
}


void delete_process_from_queue_wrapper(process* process_to_delete, queue_process_type type){
    if (type == ACTIVATABLE_QUEUE){
        queue_del(process_to_delete,link_queue_activable);
    }
    else if (type == ASLEEP_QUEUE){
        queue_del(process_to_delete, link_queue_asleep);
    }

}


process* get_peek_element_queue_wrapper(queue_process_type type){
    if (type == ACTIVATABLE_QUEUE){
        return queue_out(&activatable_process_queue, process, link_queue_activable);
    }
    else if (type == ASLEEP_QUEUE){
        return queue_out(&asleep_process_queue, process, link_queue_asleep);
    }
    return NULL;

}


void scheduler(){
    printf("[scheduler] Inside the scheduler with pid equal to %d \n", getpid());
    //Process has been called before any execution has started
    int currently_running_process_id = getpid();
    
    //This remains very experimental
    if (currently_running_process_id == -1){
        //In this case no process is running and we have called the scheduler for the first time
        //we set the running proces in this case to the idle process
        printf("[scheduler] Inside the scheduler with pid equal to %d ", getpid());
        setpid(0);
    }
    else{
        //In here we treat the normal case 
        //We take the current process struct:
        process* current_process = get_process_struct_of_pid(currently_running_process_id);
        process* top_process = get_peek_element_queue_wrapper(ACTIVATABLE_QUEUE);
        printf("[scheduler] current process pid = %d, peek pid = %d\n", current_process->pid, top_process->pid);
        printf("[scheduler] current process prio = %d, peek prio = %d\n", current_process->prio, top_process->prio);
        if (top_process->prio > current_process->prio){
            setpid(top_process->pid);
            add_process_to_queue_wrapper(current_process, ACTIVATABLE_QUEUE);  
            context_switch(top_process->context_process, current_process->context_process);
        }
    }
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
//     printf("name of running process %s \n", mon_nom());
//     emptyAndFreeListeMourant();
//     process* oldRunningProcess = get_running_process();
//     printf("Check point 2 \n");

//     if (oldRunningProcess->state == ELU){
//         printf("Check point 2 before prime  \n");
//         oldRunningProcess->state = ACTIVABLE;
//         add_process_activable(oldRunningProcess);
//     }

//     printf("Check point 2 prime  \n");

//     if (oldRunningProcess->state == MOURANT){
//         printf("Adding dead process %s \n", mon_nom());
//         add_process_mourant(oldRunningProcess);
//     }

//     printf("Check point 3 \n");
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
//     printf("Ordananceur pid_running = %i and new pid is equal to %i \n", oldRunningProcess->pid, get_running_process()->pid);
//     if (oldRunningProcess->pid != get_running_process()->pid){
//         ctx_sw(oldRunningProcess->table_registers, get_running_process()->table_registers);
//     }
//     return;
// }

