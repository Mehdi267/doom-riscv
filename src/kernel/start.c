/*
 * Projet PCSEA RISC-V
 *
 * Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 *
 * See license for license details.
 */

//#include <unistd.h>
#include "assert.h"
#include "drivers/splash.h"
#include "memory/frame_dist.h"
#include "process/helperfunc.h"
#include "process/process.h"
#include "process/scheduler.h"
#include "riscv.h"
#include "stddef.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "tests/tests.h"
#include "memory/virtual_memory.h"
#include <stdint.h>

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
