/*
 * Projet PCSEA RISC-V
 *
 * Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 *
 * See license for license details.
 */

//#include <unistd.h>
#include "stdio.h"
#include "assert.h"
#include "stddef.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "riscv.h"
#include "process/process.h"
#include "process/helperfunc.h"
#include "drivers/splash.h"
#include "tests/tests.h"

int kernel_start()
{
	splash_screen();
	splash_vga_screen();

	//if (initialize_process_workflow()<0){
	//	puts("error while setting up process");
	//	exit(-1);
	//}

  uint8_t variable = 42;
  uint8_t *ptr1 = &variable;
  uint8_t *ptr2 = ptr1 + 0x100000000;

  printf("ptr1 @%p = %u\n", ptr1, *ptr1);
  printf("ptr2 @%p = %u\n", ptr2, *ptr2);	
  if (*ptr1 == *ptr2)
      puts("Test mémoire virtuelle OK");
  else
      puts("Test mémoire virtuelle FAIL");

	/**
	 * These lines are used for debugging purposes, they are not relevant
	 * please don't remove them
	*/
   	// char str[80];
	// sprintf(str,"%li",csr_read(mstatus));
	// puts(str);

	// printf("hello there");
	// activate_and_launch_scheduler();

	

   	// if (activate_and_launch_custom_process(get_process_struct_of_pid(1))<0){
    //     return -1;
    // }
	while (1) wfi(); //endort le processeur en attente d'une interruption
	
	// exit(kernel_tests(NULL));

}
