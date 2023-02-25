/*
 * Projet PCSEA RISC-V
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 * Frédéric Pétrot <frederic.petrot@univ-grenoble-alpes.fr> - 2022
 * See license for license details.
 */

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "stdint.h"
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
} context_t;

/*
 * Save the current context on the stack and restore a previously saved context
 * current	: data structure in which the current context will be saved
 * future	: data structure holding the context of the future process to execute
 */
void context_switch(context_t *current, context_t *future);

/**
 * @brief Changes the priority attribute of the process with pid 
 * given in the function arguments
 * @param pid id of the process that will change the priority
 * @param newprio the new priority that will given to the process
 * @return int the new priority that was set if the pid exits and a negative value 
 * if the pid given in the function argument does not exists
 */
int chprio(int pid, int newprio);

/**
 * @brief Exists the running process  
 * and the value retval is passed to the parent process tha called waitpid.
 * @param retval this value is passed as signal to the parent process
 */
void exit(int retval);

/**
 * @brief Returns the pid of the currently running process
 * @return the pid of the currently running process
 */
int getpid(void);

/**
 * @brief Returns the priority of the process with the pid given 
 * as function arguments
 * @param pid id of the process that we will get the priority of
 * @return int the priority of the process with if given as the function argument 
 * or a negative value if the process does not exist
 */
int getprio(int pid);

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
int kill(int pid);

#endif