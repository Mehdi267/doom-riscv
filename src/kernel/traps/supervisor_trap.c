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
#include "../msgqueue.h"

extern void inc_sepc(void); // defined in supervisor_trap_entry.S
unsigned long syscall_handler(struct trap_frame *tf) {
  // we need to return a ulong because some functions returns int (32 bit)
  // and other ulong (64 bits)
  switch (tf->a7) {
    case SYSC_start:
      // TODO: call the other start func
      /* start(&(tf->a0)), tf->a1, tf->a2, (const char *)tf->a3, (void *)tf->a4); */
      return -1;
    case SYSC_getpid:
      return getpid();
    case SYSC_getprio:
      return getprio(tf->a0);
    case SYSC_chprio:
      return  chprio(tf->a0, tf->a1);
    case SYSC_kill:
      return kill(tf->a0);
    case SYSC_waitpid:
      return waitpid(tf->a0, (int *)tf->a1);
    case SYSC_exit:
      exit_process(tf->a0);
      return 0; // we need to return something
    case SYSC_cons_write:
      // TODO:
      return -1;
    case SYSC_cons_read:
      // TODO:
      return -1;
    case SYSC_cons_echo:
      // TODO:
      return -1;
    case SYSC_pcount:
      return pcount(tf->a0, (int*) tf->a1);
    case SYSC_pcreate:
      return pcreate(tf->a0);
    case SYSC_pdelete:
      return pdelete(tf->a0);
    case SYSC_preceive:
      return preceive(tf->a0, (int *)tf->a1);
    case SYSC_preset:
      return preset(tf->a0);
    case SYSC_psend:
      return psend(tf->a0, tf->a1);
    case SYSC_clock_settings:
      clock_settings((unsigned long *)tf->a0, (unsigned long *) tf->a1);
      break;
    default:
      blue_screen(tf);
      // no return
      assert(false);
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
        int retval = syscall_handler(tf);
        uint64_t * a0_stack_addr = ((uint64_t *)(tf->sp))+10;
        *(a0_stack_addr) = retval; // WRITING a0 in the stack so when it will be restaured
        inc_sepc();
        break;

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
