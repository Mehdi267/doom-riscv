#ifndef MSGQUEUE_H_
#define MSGQUEUE_H_
#include "queue.h"

typedef struct msg_queue_t {
  int * msg_arr;
  size_t size; // size of the array
  int oldmi; // the index of the oldest message in the arr
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

int pcreate(int count);
int pdelete(int fid);
int psend(int fid, int message);
int preceive(int fid, int *message);
#endif // MSGQUEUE_H_
