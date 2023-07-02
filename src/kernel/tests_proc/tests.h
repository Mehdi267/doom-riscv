#ifndef _TESTS_H_
#define _TESTS_H_

#include "stdint.h"
#include "../process/process.h"
#include "../process/helperfunc.h"
#include "../sync/timer_api.h"
#include "../process/memory_api.h"
#include "../sync/semaphore_api.h"
#include "../sync/msgqueue.h" 
#include "assert.h"
#include "drivers/clint.h"
#include "stdio.h"
#include "sysapi_kernel.h"


/*
 * Start a simple process
 */
int test0(void* arg);

/**
 * IT test
 */
void test_it();


/*
 * Start a process with a parameter
 * End normally
 * Father wait for his child
 */
int test1(void* arg);

/*
 * Test kill and exit
 */
int test2(void* arg);

/*
 * Prioities and chprio
 */
int test3(void* arg);

/*
 * shared time and active waiting
 */
int test4(void* arg);

/*
 * Robustness of the scheduler
 */
int test5(void* arg);

/*
 * timer tests
 */
int test6(void* arg);

/*
 * Timer and shared memory
 */
int test7(void* arg);

/*
 * Test the process memory api
 */
int test_memory(void* arg);

/**
 * Test les semaphores 
 */
int test_sem(void *arg);

/*
 *  Message queues test
 */
int test10(void* arg);
int test13_msg(void* arg);
int test12(void *arg);
int test13(void *arg);
int test14(void *arg);
int test15(void *arg);
int test17(void *arg);

/**
 * Semaphores test 
 */
int test12_sem(void *arg);
int test13_sem(void *arg);
int test15_sem(void *arg);
int test16_sem(void *arg);
int test17_sem(void *arg);

/*
 *  Test message queue/semaphores
 */
int test11(void *arg);
int test20(void *arg);

/*
 *  Tests shared memory
 */
int test21(void *arg);

/*
 * Test shared memory clean and 
 */
int test22(void *arg);



typedef struct test_apps {
    process_function_t test_func;
    const char *test_name;
    int test_return_value;
} test_apps_t ;


#endif /* _TESTS_H_ */
