/**
* Projet PCSEA RISC-V
* Mehdi Frikha
* See license for license details.
*/

#ifndef _HELPFUNCTIONS_H_
#define _HELPFUNCTIONS_H_

#include "stdint.h"
#include "process.h"

/**
 * @brief Casts an int to a void * pointer 
*/
extern void* cast_int_to_pointer(int);

/**
 * @brief Casts a pointer into an int
*/
extern long cast_pointer_into_a_long(void * pointer);

/**
 * @brief returns the struct process that holds the pid given as the function argument pid
 * @param pid id of the process that we want to get its struct
 * @return the process struct that hold pid argument or null if it can't find the struct 
*/
extern process* get_process_struct_of_pid(int pid);

/**
 * @brief Returns the name of the currently running process
 * @return the name of the currently running process
 */
extern char* getname(void);

/**
 * @brief Returns the name of the process with with the pid given
 * as the function parameter
 * @param pid id of the process that we want to print its name 
 * @return the name of the process with the pid given ad the function argument or NULL if it can't find it
 */
extern char* get_pid_name(int pid);


/**
 * @brief Increment the pid and returns the new value of 
 * the pid
*/
extern int increment_pid_and_get_new_pid();
/**
 * @brief Returns the hash table tha map id with process structs
*/
extern hash_t* get_process_hash_table(void);

/**
 * @brief Validate that the process is an state on which we can call system calls
 * we will mostly check that the process is not a ZOMBIE but the function can be extended to 
 * other processes
*/
extern int validate_action_process_valid(process* process_pid);

/**
 * @brief set supervisor interrupts to the value given a fnction argument
*/
extern void set_supervisor_interrupts(bool val);






#endif