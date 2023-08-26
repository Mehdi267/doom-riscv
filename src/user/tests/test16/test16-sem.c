/*******************************************************************************
 * Test 16
 *
 * Allocation performance.
 ******************************************************************************/

#include "sysapi.h"

int main(void *arg)
{
        int pid;
        (void)arg;
        pid = start("proc16_1", 4000 + NBSEMS * 4, 128, 0);
        assert(pid > 0);
        assert(waitpid_old(pid, 0) == pid);
        return 0;
}
