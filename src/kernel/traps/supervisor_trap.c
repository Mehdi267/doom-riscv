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
#include "../process/timer_api.h"
#include "syscall_num.h"


int syscall_handler(struct trap_frame *tf) {
  switch (tf->a7) {
    case SYSC_start:
      // TODO:
      /* start(&(tf->a)), tf->a1, tf->a2, (const char *)tf->a3, (void *)tf->a4); */
      break;
    case SYSC_getpid:
      getpid();
      break;
    case SYSC_clock_settings:
      clock_settings((unsigned long *)tf->a0, (unsigned long *) tf->a1);
      break;
    default:
      return -1; // now known a7?
  }
  return 0;
}



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
				csr_clear(sip, MIP_STIP);
				break;
      case intr_u_software:
        if (syscall_handler(tf) < 0)
          blue_screen(tf);

			default:
				die(
						"machine mode: unhandlable interrupt trap %d : %s @ %p",
						interrupt_number, interruption_names[interrupt_number], sepc
				);
				break;
		}
	} else {
		// TODO ADD SYSTEM CALLS TREATEMENT
        printf("Supervisor Exception scause id = %ld\n", scause);
		switch (scause) {
			// case CAUSE_FETCH_PAGE_FAULT:
            //     break;
			// case CAUSE_STORE_PAGE_FAULT:
			// 	printf("found execption \n");
			// 	break;
			// // case intr_s_timer: 
			default:
				blue_screen(tf);
				// no return
		}
	}
}
