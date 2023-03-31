#include "tests.h"
#include "assert.h"

/* Test d'utilisation d'un semaphore comme simple compteur. */
int test_sem(void *arg)
{
        int sem1;
        (void)arg;
        sem1 = screate(2);
        assert(sem1 >= 0);
        assert(scount(sem1) == 2);
        assert(signal(sem1) == 0);
        assert(scount(sem1) == 3);
        assert(signaln(sem1, 2) == 0);
        assert(scount(sem1) == 5);
        assert(wait(sem1) == 0);
        assert(scount(sem1) == 4);
        assert(sreset(sem1, 7) == 0);
        assert(scount(sem1) == 7);
        assert(sdelete(sem1) == 0);
        printf("ok.\n");
        return 0;
}