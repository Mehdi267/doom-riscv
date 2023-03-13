/**
* Projet PCSEA RISC-V
* Mehdi Frikha
* See license for license details.
*/

#include "helperfunc.h"
#include "process.h"
#include "stddef.h"

#include "stdio.h"
#include "assert.h"
#include "stddef.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "riscv.h"


void* cast_int_to_pointer(int int_to_cast){
    return (void*)((long) int_to_cast);
}

long cast_pointer_into_a_long(void * pointer){
    return (long) pointer;
}

int cast_pointer_into_a_int(void * pointer){
    return (int)((long) pointer);
}


void set_supervisor_interrupts(bool val){
    if (val){
        csr_set(sstatus, 2);
    }
    else{
        csr_clear(sstatus, 2);
    }
}

void print_pte(page_table_entry *pte)
{
    debug_print_memory("valid = %i \n ",pte->valid);
    debug_print_memory("read = %i \n ",pte->read);
    debug_print_memory("write = %i \n ",pte->write);
    debug_print_memory("exec = %i \n ",pte->exec);
    debug_print_memory("res = %i \n ",pte->resU);
    debug_print_memory("global = %i \n ",pte->global);
    debug_print_memory("resA = %i \n ",pte->resA);
    debug_print_memory("resD = %i \n ",pte->resD);
    debug_print_memory("rsw = %i \n ",pte->rsw);
    debug_print_memory("ppn0 = %i \n ",pte->ppn0);
    debug_print_memory("ppn1 = %i \n ",pte->ppn1);
    debug_print_memory("ppn2 = %i \n ",pte->ppn2);
    debug_print_memory("reserved = %i \n ",pte->reserved);
    debug_print_memory("pbmt = %i \n ",pte->pbmt);
    debug_print_memory("n = %i \n ",pte->n);
}


process* get_process_struct_of_pid(int pid){
    process* process_pid = ((process*) hash_get(get_process_hash_table(), cast_int_to_pointer(pid), NULL));
    if (process_pid == NULL){
        return NULL;
    }
    return process_pid;
}


char* getname(void){
    process* currently_running_process = ((process*) hash_get(get_process_hash_table(), cast_int_to_pointer(getpid()), NULL));
    if (currently_running_process == NULL){
        return NULL;
    }
    return currently_running_process->process_name;
}


char* get_pid_name(int pid){
    process* process_pid = ((process*) hash_get(get_process_hash_table(), cast_int_to_pointer(pid), NULL));
    if (process_pid == NULL){
        return NULL;
    }
    return process_pid->process_name;

}

int increment_pid_and_get_new_pid(){
    pid_iterator++;
    return pid_iterator;
}


hash_t* get_process_hash_table(void){
    return pid_process_hash_table;
}


int validate_action_process_valid(process* process_pid){
    if (process_pid == NULL){
        return -1;
    }
    if (process_pid->state == ZOMBIE){
        return -1;
    }
    return 0;
}

