#include "fs.h"
#include "stdio.h"
#include "mbr.h"

int set_up_file_system() {
  printf("Setting up file system\n");
  if (find_mbr() < 0) {
    printf("\033[0;31mMbr was not found\033[0m\n");  // Print in red
    printf("\033[0;32mSetting up mbr\033[0m\n");     // Print in red
    return set_up_mbr();
  } else {
    printf("\033[0;32mMbr was found\033[0m\n");      // Print in green
    set_up_mbr();
  }
  print_partition_status();
  return 0;
}