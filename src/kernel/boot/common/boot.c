/*
 * Projet PCSEA RISC-V
 *
 * Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 *
 * See license for license details.
 */

#include "stdlib.h"
#include "assert.h"
#include "riscv.h"

#include "bios/info.h"
#include "traps/trap.h"
#include "timer.h"
#include "drivers/splash.h"
#include "memory/frame_dist.h" //used to call init_frames
#include "process/process.h" // used for the debug method


// Indicates if sum will be activated or not (used to debug user mode)
#define USER_PROCESS_DEBUG

/*
* External prototypes
*/
// kernel/start.c
extern int kernel_start();

// Specific machine setup: kernel/boot/$(MACHINE)/setup.c
extern void arch_setup();

/*
* Delegation of certain interrupts and exceptions to Supervisor mode.
*/
static void delegate_traps(){
  /**
  * @brief   This function delegates timer interrupt to the Supervisor mode 
  * instead of machine mode 
  * --Long version:
  * ---1/---
  * "Traps never transition from a more-privileged mode to a less-privileged mode." page 44 privileged isa
  * We can conclude with the line above that when we are in machine mode or supervisor mode trap, we cannot
  * transition to a lower privileged mode, which would be the supervisor mode/user mode in this case.
  * ---2/---
  * "By default, all traps at any privilege level are handled in machine mode" page 43 privileged isa
  * With this line, we can conclude that if we are in the supervisor mode or any mode and we encounter an interrupt,
  * we go directly to the machine mode and use mtvec and the machine trap handling functions like mtrap_entry, etc.
  * As it was already added in the machine trap handler, we can detect that the interrupt was called from the supervisor
  * mode and use the appropriate function. However, this is not the proper approach because when we are in the supervisor mode,
  * we would like to exploit the methods that we added for trap handling in this mode, like strap_entry, strap_handler,
  * in order to have more control over what we do. For that reason, we exploit the two registers:
  * medeleg (exceptions) and mideleg (interrupts) to delegate the appropriate traps to the appropriate mode.
  */
  // We delegate everything to supervisor mode
  csr_set(medeleg, 0xffffffff);
  csr_set(mideleg, 0xffffffff);
  // csr_clear(mideleg, 1<<9); 
}


static inline void setup_pmp(void) {
  /*
   * Désactivation de la protection de la mémoire physique (PMP).
   *
   * Configuration de la PMP pour donner  un accès complet à la mémoire.
   * CSR concernés: pmpaddr0 and pmpcfg0.
   */

  // Ignore the illegal-instruction trap if PMPs aren't supported.
  uintptr_t pmpc = PMP_NAPOT | PMP_R | PMP_W | PMP_X;
  uintptr_t pmpa = ((uintptr_t)1 << (__riscv_xlen == 32 ? 31 : 53)) - 1;
  __asm__ __volatile__("la t0, 1f\n\t"
             "csrrw t0, mtvec, t0\n\t"
             "csrw pmpaddr0, %1\n\t"
             "csrw pmpcfg0, %0\n\t"
             ".align 2\n\t"
             "1: csrw mtvec, t0"
             : : "r"(pmpc), "r"(pmpa) : "t0");
}

/**
* This function performs the necessary configuration to switch from machine mode
* to supervisor mode, which is the mode in which we boot when starting the processor.
*/
static inline void enter_supervisor_mode() {
  // It is mandatory to configure Physical Memory Protection (PMP)
  // before switching to supervisor mode.
  setup_pmp();
  // Changing to supervisor mode
  // Store the address of the method to be executed in supervisor mode
  // in the mepc register. In this case, it will be the kernel_start method
  // defined in the start.c file.
  csr_write(mepc, kernel_start);

  // The purpose of the following code is to set the MPP field in the mstatus
  // register to the level to which we want to go after handling the current
  // interrupt. In our case, we want to switch from the current mode (machine mode)
  // to the supervisor mode, which is identified by the following bits: 01
  csr_set(mstatus, MSTATUS_MPP_0);
  csr_clear(mstatus, MSTATUS_MPP_1);

  #ifdef USER_PROCESS_DEBUG
    // The following lines allow us to access user pages from kernel mode.
    // This is not very secure because a malicious user can potentially make the supervisor
    // run code or modify memory that they are not allowed to access.
    // Solving this issue is not hard, but it was not done during this project.
    // Set the SUM value in sstatus to one to debug user processes.
    csr_set(sstatus, SSTATUS_SUM);
    debug_print_memory("Sum Attribute has been set correctly sstatus = %ld\n", csr_read(sstatus));
  #endif
  // The transition to the next level stored in mpp will be automatically done
  // with the mret instruction, which changes the mode based on what exists in mpp.
  mret();
}


/*
* boot_riscv
*
* This function is called from crtm.S
* At this stage, only the machine trap vector mtvec has been configured.
* The currently used stack is the machine stack that was allocated in crtm.S.
* This stack is also used to handle machine traps.
*
* The processor is still in machine mode.
*/
__attribute__((noreturn)) void boot_riscv(){
  // Configure machine-specific components (uart/htif, timer, disk and external interrupts).
  arch_setup();
  // Delegate interrupts and exceptions
  delegate_traps();

  #ifdef VIRTMACHINE
    // Enables timer interrupts for the machine mode
    // since this machine cannot handle time interrupts
    csr_set(mie, MIE_MTIE);
    csr_set(sie, SIE_STIE);
    csr_set(sie, 0x2); // We enable software interrupts
    csr_set(mie, 1<<13);
    csr_set(mie, 1<<11);
    csr_set(sie, 1<<9);
    csr_set(mstatus, MSTATUS_MIE);
  #else
    csr_set(mip, MIP_STIP); // Activate supervisor interrupt pending
    // Enables timer and external interrupts for the supervisor mode
    csr_set(sie, SIE_STIE);
    csr_set(sie, SIE_SEI);
    // Disable machine mode interrupts
    csr_clear(mstatus, MSTATUS_MIE);
  #endif

  // Initialize frame division of memory 
  // into 4K pages  
  init_frames();
  
  /**
  * This function will enter the supervisor mode and enable
  * supervisor mode interrupts.
  */
  enter_supervisor_mode();
  __builtin_unreachable();
}

