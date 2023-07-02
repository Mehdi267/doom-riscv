#ifndef _TESTS_DRIVERS_H_
#define _TESTS_DRIVERS_H_

#include "drivers/disk_device.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "drivers/clint.h"
#include "timer.h"
#include "logger.h"

/**
 * @brief Test basic disk operation
 * @param arg 
 * @return int 
 */
extern int disk_driver_test(void *arg);

/**
 * @brief Test multiple disk operations 
 * it would be better to test this while using multiple 
 * processes that get interuppted while the disk 
 * is fetching data 
 * @param arg 
 * @return int 
 */
extern int disk_driver_stress_test(void *arg);


#endif /* _TESTS_H_ */
