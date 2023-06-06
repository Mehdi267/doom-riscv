/**
* Projet PCSEA RISC-V
* Mehdi Frikha
* See license for license details.
*/
#include "stdbool.h"
#include "../process/process.h"
#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_


#define MAX_COUNT 32767
#define MIN_COUNT -32768

#define NB_MAX_SEMS 10000

/**
 * @brief This is enum is used to determine the type of signal that was used to 
 * to make the process leave the wake up from the semaphore
 * @param SIGNAL_CALL the signal type that results from the call of th signal method
 * @param SDETETE_CALL the signal type that results from the call of th sdelete method
 * @param SRESET_CALL the signal type that results from the call of th sreset method
 * 
 */
typedef enum _awake_signal {
  SIGNAL_CALL = 0,
  SDETETE_CALL = -3,
  SRESET_CALL = -4,
  KILL_CALL,
} awake_signal_t;

//This table will link a semaphore with its information
extern hash_t* semaphore_table;
//This value is used to set the id of the semaphore
extern int semaphore_id_counter;
//Indicates how many semaphores are currecly running
extern int currently_running_semaphores;

/**
 * @brief Initializes the semaphore hash table that link the 
 * semaphores with its information
 * @return int a postive value if successfull and a negative value otherwise 
 */
extern int init_semaphore_table();

/**
 * @brief used to save all the pages that the current process is using
*/
typedef struct blocked_process_semaphore{
   process* blocked_process;
   struct blocked_process_semaphore* next_block_struct;
} blocked_process_semaphore_t;

/**
 * @brief used to save all the blocked semaphores that are stuck in the asleep method
*/
typedef struct list_semaphore_header{
   blocked_process_semaphore_t* head_blocked;
   blocked_process_semaphore_t* tail_blocked;
} list_semaphore_header_t;

/**
 * @brief THis struct is used to save information about semaphores
 * it will mostly be linked to semaphore id(the linkage will be created using a hash table) 
 * @param atomic_block is used to enforce atomic operation ->true meaning a process is acting upon the semaphore
 * and false if the semaphore is free
 * @param count indicates the current value of the semaphore
 * @param list_header_process a pointer to the list of the processes that are asleep 
 */
typedef struct semaphore{
    int16_t count;
    list_semaphore_header_t* list_header_process;
}semaphore_t;


/**
 * @brief Returns the count of the semaphore given as function argument
 * @param sem : semaphore id
 * @return int les 16 bits de poids fort sont à 0, et les 16 bits de poids faible,
 * interprétés comme un entier signé sur 16 bits, sont la valeur du sémaphore
 */
int scount(int sem);

/**
 * @brief This function creates a semaphore, 
 * If there are no more semaphores available, or if count is negative, the call fails and the return value is -1, otherwise 
 * it is equal to the identification of the semaphore that was allocated.
 * 
 * @param count the count value that will attached to the semaphore
 * @return int -1 if the creation of the semaphore was not successful and 
 * the id of the semaphore if the creation of hte semaphore was successful 
 */
int screate(short int count);

/**
 * @brief Passes all of the processes that are blocked within the semaphores
 * in the activatable state 
 * @param sem the id of the semaphore that we want to delete
 * @return int if the semaphore id given as function argument is not calid the we return -1 
 * of we simply return null(0) of the operation was succesfull
 */
int sdelete(int sem);

/**
 * @brief Passes all of the processes that are blocked within the semaphores
 * in the activatable state and associates a new count value to the semaphore equal
 * to the one given as function argument 
 * @param sem semaphore id 
 * @param count the new count value that will be associated to the semaphore 
 * @return int if the sem id 
 */
int sreset(int sem, int count);

/**
 * @brief The wait primitive decrements the value of the semaphore identified by sem by 1. If the operation should cause the counter to overflow, 
 * then it is not performed. Otherwise, if the value after decrementation is strictly negative,
 *  the process goes from the active state to the blocked state on the semaphore identified by the value sem. 
 * This process can return to the activatable 
 * or active state when another process executes a signal/signaln primitive which will release it, or the sreset or sdelete primitives.
 * 
 * @param sem 
 * @return int The return value of wait is -1 if the value of sem is invalid, -2 in the event of an overflow,
 *  -3 if the wake-up is consecutive to sdelete, -4 if it is consecutive to sreset, null otherwise.
 * @note I used google translate on the description that was provided in the projet details 
 */
int wait(int sem);

/**
 * @brief The try_wait primitive tests the value of the semaphore. If the value is strictly greater than 0, 
 * then the primitive decrements by 1 the value of the semaphore identified by sem.
 * If the value before decrement is negative or null, an error is returned to indicate that the P operation cannot be called without blocking the calling process. Finally, 
 * if the operation should cause the counter to overflow, then it is not performed.
 * 
 * @param sem 
 * @return int The return value of try_wait is -1 if the value of sem is invalid, -3 if the counter is negative or zero, -2 on overflow, and zero otherwise.
 * @note I used google translate on the description that was provided in the projet details 
 */
int try_wait(int sem);


/**
 * @brief The signal primitive increments the value of the semaphore identified by sem by 1 
 * and if the value after incrementation is negative or zero, the first process
 * in the blocked state in the semaphore queue goes into the activatable or active state.
 * 
 * @param sem the id the semaphore that we want to call sign  
 * @return int La valeur de retour de signal/signaln est -1 si la valeur de sem est invalide, 
 * -2 en cas de dépassement de capacité et nulle sinon.
 * @note I used google translate on the description that was provided in the projet details 
 */
int signal(int sem);


/**
 * @brief The signaln primitive is equivalent to count signal primitives executed atomically,
 *  i.e. without another process being able to execute between two calls to signal.
 * @param sem the semaphore that we can to call 
 * @param count the amount of times that we want to call the signal method 
 * @return int 
 * @note I used google translate on the description that was provided in the projet details 
 */
int signaln(int sem, short int count);

/**
 * @brief This method is called to launch a when we kill a aprocess and we want to remove from the semaphore queue 
 * and we also in this case enumerate the semaphore counter to make sure that the new process cab get called  
 * @param sem semaphore id 
 * @param signal_enum the signal that we want exploit we want remove the association between the process and the semaphore
 * @param pid the process id that we want to remove from the queue
 * @return int 
 */
int proc_kill_diag(int sem, awake_signal_t signal_enum, int pid);

//Sempahore wrappers that will be used 
/* Wrapper sur les verrous basés sur les sémaphores ou les files de messages */
union sem {
   int fid;
   int sem;
};
void xwait(union sem *s);
void xsignal(union sem *s);
void xsdelete(union sem *s);
void xscreate(union sem *s);


#endif