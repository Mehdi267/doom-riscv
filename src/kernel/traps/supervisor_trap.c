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

#include "traps/trap.h"
#include "timer.h"



void strap_handler(uintptr_t scause, void *sepc, struct trap_frame *tf)
{
	if (scause & INTERRUPT_CAUSE_FLAG) {
		// Interruption cause
		uint8_t interrupt_number = scause & ~INTERRUPT_CAUSE_FLAG;
		switch (scause & ~INTERRUPT_CAUSE_FLAG) {
			case intr_s_timer: // in case the s timer interrupt has not been delegated to supervisor mode
				handle_stimer_interrupt();
				/**
				 * We clear the bit in the sip register that was responsible for this interrupt 
				 * so that we don't jump into the same interrupt again
				*/
				csr_clear(sip, intr_s_timer);
				csr_set(sip, intr_s_timer);
				break;
			default:
				die(
						"machine mode: unhandlable interrupt trap %d : %s @ %p",
						interrupt_number, interruption_names[interrupt_number], sepc
				);
				break;
		}
	} else {
		// Exception cause
		switch (scause) {
			default:
				blue_screen(tf);
				// no return
		}
	}
}
