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

int kernel_start()
{
	splash_screen();
	splash_vga_screen();

	puts("hello there");
	while (1) wfi(); //endort le processeur en attente d'une interruption
	
	exit(0);
}
