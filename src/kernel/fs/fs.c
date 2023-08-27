#include "fs.h" //pointless
#include "stdio.h" //to do prints
#include "mbr.h" //to handle mbr init and usage
#include "disk_buffer.h"
#include "stdlib.h" //disk cache struct and other constants
#include <stdint.h> //uint
#include <string.h> // memset
#include "drivers/disk_device.h" //to do disk operations
#include "ext2.h"  // 
#include "super_block.h" // 
#include "chdev.h" // 
#include "hash.h" // hash table related
#include "logger.h" // Used to log and print fs conf 
#include "inode.h" 

#include "../memory/frame_dist.h" 


file_system_t* root_file_system = 0;

int set_up_file_system() {
  printf("Setting up file system\n");
  if (find_mbr() < 0) {
    printf("\033[0;31mMbr was not found\033[0m\n");  // Print in red
    printf("\033[0;32mSetting up mbr\033[0m\n");     // Print in red
    return set_up_mbr();
  } else {
    printf("\033[0;32mMbr was found\033[0m\n");      // Print in green
  }
  if (init_fs_drivers()<0){
    return -1;
  }
  print_partition_status();
  return 0;
}

int clear_and_mount_test_part(){
  if (setup_test_partition(EXT2_PARTITION)<-1){
    return -1;
  }
  if (mount_custom_fs(0)){
    return -1;
  }
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
  if (configure_root_file_system(
    EXT2_PARTITION,//partition type number
    SUPER_BLOCK_LOC,//super block location
    BLOCK_TABLE_BLOCK,
    EXT2_BLOCK_SIZE,  partition)<0){
      return -1;
  }
  if (config_super_block()){
    return -1;
  }
  if (config_blk_desc_table()){
    return -1;
  }
  return 0;
}

int mount_root_file_system(){
  if (global_mbr == 0){
    return -1;
  }
  for (int i = 0; i < NB_PARTITIONS; i++){
    if(global_mbr->partitionTable[i].type == EXT2_PARTITION){
      PRINT_GREEN("Ext2 file system was found\n");
      if (configure_root_file_system(
        EXT2_PARTITION,//partition type number
        SUPER_BLOCK_LOC,//super block location
        BLOCK_TABLE_BLOCK,
        EXT2_BLOCK_SIZE,  
        i)<0){
          return -1;
      }
      if (config_super_block()){
        return -1;
      }
      if (config_blk_desc_table()){
        return -1;
      }
      return 0;
    }
  }
  PRINT_RED("No ext2 file system was found\n");
  return -1;
}

void load_and_print_superblock(){
  if (root_file_system == 0){
    PRINT_RED("[print]No super block was found\n");
    return;
  }
  char* data = disk_read_block(
          root_file_system->superblock_loc, LOCK);
  print_super_block((super_block*) data);
  unlock_cache(root_file_system->superblock_loc);
}

void load_and_print_desc_table(){
  if (root_file_system == 0){
    PRINT_RED("[print]No desc block was found\n");
    return;
  }
  char* data = disk_read_block(
          root_file_system->desc_table_loc, LOCK);
  printblock_group_descriptor((block_group_descriptor*) data);
  unlock_cache(root_file_system->desc_table_loc);
}

void print_fs_details(){
  print_mem_usage();
  load_and_print_superblock();
  load_and_print_desc_table();
  if (root_file_system == 0){
    return;
  }
  print_cache_details(root_file_system->inode_list);
  print_data_bitmap();
  print_inode_bitmap();
}


int configure_root_file_system(
  uint8_t fs_type,
  uint32_t superblock_loc,
  uint32_t desc_table_loc,
  uint32_t block_size,
  uint8_t partition
  ){
  print_fs_no_arg("[fs]configure_root_file_system was called\n");
  if (root_file_system == 0){
    root_file_system = (file_system_t*)
                      malloc(sizeof(file_system_t));
    if (root_file_system == 0){
      //No space
      PRINT_RED("No space when alloc root fs");
      return -1;
    }
  } else{
    print_fs_no_arg("[fs]root_file_system was found\n");
    if (root_file_system->super_block != 0){
      free(root_file_system->super_block);
    } 
    if (root_file_system->desc_table != 0){
      free(root_file_system->desc_table);
    }
    if (root_file_system->inode_hash_table != 0){
      hash_destroy(root_file_system->inode_hash_table);
    }
    free_inode_list(root_file_system->inode_list);
    sync_all();
    free_cache_list();
  }
  root_file_system->fs_type = fs_type; 
  root_file_system->superblock_loc = superblock_loc; 
  root_file_system->desc_table_loc = desc_table_loc; 
  root_file_system->block_size = block_size; 
  root_file_system->partition = partition;
  root_file_system->super_block = 0; 
  root_file_system->desc_table = 0;
  root_file_system->inode_list = 0;
  root_file_system->inode_hash_table = (hash_t *)malloc(sizeof(hash_t));
  if (root_file_system->inode_hash_table == 0){
    return -1;
  }
  hash_init_direct(root_file_system->inode_hash_table);
  PRINT_GREEN("File system was configured\n");
  return 0;
}

int sync_all(){
  print_fs_no_arg("Sync has been called\n");
  int res = 0;
  if (sync()<0){
    res = -1;
  }
  if (sync_inodes()){
    res = -1;
  }
  print_fs_no_arg("Sync finished\n");
  return res;
}




