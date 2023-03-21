#include "msgqueue.h"
#include "process/process.h"
#include "process/scheduler.h"
#include "assert.h"
#include <stdint.h>
#include <stdio.h>
#include "process/helperfunc.h"
#define SUCCES 0
#define FAILURE -1
#define NBQUEUE 100

msg_queue_t* all_queues[NBQUEUE];

/*
** HELPER FUNCTIONS DECLARATION
*/

/**
 * @brief For each process in queue make it activable and push it in the
 * activable queue
 * @param queue: queue of blocked process;
 */
void free_blocked_queue(link * queue);

/**
 * @brief checks if the msg_queue is full
 * @param msg_queue: the msg_queue
 */
bool is_full(msg_queue_t * msg_queue);

/**
 * @brief checks if the msg_queue is empty
 * @param msg_queue: the msg_queue
 */
bool is_empty(msg_queue_t * msg_queue);


/**
 * @brief removes and returns the oldest message from the msg_queue
 * @param msg_queue: the msg_queue
 */
int pop_oldest_msg(msg_queue_t * msg_queue);

/**
 * @brief initialize each queue in all_queues variable to NULL
 */
void init_msg_queues();

/**
 * @brief returns the 1st free queue index of all_queues
 */
int get_first_free_index();

/**
 * @brief add the message to msg_queue
 * @param msg_queue: the msg_queue
 * @param message: the message
 * @note this function doesnt verify if theres is space available,
 * make sure there's space calling is full helper func
 */
void add_message(msg_queue_t *msg_queue, int message);

/*
** MAIN FUNCTIONS
*/
int pcreate(int count) {
  int index;
  if ((index = get_first_free_index()) < 0)
    return -1;
  all_queues[index] = malloc(sizeof(msg_queue_t));
  all_queues[index]->msg_arr = malloc(sizeof(int) * count);
  all_queues[index]->size = count;
  all_queues[index]->iffc = 0;
  all_queues[index]->oldmi = -1;
  INIT_LIST_HEAD(&all_queues[index]->blocked_cons);
  INIT_LIST_HEAD(&all_queues[index]->blocked_prod);
  return index;
}

int pdelete(int fid) {
  if (fid < 0 || fid >= NBQUEUE)
    return FAILURE;
  // destruction of the msg queue
  free(all_queues[fid]->msg_arr);
  // for each blocked consummer process make it activatable
  free_blocked_queue(&all_queues[fid]->blocked_cons);
  // for each blocked producer process make it activatable
  free_blocked_queue(&all_queues[fid]->blocked_prod);
  return SUCCES;
}

int psend(int fid, int message) {
  // check valid fid
  if (fid < 0 || fid >= NBQUEUE)
    return FAILURE;
  msg_queue_t * msg_queue = all_queues[fid];
  link * blocked_consumers = &(msg_queue->blocked_cons);
  link * blocked_producers = &(msg_queue->blocked_prod);
  // the queue is empty and there are blocked consumers
  if (is_empty(msg_queue) && !queue_empty(blocked_consumers)) {
    process * curr = queue_out(blocked_consumers, process, next_prev);
    curr->state = ACTIVATABLE;
    queue_add(curr, &activatable_process_queue, process, next_prev, prio);
    curr->message.value = message;
    curr->message.status = RECEIVED;
  } else if (is_full(msg_queue)) {
    // the msg_queue is full
    process * p = get_current_process();
    p->message.value = message;
    p->state = BLOCKEDQUEUE;
    queue_add(p, blocked_producers, process, next_prev, prio);
    scheduler();
  } else {
    // no process is blocked and there is space in the msgqueue
    add_message(msg_queue, message);
  }
  return SUCCES;
}

int preceive(int fid, int *message) {
  // check valid fid
  if (fid < 0 || fid >= NBQUEUE)
    return FAILURE;

  msg_queue_t * msg_queue = all_queues[fid];
  link * blocked_consumers = &(msg_queue->blocked_cons);
  if (is_empty(msg_queue)) {
    // the msg_queue is empty
    process * p = get_current_process();
    p->state = BLOCKEDQUEUE;
    queue_add(p, blocked_consumers, process, next_prev, prio);
    scheduler();
    if (p->message.status != RECEIVED) {
      return FAILURE;
    }
    if (message != NULL)
      *message = p->message.value;
  } else if (is_full(msg_queue) && !queue_empty(&msg_queue->blocked_cons)) {
    if (message != NULL)
      *message = pop_oldest_msg(msg_queue);
    process * blocked = queue_out(blocked_consumers, process, next_prev);
    add_message(msg_queue, blocked->message.value);
    queue_add(blocked, &activatable_process_queue, process, next_prev, prio);
  } else {
    // the queue is not empty and there arent blocked consumers
    if (message != NULL)
      *message = pop_oldest_msg(msg_queue);
  }
  return SUCCES;

}




/*
** HELPER FUNCTIONS DEFINITION
*/
void free_blocked_queue(link * queue) {
  // for each blocked process make it activatable
  while(!queue_empty(queue)) {
    process* curr = queue_out(queue, process, next_prev);
    curr->state = ACTIVATABLE;
    // reset pointers
    curr->next_prev.next = 0;
    curr->next_prev.prev = 0;
    queue_add(curr, &activatable_process_queue, process, next_prev, prio);
  }

}

bool is_full(msg_queue_t * msg_queue) {
  return msg_queue->iffc == msg_queue->oldmi;
}

bool is_empty(msg_queue_t * msg_queue) {
  assert(msg_queue->msg_arr != NULL);
  return msg_queue->oldmi == -1;
}
// TODO: check if possible interrupts can make problems
int pop_oldest_msg(msg_queue_t * msg_queue) {
  int msg =  msg_queue->msg_arr[msg_queue->oldmi];
  if (msg_queue->iffc != msg_queue->oldmi + 1) {
    msg_queue->oldmi++;
  } else {
    assert(msg_queue->oldmi == 0 && msg_queue->iffc == 1);
    msg_queue->oldmi--;
    msg_queue->iffc--;
  }
  return msg;
}

void init_msg_queues() {
  for (int i = 0; i < NBQUEUE; i++)
    all_queues[i] = NULL;
}

int get_first_free_index() {
  for (int i = 0; i < NBQUEUE; i++) {
    if (all_queues[i] == NULL)
      return i;
  }
  return -1;
}

void add_message(msg_queue_t *msg_queue, int message) {
  msg_queue->msg_arr[msg_queue->iffc] = message;
  if (msg_queue->oldmi == -1) {
    // (i.e) this message is the 1st
    assert(msg_queue->iffc == 0);
    msg_queue->oldmi = 0;
  }
  msg_queue->iffc = (msg_queue->iffc + 1) % msg_queue->size;
}
