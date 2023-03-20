/*******************************************************************************
 * Test 7
 *
 * Test de l'horloge (ARR et ACE)
 * Tentative de determination de la frequence du processeur et de la
 * periode de scheduling
 ******************************************************************************/


#include "tests.h"

#include "stdio.h"
#include "assert.h"


int acquire(void *arg)
{
        volatile unsigned long *page = NULL;
        page = shm_acquire("test_memory");
        assert(page != NULL);
        *page = 0xdeadbeef;
        shm_release("test_memory");
        return 0;
}



int test_memory(void *arg)
{
        int pid1;
        volatile unsigned long *page = NULL;

        page = shm_create("test_memory");
        assert(page != NULL);
        pid1 = start(acquire, 4000, 127, "acquire", 0);
        assert(pid1 > 0);
        assert(waitpid(pid1, NULL) == pid1);
        assert(*page == 0xdeadbeef);
        shm_release("test_memory");
        assert(shm_acquire("test_memory")==NULL);
        return 0;
}
