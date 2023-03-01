/**
* Projet PCSEA RISC-V
* Mehdi Frikha
* See license for license details.
*/

#include "helperfunc.h"
#include "process.h"
#include "stddef.h"

void* cast_int_to_pointer(int int_to_cast){
    return (void*)((long) int_to_cast);
}

long cast_pointer_into_a_long(void * pointer){
    return (long) pointer;
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

