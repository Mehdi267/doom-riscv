#ifndef MSGQUEUE_H_
#define MSGQUEUE_H_
#include "queue.h"

typedef struct msg_queue_t {
  int * msg_arr;
  link blocked_process;
} msg_queue_t;

int pcreate(int count);

int pdelete(int fid);
#endif // MSGQUEUE_H_
