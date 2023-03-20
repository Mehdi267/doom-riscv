#include "msgqueue.h"
#include "process/scheduler.h"
#include "assert.h"
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
  all_queues[index]->msg_arr = malloc(sizeof(int) * count);
  link new_head = LIST_HEAD_INIT(new_head);
  all_queues[index]->blocked_process = new_head;
  return index;
}

int pdelete(int fid) {
  if (fid < 0 || fid >= NBQUEUE)
    return FAILURE;
  // destruction of the msg queue
  free(all_queues[fid]->msg_arr);
  // for each blocked process make it activatable
  process* curr;
  queue_for_each(curr, &all_queues[fid]->blocked_process, process, next_prev) {
    curr->state = ACTIVATABLE;
    queue_add(curr, &activatable_process_queue, process, next_prev, prio);
  }
  return SUCCES;
}

/* La primitive psend envoie le message dans la file identifiée par */
/* fid. On distingue trois cas : */

/* Si la file est vide et que des processus sont bloqués en attente de */
/* message, alors le processus le plus ancien dans la file parmi les */
/* plus prioritaires est débloqué et reçoit ce message. */
/* Si la file est pleine, le processus appelant passe dans l'état */
/* bloqué sur file pleine jusqu'à ce qu'une place soit disponible */
/* dans la file pour y mettre le message. */
/* Sinon, la file n'est pas pleine et aucun processus n'est bloqué en */
/* attente de message. Le message est alors déposé directement dans la */
/* file. */

/* Il est possible également, qu'après avoir été mis dans l'état bloqué */
/* sur file pleine, le processus soit remis dans l'état activable par un */
/* autre processus ayant exécuté */
/* preset */
/* ou pdelete. */
/* Dans ce cas, la valeur de retour de psend est strictement négative. */
/* Si la valeur de fid est invalide, la valeur de retour est négative */
/* sinon elle est nulle. */
/* int psend(int fid, int message) { */

/* } */
