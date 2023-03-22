/*******************************************************************************
 * Test 13
 *
 * sreset, sdelete
 ******************************************************************************/

#include "tests.h"
#include "assert.h"

int proc13_1(void *arg)
{
        int sem = cast_pointer_into_a_int(arg);
        assert(try_wait(sem) == 0);
        assert(try_wait(sem) == -3);
        printf("1");
        assert(wait(sem) == -4);
        printf(" 9");
        assert(wait(sem) == -3);
        printf(" 13");
        assert(wait(sem) == -1);

        exit_process(1);
        return 1;
}

int proc13_2(void *arg)
{
        int sem = cast_pointer_into_a_int(arg);
        printf(" 5");
        assert(wait(sem) == -4);
        printf(" 11");
        assert(wait(sem) == -3);
        printf(" 15");
        assert(wait(sem) == -1);

        return 2;
}


int proc13_3(void *arg)
{
        int sem = cast_pointer_into_a_int(arg);
        printf(" 3");
        assert(wait(sem) == -4);
        printf(" 7");
        assert(wait(sem) == 0);
        printf(" 8");
        assert(wait(sem) == -3);
        printf(" 12");
        assert(wait(sem) == -1);
        exit_process(3);
        assert(!"Should not arrive here !");
        while(1);
        return 0;
}

int test13_sem(void *arg)
{
        int sem;
        int pid1, pid2, pid3;
        int ret;

        (void)arg;

        assert(getprio(getpid()) == 128);
        assert((sem = screate(1)) >= 0);
        pid1 = start(proc13_1, 4000, 129,"proc13_1", cast_int_to_pointer(sem));
        assert(pid1 > 0);
        printf(" 2");
        pid2 = start(proc13_2, 4000, 127,"proc13_2", cast_int_to_pointer(sem));
        assert(pid2 > 0);
        pid3 = start(proc13_3, 4000, 130,"proc13_3", cast_int_to_pointer(sem));
        assert(pid3 > 0);
        printf(" 4");
        assert(chprio(getpid(), 126) == 128);
        printf(" 6");
        assert(chprio(getpid(), 128) == 126);
        assert(sreset(sem, 1) == 0);
        printf(" 10");
        assert(chprio(getpid(), 126) == 128);
        assert(chprio(getpid(), 128) == 126);
        assert(sdelete(sem) == 0);
        printf(" 14");
        assert(waitpid(pid1, &ret) == pid1);
        assert(ret == 1);
        assert(waitpid(-1, &ret) == pid3);
        assert(ret == 3);
        assert(waitpid(-1, &ret) == pid2);
        assert(ret == 2);
        assert(signal(sem) == -1);
        assert(scount(sem) == -1);
        assert(sdelete(sem) == -1);
        printf(" 16.\n");
        return 0;
}
