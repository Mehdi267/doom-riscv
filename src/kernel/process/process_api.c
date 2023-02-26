#include "process.h"
#include "hash.h"
#include "stddef.h"
#include "string.h"
#include "mem.h"
#include "stdlib.h"
#include "helperfunc.h"
#include "stdio.h"


//Hash table that associates to every pid the process struct associated to it
hash_t *pid_process_hash_table = NULL;
//Id du process encore d'execution
int current_running_process_pid = -1;
//Iterateur de pid qui sera utilisé pour associer au process des id
int pid_iterator = -1;


int getpid(void){
    return current_running_process_pid;
}


int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg){    
    //We check that the function arguments are valid
    if (!(prio<= MAXPRIO && prio>=MINPRIO)){
        return -1;
    }

    //Naif check, we can do a thorough check of memory at this level
    //in here or we can do that use memory api methods   
    if (!(ssize > 0)){
        return -1;
    }

    //generate process
    process *new_process = (process*) malloc(sizeof(process)); 
    if (new_process == NULL){
        return -1;
    }
    
    //Create a new pid and and new process to hash table
    new_process->pid = increment_pid_and_get_new_pid();
    hash_set(get_process_hash_table(), cast_int_to_pointer(new_process->pid), new_process);
    printf("proc pid %d \n", new_process->pid);

    //priority config
    new_process->prio = prio;
    
    //name config
    new_process->process_name = (char*) malloc(strlen(name)); 
    if (new_process->process_name == NULL){
        return -1;
    }
    strcpy(new_process->process_name, name);

    //state config
    new_process->state = ACTIVATABLE;
    
    //We add PROCESS_SETUP_SIZE because we need space to call the function 
    //and in order to place the exit method in the stack
    new_process->ssize = ssize + PROCESS_SETUP_SIZE; 

    //Context setup
    new_process->context_process = (context_t*) malloc(sizeof(context_t));
    if (new_process->context_process == NULL){
        return -1;
    }
    // new_process->context_process->ra = (uint64_t) exit;
    
    // new_process->context_process->a0 = (uint64_t) arg;

    //The parent of the process is the process that called the start method
    new_process->parent = (process*) hash_get(get_process_hash_table(), cast_int_to_pointer(getpid()), NULL);

    //if the parent process is null that means we created the head of the tree thus the new process
    //is not attached to a parent 
    if (new_process->parent != NULL){
        //We add the new process as a child to the parent process
        if (new_process->parent->children_tail != NULL){
            new_process->parent->children_tail->next_sibling = new_process;
            new_process->parent->children_tail = new_process;
        }
        else{
            new_process->parent->children_head = new_process;
            new_process->parent->children_tail = new_process;
        }
    }
    new_process->children_head = NULL;
    new_process->children_tail = NULL;
    new_process->next_sibling = NULL; 

    printf("[%s] created process with pid = %d \n", new_process->process_name, new_process->pid);
    return 0;
}



