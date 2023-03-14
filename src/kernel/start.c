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
#include "memory/virtual_memory.h"


int kernel_start()
{
    if (set_up_virtual_memory()<0){
		puts("error while setting up virtual memory");
		exit(-1);
    }

    splash_screen();
    splash_vga_screen();

    if (initialize_process_workflow()<0){
        puts("error while setting up process");
        exit(-1);
    }
	/**
	 * These lines are used for debugging purposes, they are not relevant
	 * please don't remove them
	*/
   	// char str[80];
	// sprintf(str,"%li",csr_read(mstatus));
	// puts(str);


	activate_and_launch_scheduler();

	

   	// if (activate_and_launch_custom_process(get_process_struct_of_pid(1))<0){
    //     return -1;
    // }
	while (1) wfi(); //endort le processeur en attente d'une interruption
	
	// exit(kernel_tests(NULL));
}
