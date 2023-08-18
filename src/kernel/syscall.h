/**
 * @file syscall_handler.h
 * @brief System call handler header file
 * 
 * This header file contains declarations for the system call handler
 * and related functions.
 */

#ifndef _SYSCALL_HANDLER_H_
#define _SYSCALL_HANDLER_H_

#include "traps/trap.h"
/**
 * @brief A void function used for testing purposes.
 * 
 * This syscall will be used as a bridge between kernel and user mode
 * for testing purposes. The implementation of this function may change
 * based on what is being tested.
 */
void void_call();

/**
 * @brief Handle system calls and dispatch appropriate actions.
 * 
 * @param tf Pointer to the trap frame containing register values.
 * @return unsigned long Return value based on the executed system call.
 */
unsigned long syscall_handler(struct trap_frame *tf);

#endif // _SYSCALL_HANDLER_H_
