/*******************************************************************************
 * Ensimag - Projet Systeme
 * Copyright 2013 - Damien Dejean <dam.dejean@gmail.com>
 * Test 22
 *
 * Checks the state of CPU's Translation Lookaside Buffer when the kernel does
 * dynamic memory mapping for a process.
 ******************************************************************************/

#include "tests.h"
#include "test22.h"

int malicious(void *arg)
{
        unsigned *shared_area = NULL;

        assert(arg == NULL);

        shared_area = shm_acquire("test22-shm");
        assert(shared_area != NULL);

        /* The parent process should have filled the shared area */
        assert(*shared_area == MAGIC_COOKIE);

        /* Write in the memory to force w flag in the TLB */
        *shared_area = MAGIC_COOKIE;

        /*
         * Try to fool the kernel: shm memory is usually done by dynamic memory
         * mapping, if TLB is not cleaned up after unmap, we should keep the
         * access to shared memory even if the entry is no more valid in page
         * directory/table !
         */

        /* Unmap */
        printf("%s", "  Unmapping shared area ... ");
        shm_release("test22-shm");
        printf("%s\n", "OK");

        /* Try a read */
        printf("%s", "  Try a read ... ");
        assert(*shared_area == MAGIC_COOKIE);
        printf("%s\n", " done, this is wrong, I should have been killed ! ");

        /* Try a write */
        printf("%s", "  Try a write ... ");
        *shared_area = 0xDEADB00B;
        printf("%s\n", " done, this is wrong, I should have been killed ! ");

        /*
         * Page is not really unmapped until the TLB is cleaned up ! Fix your
         * kernel !
         */

        return (int)MALICIOUS_SUCCESS;
}


int pagefault(void *arg)
{
        (void)arg;
        /* Try to fault */
        *((int*)0) = 0;
        /* We should have been killed ... */
        return (int)MALICIOUS_SUCCESS;
}


int test22(void *arg)
{
        (void)arg;
        int pagefault_pid = -1;
        int pagefault_ret = -1;
        unsigned *shared_area = NULL;
        int malicious_pid = -1;
        int malicious_ret = -1;

        /*
         * Check page fault handling.
         */
        printf("\n%s\n", "Test 22: checking page fault handling...");
        pagefault_pid = start(pagefault, 4000, getprio(getpid()) - 1, "pagefault", NULL);

        waitpid(pagefault_pid, &pagefault_ret);

        /* "pagefault" should have been killed */
        switch (pagefault_ret) {
                case 0:
                        printf("-> %s\n-> %s\n", "\"pagefault\" process killed.", "TEST PASSED");
                        break;

                case (int)MALICIOUS_SUCCESS:
                        printf("-> %s\n-> %s\n", "\"pagefault\" process should not ends correctly.", "TEST FAILED");
                        break;
                default:
                        printf("-> %s\n-> %s\n", "unexpected return value for \"pagefault\" process. Check waitpid and kill behaviors", "TEST FAILED");
        }


        /*
         * Check virtual memory mappings consistency.
         */
        shared_area = shm_create("test22-shm");
        assert(shared_area != NULL);

        /* The malicious process will check this value */
        *shared_area = MAGIC_COOKIE;

        printf("%s\n", "Test 22: checking shared memory mappings ...");
        malicious_pid = start(malicious, 4000, getprio(getpid()) - 1, "malicious", NULL);

        waitpid(malicious_pid, &malicious_ret);

        /* "malicious" should have been killed */
        switch (malicious_ret) {
                case 0:
                        printf("-> %s\n-> %s\n", "\"malicious\" process killed.", "TEST PASSED");
                        break;

                case (int)MALICIOUS_SUCCESS:
                        printf("-> %s\n-> %s\n", "\"malicious\" process should not ends correctly.", "TEST FAILED");
                        break;
                default:
                        printf("-> %s\n-> %s\n", "unexpected return value for \"malicious\" process. Check waitpid and kill behaviors", "TEST FAILED");
        }

        /* and should not have been able to modify shared memory */
        if (*shared_area != MAGIC_COOKIE) {
                printf("-> %s\n-> %s\n", "\"malicious\" have been able to write unmapped memory.", "TEST FAILED");
        }

        shm_release("test22-shm");
        return 0;
}
