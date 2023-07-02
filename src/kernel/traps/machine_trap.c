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
#include "../input-output/keyboard.h"

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

#define PLIC_MCLAIM(hart) (0x0c000000L + 0x200004 + (hart)*0x2000)
#define PLIC_SCLAIM(hart) (0x0c000000L + 0x201004 + (hart)*0x2000)


void mtrap_handler(uintptr_t mcause, void *mepc, struct trap_frame *tf)
{
  if ((mcause & 0xff)  != 7){
   printf("intt machine, mcause = %ld\n", mcause&0xff);
  }
	
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
				handle_stimer_interrupt();
				csr_clear(mip, intr_s_timer);
				break;
			case intr_s_external:
        //This is what gets called when we have a keyboard interrupt
				//interruption clavier
				handle_keyboard_interrupt();
				// csr_clear(mip, SIE_SEI); //clear interrupt
				break;
      case intr_m_external:
        printf("intr_m_external not treated");
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
