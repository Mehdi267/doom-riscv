/*******************************************************************************
 * Test 16
 *
 * Allocation performance.
 ******************************************************************************/

#include "tests.h"
#include "assert.h"
#include "test16-sem.h"
#include "sysapi_kernel.h"
#include "drivers/clint.h"

static unsigned long test16_1(void)
{
        unsigned long long tsc, tsc1, tsc2;
        unsigned long count = 0;

        // __asm__ __volatile__("rdtsc":"=A"(tsc1));
        tsc1 = get_stime();
        tsc2 = tsc1 + 10000000;
        assert(tsc1 < tsc2);
        do {
                unsigned i;
                test_it();
                for (i=0; i<100; i++) {
                        int sem1 = screate(2);
                        int sem2 = screate(1);
                        if (!(sem1 >= 0)){
                          printf("id %d\n",sem1);
                          printf("id %d\n",sem2);
                        }
                        assert(sem1 >= 0);
                        assert(sem2 >= 0);
                        assert(sem1 != sem2);
                        assert(sdelete(sem1) == 0);
                        assert(sdelete(sem2) == 0);
                }
                //puts("iter");
                // __asm__ __volatile__("rdtsc":"=A"(tsc));
                        tsc = get_stime();
                count += 2 * i;

        } while (tsc < tsc2);
        return (unsigned long)div64(tsc - tsc1, count, 0);
}

int proc16_1(void *arg)
{
        int sems[NBSEMS];
        int i;
        unsigned long c1, c2;
        unsigned long long seed;

        (void)arg;

        c1 = test16_1();

        printf("semaphores have been created and deleted succefully%lu \n", c1);
        // __asm__ __volatile__("rdtsc":"=A"(seed));
        seed = get_stime();
        setSeed(seed);
        for (i=0; i<NBSEMS; i++) {
                int sem = screate(randShort());
                if (sem < 0) assert(!"*** Increase the semaphore capacity of your system to NBSEMS to pass this test. ***");
                sems[i] = sem;
        }
        if (screate(0) >= 0) assert(!"*** Decrease the semaphore capacity of your system to NBSEMS to pass this test. ***");
        assert(sdelete(sems[NBSEMS/3]) == 0);
        assert(sdelete(sems[(NBSEMS/3)*2]) == 0);
        printf("calling test 16 again ....\n");
        c2 = test16_1();
        printf("%lu ", c2);
        setSeed(seed);
        for (i=0; i<NBSEMS; i++) {
                short randVal = randShort();
                if ((i != (NBSEMS/3)) && (i != (2*(NBSEMS/3)))) {
                        assert(scount(sems[i]) == randVal);
                        assert(sdelete(sems[i]) == 0);
                }
        }
        if (c2 < 2 * c1)
                printf("ok.\n");
        else
                printf("Bad algorithm complexity in semaphore allocation.\n");
        return 0;
}



int test16_sem(void *arg)
{
        int pid;
        (void)arg;
        pid = start(proc16_1, 4000 + NBSEMS * 4, 128, "proc16_1", 0);
        assert(pid > 0);
        assert(waitpid(pid, 0) == pid);
        return 0;
}
