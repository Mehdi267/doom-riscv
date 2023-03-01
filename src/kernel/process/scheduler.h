/**
* Projet PCSEA RISC-V
* Mehdi Frikha
* See license for license details.
*/

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_


#include "stdint.h"
#include "hash.h"
#include "process.h"
#include "queue.h"

typedef enum _queue_type {ACTIVATABLE_QUEUE, ASLEEP_QUEUE} queue_process_type;


/**
 * @brief Global process queue
 * @param activatable_process_queue : used ot link activatable processes will mostly be used by the scheduler to 
 * execute the approriate process
 * @param asleep_process_queue : used to link process that are asleep, the priority that we will be used here 
 * is the time that the process spent asleep
 */
extern link activatable_process_queue;
extern link asleep_process_queue;

extern int init_scheduling_process_queue();

/**
 * @brief Adds a process to a queue using prio as the 
 * element that will be used in the priority of the linked list 
 * to organize the elements and it identify the queue that we will exploit 
 * with the argument type of type queue_process_type 
 * @param type the queue at which we will add the element
 * @param process_to_add the process that we will add
 */
extern void add_process_to_queue_wrapper(process* process_to_add, queue_process_type type);

/**
 * @brief deletes a process to a queue indetifier by the enum 
 * queue_process_type that is also given an argument 
 * @param type used to indicate which queue we will use
 * @param process_to_delete the process that we will delete from the list
 */
extern void delete_process_from_queue_wrapper(process* process_to_delete, queue_process_type type);

/**
 * @brief get the peek process from a queue identified by the type param
 * @param type used to indicate which queue we take the peek from
 */
extern process* get_peek_element_queue_wrapper(queue_process_type type);

/**
 * @brief does the process schduling
 * 
 */
extern void scheduler();

#endif

