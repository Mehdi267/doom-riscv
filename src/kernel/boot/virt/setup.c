/*
 * Projet PCSEA RISC-V
 *
 * Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 *
 * See license for license details.
 */


#include "stdint.h"
#include "string.h"
#include "riscv.h"

#include "drivers/auxval.h"
#include "drivers/plic.h"
#include "drivers/clint.h"
#include "drivers/poweroff.h"
#include "drivers/console.h"

char * default_program = "console";

//Check hw/riscv/virt.c in the qemu repo for more details 
auxval_t __auxv[] = {
		{UART0_CLOCK_FREQ,       1843200},
		{UART0_BAUD_RATE,        115200},
		{VIRT_UART0_CTRL_ADDR, 0x10000000},
		{VIRT_TEST_CTRL_ADDR,  0x100000}, 
		{0,                      0}
};

void arch_setup()
{
	register_console(&console_ns16550a);
	register_poweroff(&poweroff_virt_test);
	register_clint(&virt_clint);
	register_plic(&virt_plic);

	// activation des compteurs de performances
	csr_write(mcounteren, -1);
	csr_write(scounteren, -1);
}
