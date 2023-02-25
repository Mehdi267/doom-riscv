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

#include "drivers/splash.h"
#include "tests/tests.h"

int kernel_start()
{
	splash_screen();
	splash_vga_screen();

	/**
	 * These lines are used for debugging purposes, they are not relevant
	 * please don't remove them
	*/
   	// char str[80];
	// sprintf(str,"%li",csr_read(mstatus));
	// puts(str);


	puts("hello there");
	while (1) wfi(); //endort le processeur en attente d'une interruption
	
	exit(kernel_tests(NULL));

}
