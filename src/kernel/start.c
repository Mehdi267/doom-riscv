/*
 * Projet PCSEA RISC-V
 *
 * Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 *
 * See license for license details.
 */
#include "drivers/splash.h" // for splash_screen and splash_vga_screen
#include "process/process.h" // for initialize_process_workflow
#include "memory/virtual_memory.h" //for set_up_virtual_memory
#include "riscv.h" // for csr_set

int kernel_start() {
  puts("Inside kernel start\n");
  if (set_up_virtual_memory() < 0) {
    puts("error while setting up virtual memory");
    exit(-1);
  }
  splash_screen();
  splash_vga_screen();
  if (initialize_process_workflow() < 0) {
    puts("error while setting up process");
    exit(-1);
  }
  csr_set(sstatus, MSTATUS_SIE); // active interruption in supervisor mode
                                 // not that there's a interrupt pending so
                                 // we will jump to that interruption automatically
  while (1)
    wfi();
}
