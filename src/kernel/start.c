/*
 * Projet PCSEA RISC-V
 *
 * Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 *
 * See license for license details.
 */

#include "drivers/splash.h"
#include "process/process.h" // for initialize_process_workflow
#include "memory/virtual_memory.h" //for set_up_virtual_memory
#include "riscv.h" // for csr_set
#include "timer.h"

#include "userspace_apps.h"
#include <stdio.h> 
#include "fs/fs.h"
#include "process/helperfunc.h"
//Testing
#include "test_interface.h"
#include "tests_fs/tests_fs.h"

int kernel_start() {
  // print_mem_symbols();
  printf("[KERNEL]Starting Kernel\n");
  if (set_up_virtual_memory() < 0) {
    puts("error while setting up virtual memory");
    exit(-1);
  } else{
    PRINT_GREEN("[MEM]Virtual memory is now active\n");
  }
  #ifdef VIRTMACHINE
    #ifdef TESTING
      if (set_up_file_system() < 0) {
        puts("error while setting up the file system");
        exit(-1);
      }
      if (clear_and_mount_test_part()< 0){
        PRINT_RED("error clearing and mounting route directory");
      }
      print_fs_details();
      kernel_drivers_tests(0);
      test_ext2_fs();
      print_fs_details();
    #else 
      if (set_up_file_system() < 0) {
        puts("error while setting up the file system");
        exit(-1);
      }
      if (mount_root_file_system()< 0){
        PRINT_RED("error while mounting file system");
      }
    #endif
  #endif
  if (initialize_process_workflow() < 0) {
    puts("error while setting up process");
    exit(-1);
  }else{
    PRINT_GREEN("[PROC]Processes were initiliazed\n");
  }
  PRINT_GREEN("[KERNEL]Kernel started\n");
  splash_screen();
  splash_vga_screen();
  #ifdef VIRTMACHINE
      set_machine_timer_interrupt(100);
  #endif
  csr_set(sstatus, MSTATUS_SIE); // active interruption in supervisor mode
                                 // note that there's a interrupt pending so
                                 // we will jump to that interruption automatically
  while (1)
    wfi();
}