#include "msgqueue.h"
#include "assert.h"
#include "process/helperfunc.h"
#include "process/process.h"
#include "process/scheduler.h"
#include "stdint.h"
#include "stdio.h"
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
  if (count<0){
    return -1;
  }
  int index;
  if ((index = get_first_free_index()) < 0)
    return -1;
  secmalloc(all_queues[index], sizeof(msg_queue_t));
  secmalloc(all_queues[index]->msg_arr, sizeof(int) * count);
  all_queues[index]->size = count;
  all_queues[index]->iffc = 0;
  all_queues[index]->number_of_msgs = 0;
  INIT_LIST_HEAD(&all_queues[index]->blocked_cons);
  INIT_LIST_HEAD(&all_queues[index]->blocked_prod);
  return index;
}


bool valid_fid(int fid) {
  if (fid < 0 || fid >= NBQUEUE || all_queues[fid] == NULL)
    return false;
  return true;
}


int pdelete(int fid) {
  if (!valid_fid(fid))
    return FAILURE;
  // destruction of the msg queue
  free(all_queues[fid]->msg_arr);
  // for each blocked consummer process make it activatable
  free_blocked_queue(&all_queues[fid]->blocked_cons);
  // for each blocked producer process make it activatable
  free_blocked_queue(&all_queues[fid]->blocked_prod);
  free(all_queues[fid]);
  all_queues[fid] = NULL;
  scheduler();
  return SUCCES;
}

int psend(int fid, int message) {
  if (!valid_fid(fid))
    return FAILURE;
  msg_queue_t * msg_queue = all_queues[fid];
  link * blocked_consumers = &(msg_queue->blocked_cons);
  link * blocked_producers = &(msg_queue->blocked_prod);
  // the queue is empty and there are blocked consumers
  if (is_empty(msg_queue) && !queue_empty(blocked_consumers)) {
    process * blocked = queue_out(blocked_consumers, process, next_prev);
    blocked->state = ACTIVATABLE;
    queue_add(blocked, &activatable_process_queue, process, next_prev, prio);
    blocked->message.value = message;
    blocked->message.status = RECEIVED;
    scheduler();
  } else if (is_full(msg_queue)) {
    // the msg_queue is full
    process * p = get_current_process();
    p->message.value = message;
    p->state = BLOCKEDQUEUE;
    p->message.blocked_head = blocked_producers;
    queue_add(p, blocked_producers, process, next_prev, prio);
    scheduler();
    if (p->message.status != SENT)
      return FAILURE;
  } else {
    // no process is blocked and there is space in the msgqueue
    add_message(msg_queue, message);
  }
  return SUCCES;
}

int preceive(int fid, int *message) {
  if (!valid_fid(fid))
    return FAILURE;

  msg_queue_t * msg_queue = all_queues[fid];
  link * blocked_producers = &(msg_queue->blocked_prod);
  link * blocked_consumers = &(msg_queue->blocked_cons);
  if (is_empty(msg_queue) && !queue_empty(blocked_producers)) {
    // the queue can be empty but there are blocked producers who couldn't write
    // bc they have minor prio.
    process * blocked = queue_out(blocked_producers, process, next_prev);
    blocked->message.status = SENT;
    if (message != NULL)
      *message = blocked->message.value;
    queue_add(blocked, &activatable_process_queue, process, next_prev, prio);
    scheduler();
  } else if (is_empty(msg_queue) &&  queue_empty(blocked_producers)) {
    // the msg_queue is empty
    process * p = get_current_process();
    p->state = BLOCKEDQUEUE;
    p->message.blocked_head = blocked_consumers;
    queue_add(p, blocked_consumers, process, next_prev, prio);
    scheduler();
    if (p->message.status != RECEIVED) {
      return FAILURE;
    }
    if (message != NULL)
      *message = p->message.value;
  } else if (is_full(msg_queue) && !queue_empty(blocked_producers)) {
    int msg = pop_oldest_msg(msg_queue);
    if (message != NULL)
      *message = msg;
    process * blocked = queue_out(blocked_producers, process, next_prev);
    add_message(msg_queue, blocked->message.value);
    blocked->message.status = SENT;
    blocked->state = ACTIVATABLE;
    queue_add(blocked, &activatable_process_queue, process, next_prev, prio);
    scheduler();
    /* assert(blocked->message.status == RECEIVED); */
    /* *message = blocked->message.value; */
  } else {
    // the queue is not empty and there arent blocked consumers
    int msg = pop_oldest_msg(msg_queue);
    if (message != NULL)
      *message = msg;
  }
  return SUCCES;

}


size_t queue_length(link * queue) {
  size_t len = 0;
  process * it;
  queue_for_each(it, queue, process, next_prev) {
    len++;
  }
  return len;
}


int pcount(int fid, int *count) {
  if (!valid_fid(fid) || count == NULL)
    return FAILURE;
  msg_queue_t * msg_queue = all_queues[fid];
  if (!queue_empty(&(msg_queue->blocked_cons)))
    *count = -queue_length(&(msg_queue->blocked_cons));
  else
    *count = msg_queue->number_of_msgs + queue_length(&(msg_queue->blocked_prod));
  return SUCCES;

}

int preset(int fid) {
  if (!valid_fid(fid))
    return FAILURE;
  msg_queue_t * msg_queue = all_queues[fid];
  msg_queue->number_of_msgs = 0;
  msg_queue->iffc = 0;
  free_blocked_queue(&(msg_queue->blocked_cons));
  free_blocked_queue(&(msg_queue->blocked_prod));
  scheduler();
  return SUCCES;
}


/*
** HELPER FUNCTIONS DEFINITION
*/
void free_blocked_queue(link * queue) {
  // for each blocked process make it activatable
  while(!queue_empty(queue)) {
    process* curr = queue_out(queue, process, next_prev);
    curr->message.status = ERROR;
    curr->state = ACTIVATABLE;
    // reset pointers
    curr->next_prev.next = 0;
    curr->next_prev.prev = 0;
    queue_add(curr, &activatable_process_queue, process, next_prev, prio);
  }
}

bool is_full(msg_queue_t * msg_queue) {
  return msg_queue->number_of_msgs == msg_queue->size;
}

bool is_empty(msg_queue_t * msg_queue) {
  assert(msg_queue->msg_arr != NULL);
  return msg_queue->number_of_msgs == 0;
}

int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

// TODO: check if possible interrupts can make problems
int pop_oldest_msg(msg_queue_t * msg_queue) {
  int index = mod((msg_queue->iffc - (int)msg_queue->number_of_msgs), (int)msg_queue->size);
  msg_queue->number_of_msgs--;
  return msg_queue->msg_arr[index];
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
  msg_queue->number_of_msgs++;
  msg_queue->iffc = (msg_queue->iffc + 1) % msg_queue->size;
}

void debug_queue(size_t n) {
  process * curr;
  printf("Blocked cons\n");
  queue_for_each(curr, &(all_queues[n]->blocked_cons), process, next_prev) {
    printf("%s -> ", curr->process_name);
  }
  printf("\nBlocked prods\n");
  printf("\nhead: %p\n", &(all_queues[n]->blocked_prod));
  queue_for_each(curr, &(all_queues[n]->blocked_prod), process, next_prev) {
    printf("{%s, %d, %p} -> ", curr->process_name, curr->prio, &(curr->next_prev));
  }
  printf("\nMSG in the arr : [");
  for (int i = 0; i < all_queues[n]->size; i++) {
    printf("%c, ", all_queues[n]->msg_arr[i]);
  }
  printf("]\n");
}
