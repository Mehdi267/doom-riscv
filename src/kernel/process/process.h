/**
* Projet PCSEA RISC-V
* Mehdi Frikha
* See license for license details.
*/


#ifndef _PROCESS_H_
#define _PROCESS_H_


#include "stdint.h"
#include "hash.h"
#include "queue.h"
#include "stdlib.h"
#include "stdio.h"
#include "stddef.h"
#include "stdarg.h"
#include "stdbool.h"

#define MAXPRIO 256
#define MINPRIO 1
#define NBPROC 30
#define PROCESS_SETUP_SIZE 2
#define IDLE_PROCESS_ID 1

/**
* Global variables
* pid_process_hash_table: Hash table that associates to every pid the process struct associated to it
* current_running_process_pid: Id of the process that is currently executing
* pid_iterator : Pid iterator that will be used to associate to every process a unique pid
* idle_process : idle process ie process that does nothing and have infinite loop
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
* registers sp and s0 to s11 are preserved across function calls.
* The rest has to be considered scratch when returning from the calls,
* Thus only these registers need to be saved in the context.
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
   uint64_t sscratch;
   uint64_t sepc;
} context_t;


/**
   * @brief Enum _process_state is used to associate to every process a certain state.
   * the text is take from the project spec
   * @param Active: The process is the one that owns the processor.
   * @param Enabled: The process only waits for the possession of the processor to run.
   * @param BLOCKEDSEMAPHORE on semaphore: The process has executed an operation on a semaphore which requires waiting to progress (for example wait).
   * @param BLOCKEDIO on I/O: The process is waiting for an I/O to be performed.
   * @param BLOCKEDWAITCHILD waiting for a child: The process is waiting for one of its child processes to complete.
   * @param ASLEEP: The process called wait_clock, the sleep primitive until a given time.
   * @param BLOCKEDQUEUE : 
   * @param Zombie: The process has either terminated or been terminated by the kill system call and its father is still alive and has not yet waitpided on it.
*/
typedef enum _process_state {ACTIF, ACTIVATABLE, BLOCKEDSEMAPHORE, BLOCKEDIO, BLOCKEDQUEUE, BLOCKEDWAITCHILD, ASLEEP, ZOMBIE} process_state;


/**
 * \brief 	A process function
 * @param	arg : the argument that will be given to the function when the process is called 
 */
typedef int	(*process_function_t)	(void*);


/**
* @brief this structure is given to all processesn it will stored at the kernel level
*/
typedef struct process_t{
   int pid; // id of the process
   char *process_name; // process name
   process_state state; // state of the process
   uint32_t ssize; // total the size allocated to the process
   uint16_t prio; // priority of the process
   context_t* context_process; //we store here the current execution context of the process ie the important registers
   process_function_t func; // the function that is associated to the process ; not very important and will be removed later
   struct process_t* parent; // parent process
   struct process_t* children_head; // the head of the children process
   struct process_t* children_tail; // the tail of the children_process
   struct process_t* next_sibling; // next sibling of the current process, this parameter is used to link the children of a process
   link link_queue_activable; //used to link the activatable processes 
   link link_queue_asleep; //used to link the asleep process
   int return_value; // return value of the process, used in waitpid
   bool called_before;
} process;


/**
 * @brief used to save the main execution context
*/
typedef struct scheduler_t{
   context_t* main_context; 
} scheduler_struct;


/**
* @brief this function defines the necessary data structures that will be exploited to
* have running processes;
* @return the value 0 if there were no errors and a negative number if there were errors
* @note The list of the data structures are :
* A hash table that will be used to associate to every pid a process structure, this table will be crucial
* for search, modification and exploitation of processes \n
* Queues are also created to store the actif and the asleep processes(not working atm, queues are defined
* globally but let's pray that i can actually make it work)
*/
extern int initialize_process_workflow();


/**
* @brief Sets the status of process given as argument to active and makes it the running process 
* and removes it from the activatable queue and launches it. Method is only called when we launch the first process
* @param process_to_activate the process that will transform into an actif, the process must not be null and the process 
* must 
* @return the value 0 if there were no errors and a negative number if there were errors
* @note THERE SHOULD NOT BE AN ACTIF PROCESS WHEN WE LAUNCH THIS or we will throw an error
*/
extern int activate_and_launch_process(process* process_to_activate);


/**
* @brief this method is used to to process function calls, by using the argument given
* by the user and it also adds a call to the exit method
* @note s0 holds the process argument with will given to a0
 * and s1 holds the process function
*/
extern void process_call_wrapper(void);


/**
* @brief Save the current context on the stack and restore a previously saved context \n
* current  : data structure in which the current context will be saved \n 
* future   : data structure holding the context of the future process to execute \n
*/
extern void context_switch(context_t *current, context_t *future);


/**
* @brief Runs the first executed process
*/
extern void first_process_call(context_t *current);



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
* and the value retval is passed to the parent process that is called waitpid.
* @param retval this value is passed as signal to the parent process
*/
extern void exit_process(int retval);


/**
* @brief Returns the pid of the currently running process
* @return the pid of the currently running process
*/
extern int getpid(void);

/**
* @brief checks if the newprio that was given to a process is superior to 
* priority of the currectly running proess and it were to be the case we will 
* call the scheduler
* @return a negative value if there were any problems
*/
extern int check_if_new_prio_is_higher_and_call_scheduler(int newprio);


/**
* @brief Checks if the process is an queue and leaves that queue if needed \n
* We only use the state of the process to make this assumption regarding the process
* location.
* @return a negative value if there were any problems
*/
extern int leave_queue_process_if_needed(process* process_to_leave);



/**
 * @brief This methods set the curretly running process to a certain value
 * this mehtod must only be called by the scheduler because it and only it change the 
 * currently runnign process
 * @param new_pid the new running process
 * @return int the pid that was given as argument or a negative value if there were an error
 */
extern int setpid(int new_pid);

/**
* @brief Returns the priority of the process with the pid given
* as function arguments
* @param pid id of the process that we will get the priority of
* @return int the priority of the process with if given as the function argument
* or a negative value if the process does not exist
*/
extern int getprio(int pid);


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
* @brief waits and places the return value of a the terminated child into the retvalp pointer
* @param pid if the value is negative, we select a random zombie child, if it is positive we check that the pid corresponds to
* child of the current process
* @param retvalp the address location in which we will store the return value of the zombie process
* @returns the pid of the process that we got the value from and if there is a problem a negative value is returned
* possible problems :
* if the pid is positive and the pid is not a child of the process that called the method does not exist
* if pid is negative and the current process does not have any children
*/
extern int waitpid(int pid, int *retvalp);


/**
* @brief the currently running process in the sleeping state until
* the number of clock interrupts passed in parameter is reached or exceeded.
*/
extern void wait_clock(unsigned long clock);

/**
 * @brief idle process, this process does nothing and never exits.
 * it has the pid 0 and it is the first process that we create, it priority is also equal to 1 
 * the lowest possible priority so that it only gets executed only if there are no other processes
 * currently running, might chahe prio to 0 less then the min prio so that when there are other processes
 * with prio 1 it does not share the time with them 
*/
extern int idle(void *arg);


/**
 * @brief the following macros are used when debugging the c 
 * code. Inspired from :
 * https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c
 */
#define DEBUG_LEVEL 2

#define debug_print(fmt, ...) \
        do {if (DEBUG_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_LEVEL == 2){ printf("File = %s : Line = %d: Func = %s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

/**
 * @brief the following macro are used to debug the scheduler,
 *  meaning when we debug the scheduler we use the debug_print_scheduler
 */
#define DEBUG_SCHEDULER_LEVEL 0 //Indicates if debug type is actuve

#define debug_print_scheduler(fmt, ...) \
        do {if (DEBUG_SCHEDULER_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_SCHEDULER_LEVEL == 2){ printf("File/Line/Func [%s][%d][%s]: " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

/**
 * @brief the following macro are used to debug the processes,
 *  meaning when we debug the scheduler we use the debug_print_process
 */
#define DEBUG_PROCESS_LEVEL 2 //Indicates if debug type is active

#define debug_print_process(fmt, ...) \
        do {if (DEBUG_PROCESS_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_PROCESS_LEVEL == 2){ printf("File/Line/Func [%s][%d][%s]: " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)


#endif

