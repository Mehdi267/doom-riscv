#include "scheduler.h"
#include "stdbool.h"
#include "process.h"
#include "semaphore_api.h"
#include "helperfunc.h"
#include <stdlib.h>
#include "msgqueue.h"
#include "assert.h"


//Boolean used to stop the child function from calling the scheduler
static bool parent_block = false;

#define XMETHODS_SEMAPHORE
//#define XMETHODS_MESSAGE_QUEUE

hash_t* semaphore_table = NULL;
int semaphore_id_counter = 0;
int currently_running_semaphores = 0;

int remove_proc_queue(semaphore_t* sem_struct, awake_signal_t signal_enum, int pid){
    if (sem_struct->list_header_process == NULL){
        return 1;//Wait was never called
    }
    if (sem_struct->list_header_process->head_blocked == NULL){
        return 1;//List is empty
    }
    blocked_process_semaphore_t* blocked_proc = NULL; 
    //Classic linked list ...
    if (sem_struct->list_header_process->head_blocked == sem_struct->list_header_process->tail_blocked){
        if (sem_struct->list_header_process->head_blocked->blocked_process->pid == pid){
            blocked_proc = sem_struct->list_header_process->head_blocked;
            sem_struct->list_header_process->head_blocked = NULL;
            sem_struct->list_header_process->tail_blocked = NULL;
        }
        else{
            return -1;
        }
    }
    else if (sem_struct->list_header_process->head_blocked->blocked_process->pid == pid){
        blocked_proc = sem_struct->list_header_process->head_blocked;
        sem_struct->list_header_process->head_blocked = blocked_proc->next_block_struct;
    }
    else if (sem_struct->list_header_process->tail_blocked->blocked_process->pid == pid){
        blocked_proc = sem_struct->list_header_process->tail_blocked;
        blocked_process_semaphore_t* iter = sem_struct->list_header_process->head_blocked;
        while (iter->next_block_struct != sem_struct->list_header_process->tail_blocked){
            iter = iter->next_block_struct;
        }
        iter->next_block_struct = NULL;
        sem_struct->list_header_process->tail_blocked = iter;
    }
    else {
        blocked_process_semaphore_t* iter = sem_struct->list_header_process->head_blocked;
        blocked_process_semaphore_t* iter_prev = sem_struct->list_header_process->head_blocked;
        while (iter!=NULL){
            if (iter->blocked_process->pid != pid){
                //We found our tag in this case 
                iter_prev->next_block_struct = iter->next_block_struct;
                blocked_proc = iter;   
                break;
            }
            iter_prev=iter;
            iter = iter->next_block_struct;
        }
        if (iter == NULL){
            //Bad call
            return -1;
        }
    }
    blocked_proc->blocked_process->state = ACTIVATABLE;
    if (signal_enum != KILL_CALL){
        //In this case the process does need to be executed again since it will be killed, thus we don't add to 
        //activatable queue
        queue_add(blocked_proc->blocked_process, &activatable_process_queue, process, next_prev, prio);
    }
    blocked_proc->blocked_process->sem_signal = signal_enum;
    free(blocked_proc);
    return 0;
}

int unblock_process_sem(semaphore_t* sem_struct, awake_signal_t signal_enum){
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
        sem_struct->list_header_process->head_blocked = sem_struct->list_header_process->head_blocked->next_block_struct;   
    }
    blocked_proc->blocked_process->state = ACTIVATABLE;
    if (signal_enum != KILL_CALL){
        //In this case the process does need to be executed again since it will be killed, thus we don't add to 
        //activatable queue
    }
    queue_add(blocked_proc->blocked_process, &activatable_process_queue, process, next_prev, prio);
    blocked_proc->blocked_process->sem_signal = signal_enum;
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
    //We have to return a signed 16 bits integer placed in a 32 bit int
    //While always having the upper half of hte bits always equal to 0
    int return_val = 0xffff & sem_struct->count;
    return return_val;
}

int screate(short int count){
    if (count<0){
        print_sem_api_no_arg("semcreate count fail 1\n");
        return -1;
    }
    if (currently_running_semaphores == NB_MAX_SEMS){
        print_sem_api_no_arg("semcreate max sem fail 2\n");
        return -1;
    }
    semaphore_t* sem = (semaphore_t*) malloc(sizeof(semaphore_t));
    if (sem == NULL){
        print_sem_api_no_arg("semcreate memory is full fail 3\n");
        return -1;
    }
    sem->count = (int16_t) count;
    sem->list_header_process = NULL;
    parent_block = false;
    int semaphore_id = increment_semaphore_id();
    if (semaphore_id < 0){
        //Overflow check
        return -1;
    }
    if (hash_set(get_semaphore_table(),
        cast_int_to_pointer(semaphore_id),
        sem)<0){
        return -1;
    }
    currently_running_semaphores++;
    return semaphore_id;
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
    //Since we call the wait method that means we suppose the exit type will be the normal type
    //the one called by the signal method, or if we just decrement inside this wait method 
    proc_sem->sem_signal = 0;

    //If the value of the sem_struct is true then the semaphore is 
    //being used this in this case 
    //We acquire the semaphore, making any ch
    
    if (sem_struct->count == MIN_COUNT){

        return -2;
    }
    sem_struct->count--;
    if (sem_struct->count<0){
        //we add the process to the list 
        //of processes that are bloqued by this semaphore
        if (sem_struct->list_header_process == NULL){
            sem_struct->list_header_process = (list_semaphore_header_t*) malloc(sizeof(list_semaphore_header_t));
            if (sem_struct->list_header_process == NULL){
        
            }
            sem_struct->list_header_process->head_blocked = NULL;
            sem_struct->list_header_process->tail_blocked = NULL;
        }
        
        blocked_process_semaphore_t* blocked_proc_sem = (blocked_process_semaphore_t*) malloc(sizeof(blocked_process_semaphore_t));
        if (blocked_proc_sem == NULL){
        
        }
        blocked_proc_sem->blocked_process = proc_sem;
        blocked_proc_sem->next_block_struct = NULL;

        if (sem_struct->list_header_process->head_blocked == NULL && sem_struct->list_header_process->tail_blocked == NULL){
            sem_struct->list_header_process->head_blocked = blocked_proc_sem;
            sem_struct->list_header_process->tail_blocked = blocked_proc_sem;
        }
        else {
            blocked_process_semaphore_t* iter_blocked_proc = sem_struct->list_header_process->head_blocked;
            blocked_process_semaphore_t* iter_blocked_proc_prev = sem_struct->list_header_process->head_blocked;
            int current_prio = proc_sem->prio;
            while(iter_blocked_proc != NULL){
                if ( current_prio > iter_blocked_proc->blocked_process->prio){
                    print_sem_api_no_arg("stuffing process into the semaphore queue\n");
                    if (iter_blocked_proc == sem_struct->list_header_process->head_blocked){
                        blocked_proc_sem->next_block_struct = sem_struct->list_header_process->head_blocked;
                        sem_struct->list_header_process->head_blocked = blocked_proc_sem;
                    }
                    else {
                        blocked_proc_sem->next_block_struct = iter_blocked_proc;
                        iter_blocked_proc_prev->next_block_struct = blocked_proc_sem;
                    }
                    break;    
                }
                iter_blocked_proc_prev = iter_blocked_proc; 
                iter_blocked_proc = iter_blocked_proc->next_block_struct;
            }
            if (iter_blocked_proc == NULL){
                print_sem_api_no_arg("Adding process into the semaphore tail\n");
                sem_struct->list_header_process->tail_blocked->next_block_struct = blocked_proc_sem;
                sem_struct->list_header_process->tail_blocked = blocked_proc_sem;
            }
        }
        proc_sem->semaphore_id = sem; 
        proc_sem->state = BLOCKEDSEMAPHORE;
    } 
    scheduler();
    //while(block) which is process/semaphore value that change by the scheduler
    return proc_sem->sem_signal;
}



int proc_kill_diag(int sem, awake_signal_t signal_enum, int pid){
    semaphore_t* sem_struct = get_semaphore_struct(sem);
    if (sem_struct == NULL){
        return -1;
    }
    //If the value of the sem_struct is true then the semaphore is 
    //being used this in this case 
    //We acquire the semaphore, making any ch
    
    //This should be impossible because this method wil only be called hwen we kill a process that is wait phase thus 
    //the semaphore is negatif at this point 
    if (sem_struct->count == MAX_COUNT){
 
        return -2;
    }
    sem_struct->count++;
    if (sem_struct->count<=0){
        //we remove the process from list 
        //of processes that are bloqued by this semaphore
        remove_proc_queue(sem_struct, KILL_CALL, pid);
    } 
    return NULL;
}

int signal(int sem){
    semaphore_t* sem_struct = get_semaphore_struct(sem);
    if (sem_struct == NULL){
        return -1;
    }
    //If the value of the sem_struct is true then the semaphore is 
    //being used this in this case 
    //We acquire the semaphore, making any ch
    
    if (sem_struct->count == MAX_COUNT){
 
        return -2;
    }
    sem_struct->count++;
    if (sem_struct->count<=0){
        //we remove thethe process from list 
        //of processes that are bloqued by this semaphore
        unblock_process_sem(sem_struct, SIGNAL_CALL);
    } 
    if(parent_block == false){
        scheduler();
    }
    return NULL;
}

int sdelete(int sem){
    semaphore_t* sem_struct = get_semaphore_struct(sem);
    if (sem_struct == NULL){
        return -1;
    }
    //We acquire the semaphore, making any ch
    
    //We free everything related to the semaphore
    while(unblock_process_sem(sem_struct, SDETETE_CALL)!=1);
    if (sem_struct->list_header_process != NULL){
        free(sem_struct->list_header_process);
    }
    // debug_print_memory("malloc test %p\n", malloc(1));
    // debug_print_memory("malloc test %p\n", malloc(500));
    hash_del(get_semaphore_table(), cast_int_to_pointer(sem));
    free(sem_struct);
    currently_running_semaphores--;
    scheduler();
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
    //We acquire the semaphore, making any ch
    //We free everything
    while(unblock_process_sem(sem_struct, SRESET_CALL)!=1);
    sem_struct->count = (int16_t) count;
    scheduler();
    return 0;
}

int try_wait(int sem){
    //Question :une erreur est retournée pour indiquer que l'opération P ne peut être appelée sans bloquer le processus appelant. Enfin, 
    //si l'opération doit provoquer un dépassement de capacité du compteur, alors elle n'est pas effectuée. //ASK PROF: logically depassement impossible
    semaphore_t* sem_struct = get_semaphore_struct(sem);
    if (sem_struct == NULL){
        return -1;
    }
    //We acquire the semaphore, making any changes to it impossible
    if (sem_struct->count > 0){
        sem_struct->count--;
    }
    else{
 
        return -3;
    } //Does not work
    scheduler();
    return 0;
}

int signaln(int sem, short int count){
    semaphore_t* sem_struct = get_semaphore_struct(sem);
    if (sem_struct == NULL){
        return -1;
    }
    if (count<0){
        return -1;
    }
    if (sem_struct->count + count > MAX_COUNT){
        return -2;
    }
    parent_block = true;
    for (int i = 0; i<count; i++){
        int val = signal(sem);
        if (val != 0){
            return val;
        }
    }
    parent_block = false;
    scheduler();
    return 0;
}

#ifdef XMETHODS_SEMAPHORE
    void xwait(union sem *s)
    {
            wait(s->sem);
    }

    void xsignal(union sem *s)
    {
            signal(s->sem);
    }

    void xscreate(union sem *s)
    {
            (s->sem = screate(0));
    }

    void xsdelete(union sem *s)
    {
            sdelete(s->sem);
    }
#endif

#ifdef XMETHODS_MESSAGE_QUEUE
    void xwait(union sem *s)
    {
            assert(preceive(s->fid, 0) == 0);
    }

    void xsignal(union sem *s)
    {
            int count;
            assert(psend(s->fid, 1) == 0);
            assert(pcount(s->fid, &count) == 0);
            //assert(count == 1); XXX
            assert(count < 2);
    }

    void xscreate(union sem *s)
    {
            assert((s->fid = pcreate(2)) >= 0);
    }

    void xsdelete(union sem *s)
    {
            assert(pdelete(s->fid) == 0);
    }
#endif

