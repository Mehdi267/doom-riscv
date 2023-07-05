#include "fs.h" //pointless
#include "stdio.h" //to do prints
#include "mbr.h" //to handle mbr init and usage
#include "disk_buffer.h"
#include "stdlib.h" //disk cache struct and other constants
#include <stdint.h> //uint
#include <string.h> // memset
#include "drivers/disk_device.h" //to do disk operations
#include "ext2.h"
#include "super_block.h"

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

int mount_custom_fs(uint8_t partition){
  if (!(partition >=1 && partition <= NB_PARTITIONS)){
    PRINT_RED("Partition number must be between 1 and NB_PARTITIONS(4)");
    return -1;
  }
  if (global_mbr == 0){
    return -1;
  }
  if (global_mbr->partitionTable[partition].type != EXT2_PARTITION){
    return -1;
  }
  return configure_root_file_system(
    EXT2_PARTITION,//partition type number
    SUPER_BLOCK_LOC,//super block location
    EXT2_BLOCK_SIZE,  partition);
}

int mount_root_file_system(){
  if (global_mbr == 0){
    return -1;
  }
  for (int i = 0; i < NB_PARTITIONS; i++){
    if(global_mbr->partitionTable[i].type == EXT2_PARTITION){
      PRINT_GREEN("Ext2 file system was found\n");
      return configure_root_file_system(
        EXT2_PARTITION,//partition type number
        SUPER_BLOCK_LOC,//super block location
        EXT2_BLOCK_SIZE,
        i
      );
    }
  }
  PRINT_RED("No ext2 file system was found\n");
  return -1;
}

void load_and_print_superblock(){
  if (root_file_system == 0){
    return;
  }
  char* data = disk_read_block(
          root_file_system->superblock_loc);
  print_super_block((super_block*) data);
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
  }else{
    sync();
    free_cache_list();
  }
  root_file_system->fs_type = fs_type; 
  root_file_system->superblock_loc = superblock_loc; 
  root_file_system->block_size = block_size; 
  root_file_system->partition = partition; 
  return 0;
}

