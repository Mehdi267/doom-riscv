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
    int pid_idle = start(idle, 1000, 1, "idle", NULL);
    printf("[create_idle_process] idle pid = %d\n", pid_idle);
    return pid_idle;
}


int idle(void *arg)
{
    for (;;) {
        printf("[%s] pid = %d\n", get_pid_name(getpid()), getpid());
        scheduler();
    }
}


int process_1(void *arg)
{
    int i = 0;
    for (;;) {
        i++;
        printf("[%s] pid = %d\n", get_pid_name(getpid()), getpid());
        scheduler();
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
        printf("[%s] pid = %d\n", get_pid_name(getpid()), getpid());
        scheduler();
        // context_switch(get_process_struct_of_pid(2)->context_process, get_process_struct_of_pid(1)->context_process);
        // if (i == 5){
        //     return 2;
        // }
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
    int p1 = start(process_1, 1000, 1, "proc1", NULL);
    int p2 = start(process_2, 1000, 1, "proc2", NULL);
    if (p1<0 || p2<0){
        return -1;
    }
    return 0;
}

