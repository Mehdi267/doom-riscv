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
#include "timer.h"

int kernel_start() {
  splash_screen();
  splash_vga_screen();

  if (initialize_process_workflow() < 0) {
    puts("error while setting up process");
    exit(-1);
  }
  /**
   * These lines are used for debugging purposes, they are not relevant
   * please don't remove them
   */
  // char str[80];
  // sprintf(str,"%li",csr_read(mstatus));
  // puts(str);

  /* set_supervisor_timer_interrupt(50); // setting the 1st interrupt */
  assert(start(test10,4000, 192, "test10", (void *)0) != -1);
  assert(start(test12,4000, 128, "test12", (void *)0) != -1);
  assert(start(test14,4000, 128, "test12", (void *)0) != -1);
  scheduler();
  while(1) wfi();

  /* if (activate_and_launch_custom_process(get_process_struct_of_pid(1)) < 0) { */
  /*   return -1; */
  /* } */
  /* while (1) */
  /*   wfi(); // endort le processeur en attente d'une interruption */

  // exit(kernel_tests(NULL));
}
