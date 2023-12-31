/*
 * Ensimag - Projet système
 * Copyright (C) 2013 - Damien Dejean <dam.dejean@gmail.com>
 */

#include "sysapi.h"

const char *tests[] = 
{
        "test0",
        "test1",
        "test2",
        "test3",
        "test4",
        "test5",
        //"test7", //takes too long
        "test8",
        "test10",
        "test11",
        "test12",
        "test13",
        "test14",
        "test15",
        //"test16",
        // "test17",
        #ifndef TESTING
        // "test19",
        #endif
        "test20",
        "test21",
        "test22",
        "test23",
        "test24",
        "test25",
        "test26",
        "test27",
        "test28",
        "test29",
        "test30",
        "test31",
        "test32",
        "test33",
        "test34",
        NULL
};

int test_res[sizeof(tests)/sizeof(char *)];
void tests_report() {
  printf("--- TESTS REPORT ---\n");
  for (int i = 0; tests[i] != NULL; i++) {
    printf("Test %s : ", tests[i]);
    if (test_res[i] == 0)
      printf("\x1B[1m\x1B[32mPASSED \x1b[0m\n");
    else
      printf("\x1B[1\x1B[31mFAILED \x1b[0m\n");
  }
  printf("end");
}

int main(void) {
  int i;
  int pid;
  long int ret;
  int has_failed = 0;
  for (i = 0; tests[i] != NULL; i++) {
    printf("Test %s : ", tests[i]);
    pid = start(tests[i], 4000, 128, NULL);
    waitpid_old(pid, &ret);
    test_res[i] = (int)ret;
    if (ret != 0 && has_failed == 0){
      printf("Test failed on i", i );
      has_failed = 1;
    }
  }
  printf("Auto test done.\n");
  tests_report();
  if (has_failed == 1){
    power_off(has_failed);
  }
}
