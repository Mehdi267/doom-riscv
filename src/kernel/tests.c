#include <stddef.h>
#include <stdio.h>
#include "tests_proc/tests.h"
#include "tests_driver/test_drivers.h"
#include "test_interface.h"

test_apps_t test_table_proc[] = {
  {test0, "test0", 0},
  {test1, "test1", 0},
  {test2, "test2", 0},
  {test3, "test3", 0},
  {test4, "test4", 0}, // Takes a lot of time
  {test5, "test5", 0},
  //{test6, "test6", 0},
  //{test7, "test7", 0},  
  //{test11, "test11", 0},  
  {test12_sem, "test12_sem", 0},
  {test13_msg, "test13_msg", 0},
  {test13_sem, "test13_sem", 0},
  {test15_sem, "test15_sem", 0},
  {test10, "test10", 0},
  {test12, "test12", 0},
  {test13_msg, "test13_msg", 0},
  {test14, "test14", 0},
  {test15, "test15", 0},
  {test17, "test17", 0},
  /*{test16_sem, "test16_sem",0},//TO much memory usage 
  //(can only executed with a heap of size 5M) */ 
  {test17_sem, "test17_sem", 0},
  {test20, "test20", 0},
  {test21, "test21", 0},
  //{test22, "test22", 0}*/ //Can only be tested in user mode
};

test_apps_t test_table_drivers[] = {
  {disk_driver_test, "disk driver", 0},
  {disk_driver_stress_test, "disk stress test", 0},
};

const unsigned int NUMBEROFTESTSDRIVER = sizeof(test_table_drivers)/sizeof(test_apps_t);
const unsigned int NUMBEROFTESTSPROC = sizeof(test_table_proc)/sizeof(test_apps_t);

/**
 * @brief Generates a test report showing all of the executed tests and their status
 * @param test_table a table taht contains test_apps indicating every test ran and 
 * the retutn value of every test from whichwe ill get a return value
*/
void generate_test_report(test_apps_t* test_table, unsigned int nb_tests){
	print_test_no_arg("--------------TESTS REPORT START--------------\n");	
	for (int test_iter = 0 ; test_iter < nb_tests ; test_iter ++ ){
		debug_print_tests("Ran test id = %d // Test name = %s // Test status =",
					 test_iter,
					 test_table[test_iter].test_name);
		if (test_table[test_iter].test_return_value)
		{
			//Test failed
			debug_print_tests("\x1B[1\x1B[31m FAILED \x1b[0m// Return value = %d\n",
								test_table[test_iter].test_return_value );
		}
		else{
			debug_print_tests("\x1B[1m\x1B[32m PASSED \x1b[0m// Return value = %d\n",
								test_table[test_iter].test_return_value );	
		}
	}
	print_test_no_arg("--------------TESTS REPORT END--------------\n");	
}

int kernel_drivers_tests(void *arg){
  int rv = 0;
  for (int test_iter = 0; test_iter < NUMBEROFTESTSDRIVER; test_iter++) {
    debug_print_tests("\n-------------------%s START-------------------\n",
                  test_table_drivers[test_iter].test_name);
    test_table_drivers[test_iter].test_return_value = test_table_drivers[test_iter].test_func(0);
    if (test_table_drivers[test_iter].test_return_value !=0){
      rv = -1;
    }
  }
  generate_test_report(test_table_drivers, NUMBEROFTESTSDRIVER);
  return rv;
}


/**
 * @brief This fuctions runs the kernel process tests, 
 * this method can be run as a kernel process 
 */
int kernel_tests(void *arg) {
  /**
   * Rc is used to indicate if the tests failed
   */
  print_test_no_arg(
      "\n---------------------Inside kernel tests---------------------\n");
  int rc = 0;

  print_test_no_arg(
      "\n---------------------kernel_tests executing---------------------\n");

  /*
   * Dans un second temps (quand vous aurez la création de task/processus), les
   * tests devront être exécutés dans un processus dédié. Comme par exemple: int
   * test_rc; int pid = sched_kstart(test0, 10, "Test 0", 0); sched_waitpid(pid,
   * &test_rc); if (test_rc) rc = 1;
   */

  int test_rc;
  int pid;
  for (int test_iter = 0; test_iter < NUMBEROFTESTSPROC; test_iter++) {
    debug_print_tests("\n-------------------%s START-------------------\n",
                      test_table_proc[test_iter].test_name);
    pid = start(test_table_proc[test_iter].test_func, 4000, 128,
                test_table_proc[test_iter].test_name, 0);
    if (waitpid(pid, &test_rc) < 0){
        exit(-1);
    }
    test_table_proc[test_iter].test_return_value = test_rc;
    debug_print_tests("\n-------------------%s END-------------------\n",
                      test_table_proc[test_iter].test_name);
  }
  print_test_no_arg("\n---------------------kernel_tests Have been "
                    "executed---------------------\n");

  generate_test_report(test_table_proc, NUMBEROFTESTSPROC);
  exit(rc);
  return rc;
}

