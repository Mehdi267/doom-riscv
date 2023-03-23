#include "tests.h"
#ifndef _TEST17_SEM_H_
#define _TEST17_SEM_H_


struct test17_buf_st {
    int mutex;
    int wsem;
    unsigned wpos;
    int rsem;
    unsigned rpos;
    char buf[100];
    int received[256];
};

// Increment a variable in a single atomic operation
static void atomic_incr(int *atomic)
{
    register unsigned reg1 = 1;
	__asm__ __volatile__ ("addi %0,x0,1 " : :"rK" (reg1) );
    __asm__ __volatile__("amoadd.w t0,%1,%0" : "+m" (*atomic) : "rK"(reg1) : "cc");
}

#endif /* _TEST17_SEM_H_ */
