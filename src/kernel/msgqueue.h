#ifndef MSGQUEUE_H_
#define MSGQUEUE_H_
#include "queue.h"

typedef struct msg_queue_t {
  int * msg_arr;
  size_t size; // size of the array
  size_t number_of_msgs;
  int iffc; // index of the first free case
  link blocked_cons;
  link blocked_prod;
} msg_queue_t;

typedef enum status {
  RECEIVED,
  ERROR,
  SENT
} message_status;

typedef struct message_t {
  int value;
  message_status status;
} message_t;

/**
 * @brief allocation of a queue of the capacity count
 * @return the SUCCES if there were no errors and a FAILURE if count is negative
 * or if there aren't more queues available
 */
int pcreate(int count);

/**
 * @brief destruction of the message queue if there were blocked process they
 * become activable and they value of psend and preceive will be FAILURE
 * @return returns FAILURE if the value fid is invalid and SUCCES if not
 */
int pdelete(int fid);

/**
 * @brief sends the message in the queue identified with fid
 * become activable and they value of psend and preceive will be FAILURE
 * @param fid: the id of the msg queue
 * @param message: the pointer where to write the message
 * @note if the queue is full process can be blocked
 * @return returns FAILURE if the process was blocked and another process
 * executed preset or pdelete, and otherwise SUCCES
 */

int psend(int fid, int message);

/**
 * @brief reads the oldest message in the queue fid, and writes it in *message
 * if its not NULL
 * @param fid: the id of the msg queue
 * @param message: the pointer where to write the message
 * @note process can be blocked if the queue is empty
 * @return returns FAILURE if the process was blocked and another process
 * executed preset or pdelete, otherwise SUCCES
 */
int preceive(int fid, int *message);

/**
 * @brief if count is not NULL two different cases:
 * if the queue is empty count get -(number of blocked consumers)
 * ifnot count gets the value of (number of blocked produres + nb of msgs)
 * @return returns FAILURE if the value fid is invalid and SUCCES if not
*/
int pcount(int fid, int *count);

#endif // MSGQUEUE_H_
