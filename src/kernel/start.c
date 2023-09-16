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

#include "drivers/gpu_device.h" //gpu related methods
#define SCREENWIDTH 320
#define SCREENHEIGHT 200

struct Pixel {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;
};


void display_test() {
  printf("size of screen %ld\n", SCREENWIDTH*SCREENHEIGHT*sizeof(struct Pixel));
  void* frame = malloc(SCREENWIDTH*SCREENHEIGHT*sizeof(struct Pixel)); 
  struct Pixel* data = (struct Pixel*)(frame);
  // Initialize pixel colors
  unsigned char orientation = 6;
  while (1) {
    orientation += 40;
    for (int x = 0; x < SCREENWIDTH; x++) {
      for (int y = 0; y < SCREENHEIGHT; y++) {
        if (x < SCREENWIDTH / 2 && y < SCREENHEIGHT / 2) {
          // Top-left corner: Red
          (data + y * SCREENWIDTH + x)->red = 255;
          (data + y * SCREENWIDTH + x)->green = 0;
          (data + y * SCREENWIDTH + x)->blue = orientation;
        } else if (x >= SCREENWIDTH / 2 && y < SCREENHEIGHT / 2) {
          // Top-right corner: Blue
          (data + y * SCREENWIDTH + x)->red = 0;
          (data + y * SCREENWIDTH + x)->green = orientation;
          (data + y * SCREENWIDTH + x)->blue = 255;
        } else if (x < SCREENWIDTH / 2 && y >= SCREENHEIGHT / 2) {
          // Bottom-left corner: Green
          (data + y * SCREENWIDTH + x)->red = 0;
          (data + y * SCREENWIDTH + x)->green = 255;
          (data + y * SCREENWIDTH + x)->blue = orientation;
        } else {
          // Remaining corner: Any color (e.g., yellow)
          (data + y * SCREENWIDTH + x)->red = 255;
          (data + y * SCREENWIDTH + x)->green = 255+orientation;
          (data + y * SCREENWIDTH + x)->blue = orientation;
        }
        (data + y * SCREENWIDTH + x)->alpha = 255; // Alpha component (transparency)
      }
    }
    // Ensure that orientation doesn't go beyond 255
    if (orientation > 255) {
        orientation = 0;
    }
    int i = 0;
    while(i<50000000){i++;}
    // printf("Updating data\n");
    gpu_dev->update_data(data, 0, 0, SCREENWIDTH, SCREENHEIGHT);
  }
  free(frame);
}


int kernel_start() {
  print_mem_symbols();
  printf("\nStarting Kernel\n");
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
  // display_test();
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