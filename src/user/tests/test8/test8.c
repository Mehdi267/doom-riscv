/*******************************************************************************
 * Test 8
 *
 * Creation de processus se suicidant en boucle. Test de la vitesse de creation
 * de processus.
 ******************************************************************************/

#include "sysapi.h"
int main(void *arg)
{
        unsigned long long tsc1;
        unsigned long long tsc2;
        int i, pid, count;
        long int r;

        (void)arg;
        assert(getprio(getpid()) == 128);

        /* Le petit-fils va passer zombie avant le fils mais ne pas
           etre attendu par waitpid_old. Un nettoyage automatique doit etre
           fait. */
        pid = start("suicide_launcher", 4000, 129, 0);
        assert(pid > 0);
        assert(waitpid_old(pid, &r) == pid);
        assert(chprio((int)r, 192) < 0);

        count = 0;
        __asm__ __volatile__("rdcycle %0":"=r"(tsc1));
        do {
                for (i=0; i<10; i++) {
                        pid = start("suicide_launcher", 4000, 200, 0);
                        assert(pid > 0);
                        assert(waitpid_old(pid, 0) == pid);
                }
                test_it();
                count += i;
                __asm__ __volatile__("rdcycle %0":"=r"(tsc2));
        } while ((tsc2 - tsc1) < 1000000000);
        printf("%lld cycles/process.\n", (tsc2 - tsc1) /  (2 * (unsigned)count));
        return 0;
}

