#ifndef _TESTS_INTERFACE_H_
#define _TESTS_INTERFACE_H_

/**
 * @brief 
 * Run kernel test suite,
 * process related    
 * @param arg 
 * @return int 0 if success and -1 otherwise 
 */
int kernel_tests(void *arg);


/**
 * @brief Runs driver tests
 * @return int 0 if success and -1 otherwise 
 */
int kernel_drivers_tests(void *arg);


#endif /* _TESTS_H_ */
