#ifndef MSGQUEUE_H_
#define MSGQUEUE_H_
#include "queue.h"

typedef struct msg_queue_t {
  int * msg_arr;
  size_t size; // the size of the msg_arr
  unsigned int iffc; // index of the first free case
  link blocked_cons;
  link blocked_prod;
} msg_queue_t;

typedef enum status {
  RECEIVED,
  ERROR
} message_status;


typedef struct message_t {
  int message;
  message_status status;
} message_t;
int pcreate(int count);
int pdelete(int fid);
int psend(int fid, int message);
#endif // MSGQUEUE_H_
