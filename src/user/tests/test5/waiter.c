#include "sysapi.h"

int main(void *arg)
{
        int pid = (int)arg;
        assert(kill(pid) == 0);
        assert(waitpid_old(pid, 0) < 0);
        return 1;
}

