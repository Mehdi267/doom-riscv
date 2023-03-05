#include "tests.h"

#include "stdio.h"
#include "assert.h"

int prio4(void *arg)
{
        /* arg = priority of this proc. */
        int r;
        assert(getprio(getpid()) == cast_pointer_into_a_int(arg));
        printf("1");
        r = chprio(getpid(), 64);
        assert(r == cast_pointer_into_a_int( arg));
        printf(" 3");
        return 0;
}



int prio5(void *arg)
{
        /* Arg = priority of this proc. */
        int r;

        assert(getprio(getpid()) == cast_pointer_into_a_int( arg));
        printf(" 7");
        r = chprio(getpid(), 64);
        assert(r == cast_pointer_into_a_int(arg));
        printf("error: I should have been killed\n");
        assert(0);
        return 0;
}



int test3(void *arg)
{
    int pid1;
    int p = 192;
    int r;

    (void)arg;

    assert(getprio(getpid()) == 128);
    pid1 = start(prio4, 4000, p,"prio4", cast_int_to_pointer(p));
    assert(pid1 > 0);
    printf(" 2");
    r = chprio(getpid(), 32);
    assert(r == 128);
    printf(" 4");
    r = chprio(getpid(), 128);
    assert(r == 32);
    printf(" 5");
    assert(waitpid(pid1, 0) == pid1);
    printf(" 6");

    assert(getprio(getpid()) == 128);
    pid1 = start(prio5, 4000, p,"prio5",cast_int_to_pointer(p));
    assert(pid1 > 0);
    printf(" 8");
    r = kill(pid1);
    assert(r == 0);
    assert(waitpid(pid1, 0) == pid1);
    printf(" 9");
    r = chprio(getpid(), 32);
    assert(r == 128);
    printf(" 10");
    r = chprio(getpid(), 128);
    assert(r == 32);
    printf(" 11.\n");
    return 0;
}


