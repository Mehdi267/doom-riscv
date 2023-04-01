/*******************************************************************************
 * Test 13
 *
 * Teste l'ordre entre les processus emetteurs et recepteurs sur une file.
 * Teste le changement de priorite d'un processus bloque sur une file.
 ******************************************************************************/

#include "../process/process.h"
#include "../process/memory_api.h"
#include "msgqueue.h" // for p funcs
#include "stdio.h"
#include "assert.h"
#include "stdint.h"
#include "tests.h"
#include "stdint.h"
#include "string.h"

struct psender {
  int fid;
  char data[32];
};

int psender(void *arg) {
  struct psender *ps = NULL;
  int64_t ps_index = (int64_t)arg;
  unsigned i;
  size_t n;

  ps = shm_acquire("test13_shm");
  assert(ps != NULL);
  n = strlen(ps[ps_index].data);

  for (i = 0; i < n; i++) {
    assert(psend(ps[ps_index].fid, ps[ps_index].data[i]) == 0);
  }
  shm_release("test13_shm");
  return 0;
}

int preceiver(void *arg) {
  struct psender *ps = NULL;
  int64_t ps_index = (int64_t)arg;
  int msg;
  unsigned i;
  size_t n;

  ps = shm_acquire("test13_shm");
  assert(ps != NULL);
  n = strlen(ps[ps_index].data);

  for (i = 0; i < n; i++) {
    assert(preceive(ps[ps_index].fid, &msg) == 0);
    assert(msg == ps[ps_index].data[i]);
  }

  shm_release("test13_shm");
  return 0;
}
int test13(void *arg) {
  struct psender *ps = NULL;
  int pid1, pid2, pid3;
  int fid = pcreate(3);
  int i, msg;

  (void)arg;
  ps = (struct psender *)shm_create("test13_shm");
  assert(ps != NULL);

  printf("1");
  assert(getprio(getpid()) == 128);
  assert(fid >= 0);
  ps[1].fid = ps[2].fid = ps[3].fid = fid;
  strncpy(ps[1].data, "abcdehm", 32);
  strncpy(ps[2].data, "il", 32);
  strncpy(ps[3].data, "fgjk", 32);
  pid1 = start(&psender, 4000, 131, "psender1", (void *)1);
  pid2 = start(&psender, 4000, 130, "psender2", (void *)2);
  pid3 = start(&psender, 4000, 129, "psender3", (void *)3);
  for (i = 0; i < 2; i++) {
    assert(preceive(fid, &msg) == 0);
    assert(msg == 'a' + i);
  }
  chprio(pid1, 129);
  chprio(pid3, 131);
  for (i = 0; i < 2; i++) {
    assert(preceive(fid, &msg) == 0);
    assert(msg == 'c' + i);
  }
  chprio(pid1, 127);
  chprio(pid2, 126);
  chprio(pid3, 125);
  for (i = 0; i < 6; i++) {
    assert(preceive(fid, &msg) == 0);
    assert(msg == 'e' + i);
  }
  chprio(pid1, 125);
  chprio(pid3, 127);
  for (i = 0; i < 3; i++) {
    assert(preceive(fid, &msg) == 0);
    assert(msg == 'k' + i);
  }
  assert(waitpid(pid3, 0) == pid3); // XXX assert(waitpid(-1, 0) == pid3); ???
  assert(waitpid(-1, 0) == pid2);
  assert(waitpid(-1, 0) == pid1);
  printf(" 2");

  strncpy(ps[1].data, "abej", 32);
  strncpy(ps[2].data, "fi", 32);
  strncpy(ps[3].data, "cdgh", 32);
  pid1 = start(&preceiver, 4000, 131, "preceiver1", (void *)1);
  pid2 = start(&preceiver, 4000, 130, "preceiver2", (void *)2);
  pid3 = start(&preceiver, 4000, 129, "preceiver3", (void *)3);
  for (i = 'a'; i <= 'b'; i++) {
    assert(psend(fid, i) == 0);
  }
  chprio(pid1, 129);
  chprio(pid3, 131);
  for (i = 'c'; i <= 'd'; i++) {
    assert(psend(fid, i) == 0);
  }
  chprio(pid1, 127);
  chprio(pid2, 126);
  chprio(pid3, 125);
  for (i = 'e'; i <= 'j'; i++) {
    assert(psend(fid, i) == 0);
  }
  chprio(pid1, 125);
  chprio(pid3, 127);
  assert(waitpid(-1, 0) == pid3);
  assert(waitpid(-1, 0) == pid2);
  assert(waitpid(-1, 0) == pid1);
  assert(pdelete(fid) == 0);
  printf(" 3.\n");
  shm_release("test13_shm");
  return 0;
}
