// Idle process user mode this method does nothing it will just iterate 
//over a loop and wait for to be interrupted 
#include "../tests/tlib/sysapi.h"

static const char *tests[500] = {
		"test0",
		"test1",
		"test2",
		"test3",
		"test4",
    "test5",
    "test7",
    "test8",
    "test10",
    "test11",
    "test12",
    "test13",
    "test14",
    "test15",
    "test16",
    "test17",
    "test19",
    "test20",
    "test21",
    "test22",
};

int main(void *arg)
{
//   csr_clear(sstatus, 0x2);
  while(1){}
  return 0;
  register unsigned reg1 = 5;
	__asm__ __volatile__ ("li s1,3562" : :"rK" (reg1) );
  if (tests[0]){
    tests[0] = "a";
  }
}