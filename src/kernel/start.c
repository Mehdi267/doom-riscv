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
  /* set_supervisor_timer_interrupt(50); // setting the 1st interrupt */
  /* assert(start(test10,4000, 192, "test10", (void *)0) != -1); */
  /* assert(start(test12,4000, 128, "test12", (void *)0) != -1); */
  /* assert(start(test14,4000, 128, "test14", (void *)0) != -1); */
  /* assert(start(test17, 4000, 128, "test17", (void *)0) != -1); */
  // we start the process who launch all the tests;
  // while(1){
  //   release_frame(get_frame());
  // }
  // assert(start(kernel_tests, 4000, 2, "kernel_tests", NULL) != -1);
  activate_and_launch_scheduler();
  while (1)
    wfi();
}
