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
#include "syscall.h"
#include "../process/process.h"
#include "../process/process_memory.h"
#include "../sync/timer_api.h"
#include "../process/memory_api.h"
#include "../process/scheduler.h"
#include "../input-output/cons_write.h"
#include "../input-output/keyboard.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>



extern void inc_sepc(void); // defined in supervisor_trap_entry.S

int debug = 0;

void noting(){}

void strap_handler(uintptr_t scause, void *sepc, struct trap_frame *tf){
  bool user = false;
  if (((csr_read(sstatus) & MSTATUS_SPP) == 0)){
			// debug_print_no_arg("Int comming from user mode\n");
    user = true;
  }
  
  if (scause & INTERRUPT_CAUSE_FLAG) {
		// Interruption cause
		uint8_t interrupt_number = scause & ~INTERRUPT_CAUSE_FLAG;
		switch (scause & ~INTERRUPT_CAUSE_FLAG) {
			#ifdef VIRTMACHINE
        case intr_s_software:
        	set_supervisor_interrupts(false);
          csr_clear(sip, 0x2);
          handle_stimer_interrupt();
          break;
      #endif
      case intr_s_timer: 
				handle_stimer_interrupt();
				/**
				 * We clear the bit in the sip register that was responsible for this interrupt 
				 * so that we don't jump into the same interrupt again
				*/
				csr_clear(sip, SIP_STIP);
				break;
			case intr_s_external:
				//interruption clavier
        handle_keyboard_interrupt();
				csr_clear(sip, SIE_SEI); //clear interrupt
				break;
			default:
				die(
						"machine mode: unhandlable interrupt trap %d : %s @ %p",
						interrupt_number, interruption_names[interrupt_number], sepc
				);
				break;
		}
	} else {
    if (scause != 8){
      debug_print("Supervisor Exception scause id = %ld\n", scause);
    }
    unsigned long retval;
    switch (scause) {
      case CAUSE_SUPERVISOR_ECALL:
        //-----------Used for testing....
        //We call the method that its code was placed in a7 regitster
        retval = syscall_handler(tf);
        // Writing in the saved trap frame directly instead of writing in the stack
        tf->a0 = (uint64_t) retval; 
        //Makes sure that we do not jump in an ecall again 
        csr_write(sepc, csr_read(sepc) + 4);
        csr_clear(sstatus, MSTATUS_SPP);
        break;
      case CAUSE_USER_ECALL:
        //We call the method that its code was placed in a7 regitster
        retval = syscall_handler(tf);
        // Writing in the saved trap frame directly instead of writing in the stack
        tf->a0 = (uint64_t) retval; 
        //Makes sure that we do not jump in an ecall again 
        csr_write(sepc, csr_read(sepc) + 4);
        csr_clear(sstatus, MSTATUS_SPP);
        break;
      case CAUSE_FETCH_PAGE_FAULT:
        printf("Trying to add to mem to,  pid = %d, name = %s\n", getpid(), getname());
        blue_screen(tf);
        if (user){
          if (check_expansion_mem(get_process_struct_of_pid(getpid()), tf)>=0){
            break;
          } else{
            PRINT_RED("Killing process = (CAUSE_FETCH_PAGE_FAULT)\n");
            kill(getpid());
            scheduler();
            break;
          }
        }
      case CAUSE_LOAD_PAGE_FAULT:
        // // while(1){}
        printf("Trying to add to mem to,  pid = %d, name = %s\n", getpid(), getname());
        blue_screen(tf);
        if (user){
          if (check_expansion_mem(get_process_struct_of_pid(getpid()), tf)>=0){
            break;
          } else{
            PRINT_RED("Killing proces = (CAUSE_LOAD_PAGE_FAULTs)\n");
            kill(getpid());
            scheduler();
            break;
          }
        }
      case CAUSE_STORE_PAGE_FAULT:
        // // while(1){}
        printf("Trying to add to mem to,  pid = %d, name = %s\n", getpid(), getname());
        blue_screen(tf);
        if (user){
          if (check_expansion_mem(get_process_struct_of_pid(getpid()), tf)>=0){
            break;
          } else{
            PRINT_RED("Killing process = (CAUSE_STORE_PAGE_FAULT)\n");
            kill(getpid());
            scheduler();
            break;
          }
        }
			default:
        // while(1){};
        //The cause is treated we exit immediately
				blue_screen(tf);
    }
	}
}