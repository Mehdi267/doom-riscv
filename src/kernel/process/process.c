#include "process.h"
#include "hash.h"
#include "stddef.h"
#include "string.h"
#include "mem.h"
#include "stdlib.h"
#include "helperfunc.h"
#include "stdio.h"



int initialize_process_hash_table(){
    pid_process_hash_table = (hash_t*) malloc(sizeof(hash_t));
    if (pid_process_hash_table == NULL){
        return -1;
    }
    return 0;
}


static int create_idle_process(){
    return start(idle, 1000, 1, "idle", NULL);
}

/**
 * @brief idle process, this process does nothing and never exits.
 * it has the pid 0 and it is the first process that is created
*/
int idle(void *arg)
{
    for (;;) {
        printf("[%s] pid = %d\n", get_pid_name(0), 0);
        context_switch(get_process_struct_of_pid(0)->context_process, get_process_struct_of_pid(1)->context_process);
    }
}

int process_1(void *arg)
{
    for (;;) {
        printf("[%s] pid = %i\n", get_pid_name(1), 1);
        context_switch(get_process_struct_of_pid(1)->context_process, get_process_struct_of_pid(2)->context_process);
    }
}

int process_2(void *arg)
{
    for (;;) {
        printf("[%s] pid = %i\n", get_pid_name(2), 2);
        context_switch(get_process_struct_of_pid(2)->context_process, get_process_struct_of_pid(1)->context_process);
    }
}


int initialize_process_workflow(){
    if (initialize_process_hash_table()<0){
        return -1;
    }
    if (hash_init_direct(pid_process_hash_table)<0){
        return -1;
    }
    if (create_idle_process()<0){
        return -1;
    }
    start(process_1, 1000, 1, "proc1", NULL);
    start(process_2, 1000, 1, "proc2", NULL);
    return 0;
}

