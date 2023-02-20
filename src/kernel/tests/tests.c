#include <stddef.h>
#include <stdio.h>

#include "tests.h"


int64_t kernel_tests(void *arg) {
	int64_t rc = 0;

	puts("Run kernel test suite:");
	if (test0(NULL))
		rc = 1;

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

	return rc;
}
