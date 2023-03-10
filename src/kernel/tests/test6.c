/*******************************************************************************
 * Test 6
 *
 * wait_clock : le programme principal lance 4 processus P1 à P4
 * et exécute un wait_clock d'une minute ;
 * - le processus P1 imprime le caractère . à raison de un par seconde
 * - le processus P2 imprime le caractère - toutes les 2 secondes ;
 * - le processus P3 imprime le caractère + toutes les 5 secondes ;
 * - le processus P4 imprime le caractère * toutes les 10 secondes ;
 * - la périodicité est gérée par un appel à wait_clock dans tous les cas ;
 * - à son réveil, le processus principal tue ses fils et se termine. :
 *  On doit voir une alternance correcte des différents caractères.
 ******************************************************************************/

#include "../sync.h"
#include "stdio.h"
#include "assert.h"
#include "tests.h"

int p1(void *arg) {
  for (;;) {
    printf(".");
    sleep(1); // 1 sec;
  }
}

int p2(void *arg) {
  for (;;) {
    printf("-");
    sleep(2); // 2 sec;
  }
}

int p3(void *arg) {
  for (;;) {
    printf("+");
    sleep(5); // 5 sec;
  }
}

int p4(void *arg) {
  for (;;) {
    printf("*");
    sleep(10); // 10 sec;
  }
}

int test6(void *arg) {
  int pid[4];

  (void)arg;
  pid[0] = start(p1, 4000, 192, "p1", NULL);
  assert(pid[0] > 0);
  pid[1] = start(p2, 4000, 182, "p2", NULL);
  assert(pid[1] > 0);
  pid[2] = start(p3, 4000, 172, "p3", NULL);
  assert(pid[2] > 0);
  pid[3] = start(p4, 4000, 162, "p4", NULL);
  assert(pid[3] > 0);

  sleep(60);
  // kill the sons on waking
  for (int i = 0; i <= 4; i++) {
    kill(pid[i]);
  }
  return 0;
}
