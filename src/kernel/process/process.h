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
#include "../memory/pages.h"
#include "../memory/virtual_memory.h"
/**
 * @brief global function constants
 * @param MAXPRIO the maximun priority of a process
 * @param MINPRIO the minimun priority of a process
 * @param NBPROC the higest number of process that we can define at a time
 * @param PROCESS_SETUP_SIZE defines the overhead that is needed for a every process(still experimental)
 *  
*/
#define MAXPRIO 256
#define MINPRIO 1
#define NBPROC 30
#define PROCESS_SETUP_SIZE 2
#define idleId 1
#define kernelId 0

/**
 * @brief These variables define the execution state of the program
 * @param DEBUG will launch debug process and eventually will print the debug messages 
 * @param TESTING will launch the testing process and will call the kernel_tests
 * @param RELEASE will not do the above and launch the kernel is production mode
 * @param TESTING_MEMORY will not do the above and launch the kernel is production mode
 * @note IMPORTANT : Only one of these variables should defined at a time
 * @param 
*/
// #define DEBUG
#define TESTING
// #define RELEASE
// #define DEBUG_SCHEDULER
#define TESTING_MEMORY

/**
* @brief Global variables
* @param pid_process_hash_table: Hash table that associates to every pid the process struct associated to it
* @param current_running_process_pid: Id of the process that is currently executing
* @param pid_iterator : Pid iterator that will be used to associate to every process a unique pid
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
   uint64_t satp;
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
   * @param KILLED: The process has been terminated and will be removed shortly from memory 
*/  
typedef enum _process_state {   ACTIF,
                                ACTIVATABLE,
                                BLOCKEDSEMAPHORE,
                                BLOCKEDIO,
                                BLOCKEDQUEUE,
                                BLOCKEDWAITCHILD,
                                ASLEEP,
                                ZOMBIE,
                                KILLED
} process_state;



/**
 * \brief 	A process function
 * @param	arg : the argument that will be given to the function when the process is called 
 */
typedef int	(*process_function_t)	(void*);


/**
  * @brief this structure is given to all processesn it will stored at the kernel level
  * @param pid  id of the process
  * @param process_name  process name
  * @param state  state of the process
  * @param ssize  total the size allocated to the process
  * @param prio  priority of the process
  * @param context_process we store here the current execution context of the process ie the important registers
  * @param func  the function that is associated to the process  not very important and will be removed later
  * @param parent  parent process
  * @param children_head  the head of the children process
  * @param children_tail  the tail of the children_process
  * @param next_sibling  next sibling of the current process, this parameter is used to link the children of a process
  * @param link_queue_activable used to link the activatable processes 
  * @param link_queue_asleep used to link the asleep process
  * @param return_value  return value of the process, used in waitpid
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
   page_table* page_table_level_2;
   page_table_link_list_t* page_tables_level_1_linkedlist;
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
extern int activate_and_launch_custom_process(process* process_to_activate);

/**
 * @brief launches the scheduler with not set defined process, the process that 
 * we will launch will be taken directly from the queue 
*/
void activate_and_launch_scheduler(void);

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
 * @brief This method is called when we want to jump to the context of a process
 * directly with doinga context switch, this will happen when a process is killed
 * and we don't need its context any more nor the memory it holds thus in this scenario we kill
 * it
 * @param future the context that we will go to 
 */
extern void direct_context_swap(context_t *future);

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
* @param current_or_queue this value idicates if the new prio will be compared to the currently running prio
* of a prio given as function argument \n
* True-> compare to current prio
* False-> compare to prio given as function arugment
* @param prio_to_compare_queue a set prio that we will compare the new prio to if the bool argument is False
* @return a negative value if there were any problems
*/
int check_if_new_prio_is_higher_and_call_scheduler(int newprio, bool current_or_queue, int prio_to_compare_queue);


/**
* @brief Checks if the process is an queue and leaves that queue if needed \n
* We only use the state of the process to make this assumption regarding the process
* location.
* @param process_to_leave the process that will be removed from the queues 
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
* @brief This function allocates memory for a process, it's current
* form remains very basic and does not follow the project specifications
* and it is only valid for a size that is less than then page size
* @param size corresponds to the size that we want to allocate
* @param process_conf the process at which the memory allocater will work on 
* @return the address of the page that we allocated
*/
void *process_memory_allocator(process* process_conf, unsigned long size);


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
#define DEBUG_LEVEL 0 //Indicates if debug type is active 

#define debug_print(fmt, ...) \
        do {if (DEBUG_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_LEVEL == 2){ printf("File = %s : Line = %d: Func = %s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

/**
 * @brief the following macro are used to debug the scheduler,
 *  meaning when we debug the scheduler we use the debug_print_scheduler
 */
#define DEBUG_SCHEDULER_LEVEL 0 //Indicates if debug type is active

#define debug_print_scheduler(fmt, ...) \
        do {if (DEBUG_SCHEDULER_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_SCHEDULER_LEVEL == 2){ printf("File/Line/Func [%s][%d][%s]: " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

#define debug_print_scheduler_no_arg(fmt, ...) \
        do {if (DEBUG_SCHEDULER_LEVEL){ printf(fmt);} } while (0)



/**
 * @brief the following macro are used to debug the processes,
 *  meaning when we debug the scheduler we use the debug_print_process
 */
#define DEBUG_PROCESS_LEVEL  0 //Indicates if debug type is active

#define debug_print_process(fmt, ...) \
        do {if (DEBUG_PROCESS_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_PROCESS_LEVEL == 2){ printf("File/Line/Func [%s][%d][%s]: " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)


/**
 * @brief the following macro are used to debug the processes,
 *  meaning when we debug the scheduler we use the debug_print_process
 */
#define DEBUG_EXIT_METHODS_LEVEL 0 //Indicates if debug type is active

#define debug_print_exit_m(fmt, ...) \
        do {if (DEBUG_EXIT_METHODS_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_EXIT_METHODS_LEVEL == 2){ printf("File/Line/Func [%s][%d][%s]: " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)


/**
 * @brief the following macro are used when running the tests
 */
#define DEBUG_TESTING_LEVEL 1 //Indicates if debug type is active

#define debug_print_tests(fmt, ...) \
        do {if (DEBUG_TESTING_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_TESTING_LEVEL == 2){ printf("File/Line/Func [%s][%d][%s]: " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

#define print_test_no_arg(fmt, ...) \
        do {if (DEBUG_TESTING_LEVEL){ printf(fmt);} } while (0)



/**
 * @brief the following macro are used to debug the memory management
 */
#define DEBUG_MEMORY_LEVEL 1 //Indicates if debug type is active

#define debug_print_memory(fmt, ...) \
        do {if (DEBUG_MEMORY_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_MEMORY_LEVEL == 2){ printf("File/Line/Func [%s][%d][%s]: " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

#define print_memory_no_arg(fmt, ...) \
        do {if (DEBUG_MEMORY_LEVEL){ printf(fmt);} } while (0)


#endif
