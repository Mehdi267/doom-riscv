/*
 * Projet PCSEA RISC-V
 *
 * Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 *
 * See license for license details.
 */

#include "assert.h"
#include "riscv.h"

#include "trap.h"
#include "traps/trap.h"
#include "timer.h"
#include "../process/process.h"

const char *interruption_names[16] = {
		"u_software",
		"s_software",
		"h_software",
		"m_software",
		"u_timer",
		"s_timer",
		"h_timer",
		"m_timer",
		"u_external",
		"s_external",
		"h_external",
		"m_external",
		"reserved",
		"reserved",
		"reserved",
		"reserved"
};


void mtrap_handler(uintptr_t mcause, void *mepc, struct trap_frame *tf)
{
	if (mcause & INTERRUPT_CAUSE_FLAG) {
		// Interruption cause
		uint8_t interrupt_number = mcause & ~INTERRUPT_CAUSE_FLAG;
		switch (mcause & ~INTERRUPT_CAUSE_FLAG) {
			case intr_m_timer:
				handle_mtimer_interrupt();
				#ifndef VIRTMACHINE
				csr_clear(mip, intr_m_timer);
				#endif
				break;
			case intr_s_timer: // in case the s timer interrupt has not been delegated to supervisor mode
				// printf("machine int = 1\n");
				handle_stimer_interrupt();
				csr_clear(mip, intr_s_timer);
				break;
			default:
				die(
						"machine mode: unhandlable interrupt trap %d : %s @ %p",
						interrupt_number, interruption_names[interrupt_number], mepc
				);
				break;
		}
	} else {
		// Exception cause
		debug_print("Machine Exception scause id = %ld\n", mcause);
		switch (mcause) {
			default:
				blue_screen(tf);
				// no return
		}
	}
}
