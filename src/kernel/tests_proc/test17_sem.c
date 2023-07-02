/*******************************************************************************
 * Test 17
 *
 * Un exemple de producteur/consommateur
 * On peut aussi faire sans operation atomique
 ******************************************************************************/

#include "test17-sem.h"
#include "tests.h"
#include <stdint.h>
#include <stdio.h>

static void buf_send(char x, struct test17_buf_st *st)
{
    assert(wait(st->wsem) == 0);
    assert(wait(st->mutex) == 0);
    st->buf[(st->wpos++) % sizeof(st->buf)] = x;
    assert(signal(st->mutex) == 0);
    assert(signal(st->rsem) == 0);
}

int proc17_1(void *arg)
{
        struct test17_buf_st *st = NULL;
        unsigned long long tsc, tsc2;
        int count;

        (void)arg;

        st = (struct test17_buf_st*) shm_acquire("test17_shm");
        // __asm__ __volatile__("rdtsc":"=A"(tsc));
        tsc = get_stime(); 
        tsc2 = tsc + 10000;
        assert(tsc < tsc2);
        do {
                int j;
                for (j=0; j<256; j++) {
                        buf_send((char)j, st);
                }
                printf("Incrementing count %d\n", count);
                count++;
                // __asm__ __volatile__("rdtsc":"=A"(tsc));
                tsc = get_stime(); 
        } while (tsc < tsc2);
        printf("return count %d\n", count);
        shm_release("test17_shm");
        return count;
}

static int buf_receive(struct test17_buf_st *st)
{
    int x;
    assert(wait(st->rsem) == 0);
    assert(wait(st->mutex) == 0);
    x = 0xff & (int)(st->buf[(st->rpos++) % sizeof(st->buf)]);
    assert(signal(st->mutex) == 0);
    assert(signal(st->wsem) == 0);
    return x;
}

int proc17_2(void *arg)
{
        struct test17_buf_st *st = NULL;

        (void)arg;

        st = (struct test17_buf_st*) shm_acquire("test17_shm");
        assert(st != NULL);

        while(1) {
                int x = buf_receive(st);
                atomic_incr(&st->received[x]);
        }
        shm_release("test17_shm");
        return 0;
}




int test17_sem(void *arg)
{
        int pid[6];
        int i;
        struct test17_buf_st *st = NULL;
        int count = 0;

        (void)arg;
        st = (struct test17_buf_st*) shm_create("test17_shm");
        assert(st != NULL);

        assert(getprio(getpid()) == 128);
        st->mutex = screate(1);
        assert(st->mutex >= 0);
        st->wsem = screate(100);
        assert(st->wsem >= 0);
        st->wpos = 0;
        st->rsem = screate(0);
        assert(st->rsem >= 0);
        st->rpos = 0;
        for (i=0; i<256; i++) {
                st->received[i] = 0;
        }
        for (i=0; i<3; i++) {
                pid[i] = start(proc17_1, 4000, 129, "proc17_1", &st);
                assert(pid[i] > 0);
        }
        for (i=3; i<6; i++) {
                pid[i] = start(proc17_2, 4000, 129, "proc17_2", &st);
                assert(pid[i] > 0);
        }
        for (i=0; i<3; i++) {
                int ret;
                assert(waitpid(pid[i], &ret) == pid[i]);
                count += ret;
        }
        assert(scount(st->rsem) == 0xfffd);
        for (i=3; i<6; i++) {
                int ret;
                int res = kill(pid[i]);
                printf("res %d \n",res);
                assert( res == 0);
                assert(waitpid(pid[i], &ret) == pid[i]);
        }
        assert(scount(st->mutex) == 1);
        assert(scount(st->wsem) == 100);
        printf("scount(st->rsem) = %d \n", (int16_t) (scount(st->rsem)));
        assert(scount(st->rsem) == 0);
        assert(sdelete(st->mutex) == 0);
        assert(sdelete(st->wsem) == 0);
        assert(sdelete(st->rsem) == 0);
        puts("Reached thus far \n");
        for (i=0; i<256; i++) {
                int n = st->received[i];
                if (n != count) {
                        printf("st->received[%d] == %d, count == %d\n", i, n, count);
                        assert(n == count);
                }
        }
        printf("ok (%d chars sent).\n", count * 256);
        return 0;
}
