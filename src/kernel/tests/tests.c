#include <stddef.h>
#include <stdio.h>
#include "tests.h"

test_apps_t test_table[] = {
  {test0, "test0", 0},
  {test1, "test1", 0},
  {test2, "test2", 0},
  {test3, "test3", 0},
  {test4, "test4", 0}, // Takes a lot of time
  {test5, "test5", 0},
  //{test6, "test6", 0},//Test is so slow
  {test7, "test7", 0}, // TODO: CHECK this test working when ran solo but not working when run with other
  //{test11, "test11", 0}, // TODO: CHECK THIS TEST not working Does not run with a semaphore 
  {test12_sem, "test12_sem", 0},
  {test13_sem, "test13_sem", 0},
  {test15_sem, "test15_sem", 0},
  {test10, "test10", 0},
  {test12, "test12", 0},
  {test14, "test14", 0},
  {test15, "test15", 0},
  {test17, "test17", 0},
  /*{test16_sem, "test16_sem",0},//TO much memory usage 
  //(can only executed with a heap of size 5M) */ 
  {test17_sem, "test17_sem", 0},
  {test20, "test20", 0},
  {test21, "test21", 0},
  //{test22, "test22", 0}*/
};

const int NUMBEROFTESTS = sizeof(test_table)/sizeof(test_apps_t);

void generate_test_report(test_apps_t* test_table){
	print_test_no_arg("--------------TESTS REPORT START--------------\n");	
	for (int test_iter = 0 ; test_iter<NUMBEROFTESTS ; test_iter ++ ){
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

int kernel_tests(void *arg) {
  /**
   * Rc is used to indicate if the tests failed
   */
  print_test_no_arg(
      "\n---------------------Inside kernel tests---------------------\n");
  int rc = 0;

  // unsigned long int *addess_user_mode = (unsigned long int *) 0x40000000;
  // unsigned long int x = *addess_user_mode;
  // debug_print_tests("hello world %ld", x);

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
  for (int test_iter = 0; test_iter < NUMBEROFTESTS; test_iter++) {
    debug_print_tests("\n-------------------%s START-------------------\n",
                      test_table[test_iter].test_name);
    pid = start(test_table[test_iter].test_func, 4000, 128,
                test_table[test_iter].test_name, 0);
    if (waitpid(pid, &test_rc) < 0){
        exit(-1);
    }
    test_table[test_iter].test_return_value = test_rc;
    debug_print_tests("\n-------------------%s END-------------------\n",
                      test_table[test_iter].test_name);
  }
  print_test_no_arg("\n---------------------kernel_tests Have been "
                    "executed---------------------\n");

  generate_test_report(test_table);
  exit(rc);
  return rc;
}
