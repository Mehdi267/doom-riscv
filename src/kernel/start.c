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

#include <stdbool.h>
#include <stdio.h> 
#include "test_interface.h"
#include "tests_fs/tests_fs.h"
#include "fs/fs.h"
#include "process/helperfunc.h"

void reverse_video(){
  // for (uint64_t start = 0x000000003000000; start < 0x000000003000000+ 0x10000; start++){
  //   *((char*)start) = 97;
  // }
  // char *add;
  // add = (char*)0x000000003000000+0x0604;
  // // print_block(add, 50);
  // printf("%s\n", add);
  // add = (char*)0x000000002000000+0x0604;
  // printf("%s\n", add);
}

int kernel_start() {
  // print_mem_symbols();
  printf("\nStarting Kernel\n");
  // char a[100];
  // cons_read(a, 50);
  reverse_video();
  if (set_up_virtual_memory() < 0) {
    puts("error while setting up virtual memory");
    exit(-1);
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
      // print_fs_details();
      // test_ext2_fs();
      // print_fs_details();
      // print_dir_list(get_inode(11), true);
    #endif
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