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
#include "../sync/timer_api.h"
#include "../sync/semaphore_api.h"
#include "../process/memory_api.h"
#include "../process/scheduler.h"
#include "syscall_num.h"
#include "../sync/msgqueue.h"
#include "../input-output/cons_write.h"
#include "../input-output/keyboard.h"
#include "../fs/fs.h"
#include "../fs/fs_api.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern void inc_sepc(void); // defined in supervisor_trap_entry.S
unsigned long static syscall_handler(struct trap_frame *tf) {
  // we need to return a ulong because some functions returns int (32 bit)
  // and other ulong (64 bits)
  unsigned long x ;
  switch (tf->a7) {
    case SYSC_start:
      return start_virtual((const char*) tf->a0, (unsigned long) tf->a1,(int) tf->a2, (void *)tf->a3);
    case SYSC_getpid:
      return getpid();
    case SYSC_getprio:
      return getprio(tf->a0);
    case SYSC_chprio:
      return  chprio(tf->a0, tf->a1);
    case SYSC_kill:
      return kill(tf->a0);
    case SYSC_waitpid:
      x = waitpid(tf->a0,(int*) tf->a1);
      // printf("{trap handeler} Value written in *retvalp %ld \n", *((unsigned long *) tf->a1));
      return x;
    case SYSC_exit:
      exit_process(tf->a0);
      return 0; // we need to return something
    case SYSC_cons_write:
      return cons_write((char*) tf->a0, (unsigned long) tf->a1);
    case SYSC_cons_read:
      return cons_read((char*) tf->a0, (unsigned long) tf->a1);
    case SYSC_cons_echo:
      cons_echo((int)tf->a0);
      return 0;
    case SYSC_pcount:
      return pcount(tf->a0, (int*) tf->a1);
    case SYSC_pcreate:
      // printf("--------pcreate called =  -----------\n");
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
    case SYSC_sleep:
      sleep(tf->a0);
      break;
    case SYSC_wait_clock:
      wait_clock(tf->a0);
      break;
    case SYSC_current_clock:
      return current_clock();
    case SYSC_scount:
      return scount(tf->a0);             
    case SYSC_screate:
      return screate(tf->a0);            
    case SYSC_sdelete:
      return sdelete(tf->a0);            
    case SYSC_signal:
      return signal(tf->a0);             
    case SYSC_signaln:
      return signaln(tf->a0, tf->a1);           
    case SYSC_sreset:
      return sreset(tf->a0, tf->a1);             
    case SYSC_try_wait:
      return try_wait(tf->a0);           
    case SYSC_wait:
      return wait(tf->a0);      
    case SYSC_shm_create:
      debug_print("--------shm method create string = = %s -----------\n", (const char*) tf->a0);
      return (unsigned long) shm_create((const char*) tf->a0);
    case SYSC_shm_acquire:
      debug_print("--------shm method acquire string = = %s ------\n", (const char*) tf->a0);
      return (unsigned long) shm_acquire((const char*) tf->a0);
    case SYSC_shm_release:
      shm_release((const char*) tf->a0);
      break; 
    case SYSC_power_off:
      exit(tf->a0);
      break;
    case SYSC_show_ps_info:
      show_ps_info();
      break;
    case SYSC_show_programs:
      show_programs();
      break;
    case SYSC_info_queue:
      info_msgqueues();
      break;
    case SYSC_display_partions:
      print_partition_status();
      break;  
    case SYSC_create_partition:
      create_partition(tf->a0, tf->a1, tf->a2);
      break;  
    case SYSC_delete_partition:
      delete_partition(tf->a0);
      break;    
    case SYSC_reset_disk:
      return set_up_mbr();
      break; 
    case SYSC_sync:
      return sync();
      break;    
    case SYSC_clear_disk_cache:
      return free_cache_list();
      break;    
    case SYSC_print_fs_details:
      print_fs_details();
      break;
    case SYSC_open:
      return open((const char *)tf->a0, tf->a1);
    case SYSC_close:
      return close(tf->a0);
    case SYSC_read:
      return read(tf->a0, (void*)tf->a1, (uint64_t) tf->a2);
    case SYSC_write:
      return write(tf->a0, (void*)tf->a1, (uint64_t) tf->a2);
    case SYSC_lseek:
      return lseek(tf->a0, tf->a1, tf->a2);
    case SYSC_print_dir_elements:
      print_dir_elements((const char*)tf->a0);    
      break;
    default:
      printf("Syscall code does not match any of the defined syscalls");
      // blue_screen(tf);
      break;
  }
  return 0;
}


void strap_handler(uintptr_t scause, void *sepc, struct trap_frame *tf){
  // printf("super int %ld\n",scause&0xff);
    
  bool user = false;
  if (((csr_read(sstatus) & MSTATUS_SPP) == 0)){
			debug_print_no_arg("Int comming from user mode");
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
        if (user){
          PRINT_RED("Killing process = (CAUSE_FETCH_PAGE_FAULT)\n");
          kill(getpid());
          scheduler();
          break;
        }
      case CAUSE_LOAD_PAGE_FAULT:
        if (user){
          PRINT_RED("Killing proces = (CAUSE_LOAD_PAGE_FAULTs)\n");
          kill(getpid());
          scheduler();
          break;
        }
      case CAUSE_STORE_PAGE_FAULT:
        if (user){
          PRINT_RED("Killing process = (CAUSE_STORE_PAGE_FAULT)\n");
          kill(getpid());
          scheduler();
          break;
        }
			default:
        //The cause is treated we exit immediately
				blue_screen(tf);
    }
	}
}
