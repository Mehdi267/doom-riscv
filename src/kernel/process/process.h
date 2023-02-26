/*
 * Projet PCSEA RISC-V
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 * Frédéric Pétrot <frederic.petrot@univ-grenoble-alpes.fr> - 2022
 * See license for license details.
 */

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "stdint.h"
#include "hash.h"

#define MAXPRIO 256
#define MINPRIO 1
#define NBPROC 30
#define PROCESS_SETUP_SIZE 2

/**
 * Global variables
 * pid_process_hash_table: Hash table that associates to every pid the process struct associated to it
 * current_running_process_pid: Id du process encore d'execution
*/
extern hash_t* pid_process_hash_table;
extern int current_running_process_pid;
extern int pid_iterator;


/**
 * @brief Allocated space for the hash table that we will use and 
 * does error hadling of the malloc 
 * @return the value 0 if there were no errors and a negative number if there were errors 
*/
extern int initialize_process_hash_table();

/* Context switching is done by a function call:
 * the riscv abi says that the only warranty we have is that the
 * registers sp and s0 to s11 are preserved accros function calls.
 * The rest has to be considered scratch when returning from the calls,
 * thus only these registers need be saved in the context.
 * Note: a few more registers will need to be saved when introducing
 * supervisor/user processes.
 */
typedef struct context {
	uint64_t sp;
	uint64_t ra;
	uint64_t s0;
	uint64_t s1;
	uint64_t s2;
	uint64_t s3;
	uint64_t s4;
	uint64_t s5;
	uint64_t s6;
	uint64_t s7;
	uint64_t s8;
	uint64_t s9;
	uint64_t s10;
	uint64_t s11;
	uint64_t a0;
} context_t;

/**
 * @brief Enum _process_state is used to associate to every process a certain state.
 * the text is take from the projet spec
 * Active: The process is the one that owns the processor.
	Enabled: The process only waits for the possession of the processor to run.
	Blocked on semaphore: The process has executed an operation on a semaphore which requires to wait to progress (for example wait).
	Blocked on I/O: The process is waiting for an I/O to be performed.
	Blocked waiting for a child: The process is waiting for one of its child processes to complete.
	Sleeping: The process called wait_clock, the sleep primitive until a given time.
	Zombie: The process has either terminated or been terminated by the kill system call and its father is still alive and has not yet waitpided on it. 
*/
typedef enum _process_state {ACTIF, ACTIVATABLE, BLOCKEDSEMAPHORE, BLOQUEDIO, BLOCKEDQUEUE, BLOCKEDWAITCHILD, ASLEEP, ZOMBIE} process_state;

/**
 * this structure is given to all processesn it will stored at the kernel level
*/
typedef struct process_t{
    int pid; // id of the process
    char *process_name; // process name
    process_state state; // state of the process
    uint32_t ssize; // total the size allocated to the process
	uint16_t prio; // priority of the process
	context_t* context_process; //on stocke ici tout les elments important pour l'utilisation du process 
	struct process_t* parent; // parent process
	struct process_t* children_head; // the head of the children process 
	struct process_t* children_tail; // the tail of the children_process
	struct process_t* next_sibling; // next sibling of the current process, this parameter is used to link the children of a proces
} process;

/**
 * @brief this function defines the necessary data structures that will be exploited to 
 * have running processes;
 * @return the value 0 if there were no errors and a negative number if there were errors 
 * @note The list of the data structures are :
 * A hash table that will be used to associate to every pid a process struture, this table will be crutial
 * for search, modification and exploitation of processess 
 * 
*/
int initialize_process_workflow();

/*
 * Save the current context on the stack and restore a previously saved context
 * current	: data structure in which the current context will be saved
 * future	: data structure holding the context of the future process to execute
 */
extern void context_switch(context_t *current, context_t *future);

/**
 * @brief Changes the priority attribute of the process with pid 
 * given in the function arguments
 * @param pid id of the process that will change the priority
 * @param newprio the new priority that will given to the process
 * @return int the new priority that was set if the pid exits and a negative value 
 * if the pid given in the function argument does not exists
 */
extern int chprio(int pid, int newprio);

/**
 * @brief Exists the running process  
 * and the value retval is passed to the parent process that called waitpid.
 * @param retval this value is passed as signal to the parent process
 */
extern void exit(int retval);

/**
 * @brief Returns the pid of the currently running process
 * @return the pid of the currently running process
 */
extern int getpid(void);

/**
 * @brief Returns the priority of the process with the pid given 
 * as function arguments
 * @param pid id of the process that we will get the priority of
 * @return int the priority of the process with if given as the function argument 
 * or a negative value if the process does not exist
 */
extern int getprio(int pid);

/**
 * @brief  
 * and the value retval is passed to the parent process tha called waitpid.
 * @param retval this value is passed as signal to the parent process
 */

/**
 * @brief Kills the process with the pid given in the argument 
 * @param pid if of the process that we will kill
 * @return int the priority of the process that holds the id given as the function
 * argument if the process exists or a negative value 
 */
extern int kill(int pid);


/**
 * @brief generates a new process that will the function given in the parameter, 
 * will allocate at least ssize memory and will associate to the created process. the value prio 
 * to indicate the priority of the process and the arg argument will be used as argument to the method provided
 * @note This function does not exploit virtual memory, it will be replaced eventually 
 * by an other functions with different arguments
 * @param pt_func the function that will be run by the process
 * @param ssize the memory size that will be allocated to the process, this value is given by the user
 * @param prio the priority of the process, must be a positive value between 1 and MAXPRIO
 * @param name name of the process
 * @param arg argument that will be given to the called function
 * @returns the pid of the process that was created and if there is a problem while creating the process for lack of space
 * for example a negative value if returned  
*/
extern int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg);


/**
 * @note This function will eventually take the name start when virtual memory is complete 
*/
extern int start_virtual(const char *name, unsigned long ssize, int prio, void *arg);

/**
 * Puts the currently running process in the sleeping state until 
 * the number of clock interrupts passed in parameter is reached or exceeded. 
*/
extern void wait_clock(unsigned long clock);

/**
 * 
*/
extern int waitpid(int pid, int *retvalp);

extern int idle(void *arg);

#endif