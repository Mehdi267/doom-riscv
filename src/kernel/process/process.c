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
#include "scheduler.h"


int initialize_process_hash_table(){
    pid_process_hash_table = (hash_t*) malloc(sizeof(hash_t));
    if (pid_process_hash_table == NULL){
        return -1;
    }
    return 0;
}


static int create_idle_process(){
    int pid_idle = start(idle, 1000, 1, "idle", cast_int_to_pointer(300));
    debug_print_process("[create_idle_process] idle pid = %d\n", pid_idle);
    return pid_idle;
}


int activate_and_launch_process(process* process_to_activate){
    if (process_to_activate == NULL){
        return -1;
    }
    if (setpid(process_to_activate->pid)<0){
        return -1;
    }
    if (process_to_activate->state != ACTIVATABLE){
        return -1;
    }
    delete_process_from_queue_wrapper(process_to_activate, ACTIVATABLE_QUEUE);
    process_to_activate->state = ACTIF;
    scheduler();
    //process_to_activate->func(NULL);
    return 0;
}


int idle(void *arg)
{
    for (;;) {
        debug_print_process("[Current process = %s] pid = %d; argument given = %ld \n", 
                            get_pid_name(getpid()), 
                            getpid(), 
                            cast_pointer_into_a_long(arg));
        // scheduler();
    }
}


int process_1(void *arg)
{
    int i = 0;
    for (;;) {
        i++;
        debug_print_process("[Current process = %s] pid = %d; argument given = %ld \n", 
                            get_pid_name(getpid()), 
                            getpid(), 
                            cast_pointer_into_a_long(arg));
        // scheduler();
        // if (i == 6){
        //     return 1;
        // }
    }
}


int process_2(void *arg)
{
    int i = 0;
    for (;;) {
        i++;
        debug_print_process("[Current process = %s] pid = %d; argument given = %ld \n", 
                            get_pid_name(getpid()), 
                            getpid(), 
                            cast_pointer_into_a_long(arg));
        // scheduler();
    }
}


int initialize_process_workflow(){
    init_scheduling_process_queue();
    if (initialize_process_hash_table()<0){
        return -1;
    }
    if (hash_init_direct(pid_process_hash_table)<0){
        return -1;
    }
    if (create_idle_process()<0){
        return -1;
    }
    int p1 = start(process_1, 1000, 1, "proc1", cast_int_to_pointer(100));
    int p2 = start(process_2, 1000, 1, "proc2", cast_int_to_pointer(200));
    if (p1<0 || p2<0){
        return -1;
    }
    return 0;
}

