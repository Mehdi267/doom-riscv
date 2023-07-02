/*
 * Projet PCSEA RISC-V
 *
 * Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 *
 * See license for license details.
 */
#include "drivers/splash.h" // for splash_screen and splash_vga_screen
#include "memory/frame_dist.h"
#include "process/process.h" // for initialize_process_workflow
#include "memory/virtual_memory.h" //for set_up_virtual_memory
#include "riscv.h" // for csr_set
#include "timer.h"
#include "test_interface.h"

#include <stdio.h>
#include "drivers/console.h"
#include "fs/fs.h"
#include "tests_fs/tests_fs.h"

int kernel_start() {
  printf("\nStarting Kernel\n");
  if (set_up_virtual_memory() < 0) {
    puts("error while setting up virtual memory");
    exit(-1);
  }
  #ifdef VIRTMACHINE
    if (set_up_file_system() < 0) {
      puts("error while setting up the file system");
      exit(-1);
    }
  #endif
  //kernel_drivers_tests(0);
  splash_screen();
  splash_vga_screen();
  if (initialize_process_workflow() < 0) {
    puts("error while setting up process");
    exit(-1);
  }

  #ifdef VIRTMACHINE
      set_machine_timer_interrupt(100);
  #endif
  csr_set(sstatus, MSTATUS_SIE); // active interruption in supervisor mode
                                 // note that there's a interrupt pending so
                                 // we will jump to that interruption automatically
  while (1)
    wfi();
}
