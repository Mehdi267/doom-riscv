#include "tests.h"

#include "stdio.h"
#include "assert.h"


int procExit(void *args)
{
        printf(" 5");
        exit_process((int) ((long)args));
        assert(0);
        return 0;
}


int procKill(void *args)
{
        printf(" X");
        return (int)(long)args;
}


int test2(void *arg)
{
        int rval;
        int r;
        int pid1;
        int val = 45;

        (void)arg;

        printf("1");
        pid1 = start(procKill, 4000, 100, "procKill", cast_int_to_pointer( val));
        assert(pid1 > 0);
        printf(" 2");
        r = kill(pid1);
        assert(r == 0);
        printf(" 3");
        r = waitpid(pid1, &rval);
        assert(rval == 0);
        assert(r == pid1);
        printf(" 4");
        pid1 = start(procExit, 4000, 192, "procExit", cast_int_to_pointer(val));
        assert(pid1 > 0);
        printf(" 6");
        r = waitpid(pid1, &rval);
        assert(rval == val);
        assert(r == pid1);
        assert(waitpid(getpid(), &rval) < 0);
        printf(" 7.\n");
        return 0;
}

