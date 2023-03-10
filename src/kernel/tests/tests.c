#include <stddef.h>
#include <stdio.h>
#include "tests.h"

#define NUMBEROFTESTS 7

void generate_test_report(test_apps_t* test_table){
	print_test_no_arg("--------------TESTS REPORT START--------------\n");	
	for (int test_iter = 0 ; test_iter<NUMBEROFTESTS ; test_iter ++ ){
		debug_print_tests("Ran test id = %d // Test name = %s // Test status =",
					 test_table[test_iter].test_id,
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
	int rc = 0;

	test_apps_t test_table[NUMBEROFTESTS] = {
		{test0, "test0", 0, 0},
		{test1, "test1", 1, 0},
		{test2, "test2", 2, 0},
		{test3, "test3", 3, 0},
		{test4, "test4", 4, 0},
		{test5, "test5", 5, 0},
		{test6, "test6", 6, 0},
	};

	print_test_no_arg("\n---------------------kernel_tests executing---------------------\n");
	/*
	 * Dans un second temps (quand vous aurez la création de task/processus), les tests devront
	 * être exécutés dans un processus dédié.
	 * Comme par exemple:
	 * int test_rc;
	 * int pid = sched_kstart(test0, 10, "Test 0", 0);
	 * sched_waitpid(pid, &test_rc);
	 * if (test_rc)
	 * 	rc = 1;
	 */
	
	int test_rc;
	int pid;
	for (int test_iter = 0 ; test_iter<NUMBEROFTESTS ; test_iter ++ ){
		debug_print_tests("\n-------------------%s START-------------------\n", test_table[test_iter].test_name);
		pid = start(test_table[test_iter].test_func, 4000, 128,test_table[test_iter].test_name, 0);
		waitpid(pid, &test_rc);
		test_table[test_iter].test_return_value = test_rc;		
		debug_print_tests("\n-------------------%s END-------------------\n", test_table[test_iter].test_name);
	}

	generate_test_report(test_table);
	return rc;
}
