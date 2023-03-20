#include "scheduler.h"
#include "stdbool.h"
#include "process.h"
#include "semaphore_api.h"
#include "helperfunc.h"

hash_t* semaphore_table = NULL;
int semaphore_id_counter =0;

int unblock_process_sem(semaphore_t* sem_struct){
    if (sem_struct->list_header_process == NULL){
        return 1;//Wait was never called
    }
    if (sem_struct->list_header_process->head_blocked == NULL){
        return 1;//List is empty
    }
    blocked_process_semaphore_t* blocked_proc = sem_struct->list_header_process->head_blocked;
    if (sem_struct->list_header_process->head_blocked == sem_struct->list_header_process->tail_blocked){
        sem_struct->list_header_process->head_blocked = NULL;
        sem_struct->list_header_process->tail_blocked = NULL;
    }
    else {
        sem_struct->list_header_process->head_blocked = sem_struct->list_header_process->head_blocked->next_shared_page;   
    }
    blocked_proc->blocked_process->state = ACTIVATABLE;
    add_process_to_queue_wrapper(blocked_proc->blocked_process, ACTIVATABLE_QUEUE);
    free(blocked_proc);
    return 0;
}


int init_semaphore_table() {
  semaphore_table = (hash_t *)malloc(sizeof(hash_t));
  if (semaphore_table == NULL) {
    return -1;
  }
  return 0;
}

int scount(int sem){
    semaphore_t* sem_struct = get_semaphore_struct(sem);
    if (sem_struct == NULL){
        return -1;
    }
    int return_val = sem_struct && sem_struct->count;
    return return_val;
}

int screate(short int count){
    if (count<0){
        return -1;
    }
    //TODO : Ask professor about this
    // if (semaphore_id_counter == INT_MAX){
    //     return -1;
    // }
    semaphore_t* sem = (semaphore_t*) malloc(sizeof(semaphore_t));
    if (sem == NULL){
        return -1;
    }
    sem->atomic_block = false;
    sem->count = (int16_t) count;
    sem->list_header_process = NULL;
    int semaphore_id = increment_semaphore_id();
    if (hash_set(get_shared_pages_hash_table(),
        cast_int_to_pointer(semaphore_id),
        sem)<0){
        return -1;
    }
    return 0;
}

int wait(int sem){
    semaphore_t* sem_struct = get_semaphore_struct(sem);
    if (sem_struct == NULL){
        return -1;
    }
    process* proc_sem = get_process_struct_of_pid(getpid());
    if (proc_sem == NULL){
        return -1;
    }
    //If the value of the sem_struct is true then the semaphore is 
    //being used this in this case 
    while (sem_struct->atomic_block == true); //TODO ADD PATERSON'S SOLUTION
    //We acquire the semaphore, making any ch
    sem_struct->atomic_block = true; //Does not work
    if (sem_struct->count == MIN_COUNT){
        sem_struct->atomic_block = false;
        return -2;
    }
    sem_struct->count--;
    if (sem_struct->count<0){
        //we add the process to the list 
        //of processes that are bloqued by this semaphore
        if (sem_struct->list_header_process == NULL){
            sem_struct->list_header_process = (list_semaphore_header_t*) malloc(sizeof(list_semaphore_header_t));
            if (sem_struct->list_header_process == NULL){
                sem_struct->atomic_block = false;
            }
            sem_struct->list_header_process->head_blocked = NULL;
            sem_struct->list_header_process->tail_blocked = NULL;
        }
        
        blocked_process_semaphore_t* blocked_proc_sem = (blocked_process_semaphore_t*) malloc(sizeof(blocked_process_semaphore_t));
        if (blocked_proc_sem == NULL){
                sem_struct->atomic_block = false;
        }
        blocked_proc_sem->blocked_process = proc_sem;
        blocked_proc_sem->next_shared_page = NULL;

        if (sem_struct->list_header_process->head_blocked == NULL && sem_struct->list_header_process->tail_blocked == NULL){
            sem_struct->list_header_process->head_blocked = blocked_proc_sem;
            sem_struct->list_header_process->tail_blocked = blocked_proc_sem;
        }
        else {
            sem_struct->list_header_process->tail_blocked->next_shared_page = blocked_proc_sem;
            sem_struct->list_header_process->tail_blocked = blocked_proc_sem;
        }
        sem_struct->atomic_block = false; 
        proc_sem->state = BLOCKEDSEMAPHORE;
    }
    sem_struct->atomic_block = false; 
    scheduler();
    //while(block) which is process/semaphore value that change by the scheduler
    return NULL;
}


int signal(int sem){
    semaphore_t* sem_struct = get_semaphore_struct(sem);
    if (sem_struct == NULL){
        return -1;
    }
    //If the value of the sem_struct is true then the semaphore is 
    //being used this in this case 
    while (sem_struct->atomic_block == true); //TODO ADD PATERSON'S SOLUTION
    //We acquire the semaphore, making any ch
    sem_struct->atomic_block = true; //Does not work
    if (sem_struct->count == MAX_COUNT){
        sem_struct->atomic_block = false; 
        return -2;
    }
    sem_struct->count++;
    if (sem_struct->count<=0){
        //we remove thethe process from list 
        //of processes that are bloqued by this semaphore
        unblock_process_sem(sem_struct);
    }
    sem_struct->atomic_block = false; 
    return NULL;
}

int sdelete(int sem){
    semaphore_t* sem_struct = get_semaphore_struct(sem);
    if (sem_struct == NULL){
        return -1;
    }
    while (sem_struct->atomic_block == true); //TODO ADD PATERSON'S SOLUTION
    //We acquire the semaphore, making any ch
    sem_struct->atomic_block = true; //Does not work
    //We free 
    while(unblock_process_sem(sem_struct)!=1);
    if (sem_struct->list_header_process != NULL){
        free(sem_struct->list_header_process);
    }
    free(sem_struct);
    hash_del(get_semaphore_table(), cast_int_to_pointer(sem));
    // sem_struct->atomic_block = false;
    return 0;
}

int sreset(int sem, int count){
    semaphore_t* sem_struct = get_semaphore_struct(sem);
    if (sem_struct == NULL){
        return -1;
    }
    if (count<0){
        return -1;
    }
    while (sem_struct->atomic_block == true); //TODO ADD PATERSON'S SOLUTION
    //We acquire the semaphore, making any ch
    sem_struct->atomic_block = true; //Does not work
    //We free everything
    while(unblock_process_sem(sem_struct)!=1);
    sem_struct->count = (int16_t) count;
    sem_struct->atomic_block = false;
    return 0;
}

int try_wait(int sem){
    //Question :une erreur est retournée pour indiquer que l'opération P ne peut être appelée sans bloquer le processus appelant. Enfin, 
    //si l'opération doit provoquer un dépassement de capacité du compteur, alors elle n'est pas effectuée. //ASK PROF: logically depassement impossible
    semaphore_t* sem_struct = get_semaphore_struct(sem);
    if (sem_struct == NULL){
        return -1;
    }
    while (sem_struct->atomic_block == true); //TODO ADD PATERSON'S SOLUTION
    //We acquire the semaphore, making any changes to it impossible
    sem_struct->atomic_block = true; //Does not work
    if (sem_struct->count > 0){
        sem_struct->count--;
    }
    else{
        sem_struct->atomic_block = false; 
        return -3;
    }
    sem_struct->atomic_block = false; //Does not work
    return 0;
}

int signaln(int sem, short int count){
    for (int i = 0; i<count; i++){
        int val = signal(sem);
        if (val != 0){
            return val;
        }
    }
    return 0;
}