#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "syscall.h"



int main() {
  int option = 0;
  char input[256];

  while (option != 8) {
    printf("Choose an option:\n");
    printf("1. Print partition status\n");
    printf("2. Create a partition\n");
    printf("3. Delete a partition\n");
    printf("4. Reset disk\n");
    printf("5. Sync\n");
    printf("6. Clear disk cache\n");
    printf("7. Print mount fs info\n");
    printf("8. Exit\n");
    printf("Enter option number: ");
    cons_read(input, sizeof(input));
    option = atoi(input);
    memset(input, 0, 256);
    if (option == 1) {
        display_partions();
    } else if (option == 2) {
        uint32_t start;
        uint32_t size;
        uint8_t type;
        printf("Enter start address: ");
        cons_read(input, sizeof(input));
        start = atoi(input);
        memset(input, 0, 256);
        printf("Enter size: ");
        cons_read(input, sizeof(input));
        size = atoi(input);
        memset(input, 0, 256);
        printf("Enter partition type: (1 for a test partition and 131 for ext2 partition)\n");
        cons_read(input, sizeof(input));
        type = atoi(input);
        memset(input, 0, 256);
        create_partition(start, size, type);
    } else if (option == 3) {
        uint8_t partitionNumber;
        printf("Enter partition number: ");
        cons_read(input, sizeof(input));
        partitionNumber = atoi(input);
        memset(input, 0, 256);
        delete_partition(partitionNumber);
    } else if (option == 4) {
      memset(input, 0, sizeof(input));
      while (input[0] != 'y' && input[0] != 'n') {
          printf("Are you sure (y/n)? ");
          cons_read(input, sizeof(input));
          if (input[0] == 'y') {
              if (reset_disk() < 0) {
                  printf("Reset failed\n");
              } else {
                  printf("Reset was successful\n");
              }
          }
      }
    } 
    else if (option == 5) {
      if (sync()<0){
        PRINT_RED("SYNC FAILED\n");
      }
    }
    else if (option == 6){
      if (clear_disk_cache()<0){
        PRINT_RED("clear_disk_cache FAILED\n");
      }
    }
    else if (option == 7){
      print_fs_details();
    }
    else if (option == 8) {
      printf("Exiting...\n");
      return 0;
    } else {
      printf("Invalid option. Please try again.\n");
    }
    printf("\n");
  }

  return 0;
}
