#ifndef _TESTS_H_
#define _TESTS_H_

#include "stdint.h"

#define DUMMY_VAL 0xDEADCAFE

/*
 * Run kernel test suite
 */
int64_t kernel_tests(void *arg);

/*
 * Start a simple process
 */
int64_t test0(void* arg);

/*
 * Start a process with a parameter
 * End normally
 * Father wait for his child
 */
int64_t test1(void* arg);

/*
 * Test kill and exit
 */
int64_t test2(void* arg);

/*
 * Prioities and chprio
 */
int64_t test3(void* arg);

/*
 * shared time and active waiting
 */
int64_t test4(void* arg);

/*
 * Robustness of the scheduler
 */
int64_t test5(void* arg);

/*
 * Semaphores or message queues test
 */
int64_t test10(void* arg);

#endif /* _TESTS_H_ */
