/*
 * Ensimag - Projet système
 * Copyright (C) 2013 - Damien Dejean <dam.dejean@gmail.com>
 */

#include "sysapi.h"

#define TESTS_NUMBER 20

const char *tests[TESTS_NUMBER] = {
        "test0",
        "test1",
        "test2",
        "test3",
        "test4",
        "test5",
        "test7", //takes too long
        "test8",
        "test10",
        "test11", 
        "test12",
        "test13",  
        "test14",
        "test15",
        "test16",
        "test17",
        //"test19", // syscalls not working yet
        "test20",
        "test21",
        "test22",
        NULL
};//*/

/*
        "test6",
        "test9",
        "test14",
        "test15",
        "test16",
        "test17",
        "test18",
        "test19",
        "test20",
        "test21",
        "test22",
};
*/

int main(void)
{
	int i;
	int pid;
	long int ret;
        // printf("%d", cons_write("help", 4));
        // csr_clear(sstatus, 0x2);
        // while(x){x = 4;}
	for (i = 0; i < TESTS_NUMBER; i++) {
		if (tests[i] == NULL){
                        break;
                }
                printf("Test %s : ", tests[i]);
		pid = start(tests[i], 4000, 128, NULL);
		waitpid(pid, &ret);
		assert(ret == 0);
	}
	printf("Auto test done.\n");
        power_off(0);
}

