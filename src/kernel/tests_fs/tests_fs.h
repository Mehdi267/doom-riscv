#ifndef TEST_FILE_SYSTEM_H 
#define TEST_FILE_SYSTEM_H

#include "stdbool.h"
#include "stdint.h"
#include "assert.h"
#include "../fs/fs.h"
#include "../fs/mbr.h"

/**
 * @brief Runs the test case for the `find_free_space` function.
 */
void test_find_free_space();

/**
 * @brief Runs the test case for the `is_segment_in_free_space` function.
 */
void test_is_segment_in_free_space();

#endif




