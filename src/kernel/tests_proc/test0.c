#include "tests.h"

#include "stdio.h"
#include "assert.h"


int test0(void *arg)
{
	(void)arg;

	register unsigned reg1 = 1u;
	register unsigned reg2 = 0xFFFFFFFFu;
	register unsigned reg3 = 0xBADB00B5u;
	register unsigned reg4 = 0xDEADBEEFu;

	printf("test0...\n");

	printf("I'm a simple process running ...");

	unsigned i;
	for (i = 0; i < 10000000; i++) {
		if (
				reg1 != 1u || 
				reg2 != 0xFFFFFFFFu || 
				reg3 != 0xBADB00B5u || 
				reg4 != 0xDEADBEEFu) {
			printf(" and I feel bad. Bybye ...\n");
			assert(0);
		}
	}

	printf(" and I'm healthy. Leaving.\n");

	printf("\r\t\t\t\t\t\t\t[OK]\n");

	return 0;
}
