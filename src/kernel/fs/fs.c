#include "fs.h" //pointless
#include "stdio.h" //to do prints
#include "mbr.h" //to handle mbr init and usage
#include "stdlib.h" //disk cache struct and other constants
#include <stdint.h> //uint
#include <string.h> // memset
#include "drivers/disk_device.h" //to do disk operations

int set_up_file_system() {
  printf("Setting up file system\n");
  if (find_mbr() < 0) {
    printf("\033[0;31mMbr was not found\033[0m\n");  // Print in red
    printf("\033[0;32mSetting up mbr\033[0m\n");     // Print in red
    return set_up_mbr();
  } else {
    printf("\033[0;32mMbr was found\033[0m\n");      // Print in green
  }
  print_partition_status();
  return 0;
}

file_system_t* root_file_system;

int configure_root_file_system(
  uint8_t fs_type,
  uint32_t superblock_loc,
  uint32_t block_size,
  uint8_t partition
  ){
  if (root_file_system == 0){
    root_file_system = (file_system_t*)
                      malloc(sizeof(file_system_t));
    if (root_file_system == 0){
      //No space
      return -1;
    }
  } 
  root_file_system->fs_type = fs_type; 
  root_file_system->superblock_loc = superblock_loc; 
  root_file_system->block_size = block_size; 
  root_file_system->partition = partition; 
  return -1;
}



int save_fs_block(char* data,
                  uint32_t data_size,
                  uint32_t block_number
                  ){
  if (data == 0 || data_size > root_file_system->block_size){
    return -1;
  }
  int res;
  uint32_t frag_size = data_size%BLOCK_SIZE;
  uint32_t number_disk_b = root_file_system->block_size/BLOCK_SIZE;
  char final_block[BLOCK_SIZE];
  memset(final_block,0,BLOCK_SIZE);
  memcpy(final_block,
        data+data_size-frag_size,
        frag_size);
  disk_op op_write; 
  for (uint32_t i = 0; i < number_disk_b; i++){
    op_write.blockNumber = block_number+i;
    op_write.type = WRITE;
    if (i != number_disk_b-1){
      op_write.data = (unsigned char *)data+i*BLOCK_SIZE;
    }
    else{
      op_write.data = (unsigned char *)final_block;
    }
    if (disk_dev->write_disk(&op_write)<0){
      printf("A save operation failed");
      res = -1;
    }
  }
  return res;
}


char** disk_read_block(uint32_t fs_block_number){
  return 0;
}
