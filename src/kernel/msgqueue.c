#include "msgqueue.h"
#include "process/scheduler.h"
#include "assert.h"
#include <stdint.h>
#include <stdio.h>
#include "process/helperfunc.h"
#define SUCCES 0
#define FAILURE -1
#define NBQUEUE 100

msg_queue_t* all_queues[NBQUEUE];
int count = 0;

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



int pcreate(int count) {
  int index;
  if ((index = get_first_free_index()) < 0)
    return -1;
  all_queues[index] = malloc(sizeof(msg_queue_t));
  all_queues[index]->msg_arr = malloc(sizeof(int) * count);
  all_queues[index]->size = count;
  all_queues[index]->iffc = 0;
  INIT_LIST_HEAD(&all_queues[index]->blocked_cons);
  INIT_LIST_HEAD(&all_queues[index]->blocked_prod);
  return index;
}

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

bool is_empty(msg_queue_t * msgqueue) {
  assert(msgqueue->msg_arr != NULL);
  return msgqueue->iffc == 0;

}
// TODO: check if possible interrupts can make problems
int psend(int fid, int message) {
  msg_queue_t * msg_queue = all_queues[fid];
  link * blocked_consumers = &(msg_queue->blocked_cons);
  link * blocked_producers = &(msg_queue->blocked_prod);
  // the queue is empty and there are blocked consumers
  if (is_empty(msg_queue) && !queue_empty(blocked_consumers)) {
    process * curr = queue_out(blocked_consumers, process, next_prev);
    curr->state = ACTIVATABLE;
    queue_add(curr, &activatable_process_queue, process, next_prev, prio);
    curr->message.message = message;
    curr->message.status = RECEIVED;
  } else if (msg_queue->size == msg_queue->iffc) {
    // the msg_queue is full
    process * p = get_current_process();
    queue_add(p, blocked_producers, process, next_prev, prio);
    scheduler();
  } else {
    // no process is blocked and there is space in the msgqueue
    msg_queue->msg_arr[msg_queue->iffc++] = message;
  }
  return SUCCES;
}

int preceive(int fid, int * message) {

}
