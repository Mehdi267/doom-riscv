/*******************************************************************************
 * Test 17
 *
 * On teste des limites de capacite
 ******************************************************************************/

#include "sysapi.h"

static int ids[1200];

static const int heap_len = 200 << 20;

int test17()
{
        int i, n, nx;
        int l = sizeof(ids) / sizeof(int);
        int count;
        int prio;

        n = 0;
        while (1) {
                int fid = pcreate(1);
                if (fid < 0) break;
                ids[n++] = fid;
                if (n == l) {
                        assert(!"Maximum number of queues too high !");
                }
                test_it();
        }
        for (i=0; i<n; i++) {
                assert(pdelete(ids[i]) == 0);
                test_it();
        }
        for (i=0; i<n; i++) {
                int fid = pcreate(1);
                assert(fid >= 0);
                ids[i] = fid;
                test_it();
        }
        assert(pcreate(1) < 0);
        for (i=0; i<n; i++) {
                assert(pdelete(ids[i]) == 0);
                test_it();
        }
        printf("%d", n);

        for (i=0; i<n; i++) {
                int fid = pcreate(1);
                assert(fid >= 0);
                assert(psend(fid, i) == 0);
                ids[i] = fid;
                test_it();
        }
        assert(pcreate(1) < 0);
        for (i=0; i<n; i++) {
                int msg;
                assert(preceive(ids[i], &msg) == 0);
                assert(msg == i);
                assert(pdelete(ids[i]) == 0);
                test_it();
        }

        count = heap_len / (int)sizeof(int);
        count /= n - 1;
        nx = 0;
        while (nx < n) {
                int fid = pcreate(count);
                if (fid < 0) break;
                ids[nx++] = fid;
                test_it();
        }
        assert(nx < n);
        for (i=0; i<nx; i++) {
                assert(pdelete(ids[i]) == 0);
                test_it();
        }
        printf(" > %d", nx);

        prio = getprio(getpid());
        assert(prio == 128);
        n = 0;
        while (1) {
                int pid = start("no_run", 2000, 127, 0);
                // printf("n = %d l = %d \n", n, l);
                if (pid < 0) break;
                ids[n++] = pid;
                // printf("after\n");
                if (n == l) {
                        assert(!"Maximum number of processes too high !");
                }
                test_it();
        }
        for (i=0; i<n; i++) {
                assert(kill(ids[i]) == 0);
                assert(waitpid_old(ids[i], 0) == ids[i]);
                test_it();
        }
        int old_n = n;
        n = 0;
        while (1) {
                int pid = start("no_run", 2000, 127, 0);
                // printf("n = %d l = %d \n", n, l);
                if (pid < 0) break;
                // printf("n = %d\n", n);
                ids[n++] = pid;
                // printf("after\n");
                if (n == l) {
                        assert(!"Maximum number of processes too high !");
                }
                test_it();
        }
        for (i=0; i<n; i++) {
                assert(kill(ids[i]) == 0);
                assert(waitpid_old(ids[i], 0) == ids[i]);
                test_it();
        }
        assert(old_n == n);
        for (i=0; i<n; i++) {
                int pid = start("proc_return", 2000, 129, (void *)i);
                assert(pid > 0);
                ids[i] = pid;
                test_it();
        }
        for (i=0; i<n; i++) {
                long int retval;
                assert(waitpid_old(ids[i], &retval) == ids[i]);
                assert(retval == i);
                test_it();
        }
        printf(", %d.\n", n);
        return 0;
}


int main(){  
  for (int i = 0; i<5; i++){
    test17();
  }
  return 0;
}