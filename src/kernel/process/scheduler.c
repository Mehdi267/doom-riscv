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


LIST_HEAD(activatable_process_queue);
LIST_HEAD(asleep_process_queue);
// initially user process are not on, this will become true when we first visit the scheduler
bool started_user_process = false; 

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


process* pop_element_queue_wrapper(queue_process_type type){
    if (type == ACTIVATABLE_QUEUE){
        return queue_out(&activatable_process_queue, process, link_queue_activable);
    }
    else if (type == ASLEEP_QUEUE){
        return queue_out(&asleep_process_queue, process, link_queue_asleep);
    }
    return NULL;

}

process* get_peek_element_queue_wrapper(queue_process_type type){
    if (type == ACTIVATABLE_QUEUE){
        return queue_top(&activatable_process_queue, process, link_queue_activable);
    }
    else if (type == ASLEEP_QUEUE){
        return queue_top(&asleep_process_queue, process, link_queue_asleep);
    }
    return NULL;

}




void scheduler(){
    
    debug_print_scheduler("[scheduler] Inside the scheduler with pid equal to %d \n", getpid());
    //Process has been called before any execution has started
    int currently_running_process_id = getpid();
    

    //Scheduler has been called directly for the first time 
    if (getpid() == -1){ // pid has not been set yet
        //TODO VÉRIFIE QUE LA LIST N'EST PAS VIDE, CE NE PEUX JAMAIS ÊTRE LE CAS CAR LA LISTE DOIT CONTENIR IDLE MAIS 
        // C'EST IMPORTANT POUR TROUVER LES ERREURS LORS DU DEBUG
        process* top_process = pop_element_queue_wrapper(ACTIVATABLE_QUEUE);
        if (top_process == NULL){
            return;
        }
        setpid(top_process->pid);
        debug_print_scheduler("[scheduler] Inside the scheduler with no process running, default launch of the peek process with id = %d", getpid());
        top_process->state = ACTIF;
        first_process_call(top_process->context_process); // pid is set by the activate method
    }
    //Custom process enable // the user must set a valid pid
    if (started_user_process == false){
        //In this case no process is running and we have called the scheduler for the first time
        // yet in this case, we want to start with a custom process/ that has been set by the user
        // in the pid field and the user has also eliminated the process from the queue
        process* top_process = get_process_struct_of_pid(getpid());
        if (top_process == NULL){
            return;
        }
        started_user_process = true;
        debug_print_scheduler("[scheduler] Inside the scheduler with no process running, custom launch of the process with id = %d", getpid());
        first_process_call(get_process_struct_of_pid(getpid())->context_process); // pid is set by the activate method
    }
    else{
        //In here we treat the normal case 
        //We take the current process struct:
        process* current_process = get_process_struct_of_pid(currently_running_process_id);
        process* top_process = get_peek_element_queue_wrapper(ACTIVATABLE_QUEUE);
        if (top_process == NULL || current_process == NULL){
            return;
        }
        debug_print_scheduler("[scheduler] current process pid = %d, peek pid = %d\n", current_process->pid, top_process->pid);
        debug_print_scheduler("[scheduler] current process priority = %d, peek priority = %d\n", current_process->prio, top_process->prio);
        if (top_process->prio >= current_process->prio){
            pop_element_queue_wrapper(ACTIVATABLE_QUEUE);
            //In this case we switch the process
            setpid(top_process->pid);
            top_process->state = ACTIF;
            add_process_to_queue_wrapper(current_process, ACTIVATABLE_QUEUE);  
            context_switch(current_process->context_process, top_process->context_process);
        }
    }
    debug_print_scheduler("[scheduler] I managed to return to the scheduler %d\n",getpid());
    return;

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

