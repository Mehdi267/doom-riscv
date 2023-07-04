#include "fs.h"
#include <stdint.h>
#include "super_block.h"
#include "mbr.h"
#include "ext2.h"
#include "stdio.h"
#include "string.h"

/*
  Source: https://www.nongnu.org/ext2-doc/ext2.pdf
  This example of taken for the link above and 
  the structure used in this example is very similar to 
  what wewill use if our implementation
  Disk Organization Example:
      Table
      Block Offset Length Description
      byte 0 512 Boot record (if present)
      byte 512 512 Additional boot record data (if present)

      Block Group 0, Blocks 1 to 1439:
      byte 1024 1024 Superblock
      block 2 1 Block group descriptor table
      block 3 1 Block bitmap
      block 4 1 Inode bitmap
      block 5 23 Inode table
      block 28 1412 Data blocks

  Disk Organization Details:
  Initially, we reserve 1024 bytes for the boot record, which may be added later.
  The superblock occupies a single block (1024 bytes).
  The size of bitmaps depends on the disk size.
  The number of inodes used is equal to 5% of the disk size.
  The remaining space is reserved for data blocks.
*/
int configure_ext2_file_system(uint8_t partition){
  uint32_t start = global_mdr->partitionTable[partition].startLBA;
  uint32_t size = global_mdr->partitionTable[partition].sizeLBA;
  configure_root_file_system(
    EXT2_PARTITION,//partition type number
    SUPER_BLOCK_LOC,//super block location
    EXT2_BLOCK_SIZE,
    partition
  );
  save_boot_record(start+BOOT_RECORD_LOC);
  PRINT_GREEN("BOOT RECORD SAVED");
  superblock_conf(start+SUPER_BLOCK_LOC, size); 
  PRINT_GREEN("SUPER BLOCK SAVED");
  return 0;
}


int save_boot_record(uint32_t boot_loc){
  //Master boot record
  char block[EXT2_BLOCK_SIZE];
  memset(block, 0, EXT2_BLOCK_SIZE);
  return save_fs_block(block, EXT2_BLOCK_SIZE, boot_loc);
};

int superblock_conf(uint32_t block_loc, 
                    uint32_t disk_size){

  //The number of inodes is equal to 10% of the 
  //size of the partition
  uint32_t number_of_inodes = (disk_size/10)/INODE_SIZE;
  uint32_t nb_blocks_inodes = (number_of_inodes*INODE_SIZE)
                                  /root_file_system->block_size;
  if ((nb_blocks_inodes*root_file_system->block_size 
        - number_of_inodes*INODE_SIZE)
        %root_file_system->block_size > 0){
    nb_blocks_inodes++;
  }
  uint32_t nb_block_inode_bitmap = nb_blocks_inodes/ 
                                  root_file_system->block_size;
  uint32_t nb_block_data_bitmap = (disk_size 
                                  - RESERVED_BLOCKS //Root and super user blocks 
                                  - nb_block_inode_bitmap
                                  - nb_blocks_inodes)
                                  /root_file_system->block_size;
  uint32_t nb_blocks_data = disk_size - (
                              nb_block_data_bitmap
                            + nb_blocks_inodes
                            + nb_block_inode_bitmap
                            + RESERVED_BLOCKS);
  //Super block
  super_block super;
  super.s_inodes_count = number_of_inodes;
  super.s_blocks_count = nb_blocks_data;                
  super.s_r_blocks_count = 0; //not used
  super.s_free_blocks_count = nb_blocks_data;
  super.s_first_data_block = SUPER_BLOCK_LOC;
  super.s_log_block_size = 2;
  super.s_log_frag_size = 0;//Not used
  super.s_blocks_per_group = nb_blocks_data; // only one group is used
  super.s_frags_per_group = 0; //not used             
  super.s_inodes_per_group = number_of_inodes; //only one group 
  super.s_mtime = 0;//timer is not being used, 
                    //i am not sure if it is even possile to get this value
  super.s_wtime = 0;
  super.s_mnt_count = 0;//not used
  super.s_max_mnt_count = 0;//not used
  super.s_magic = EXT2_SUPER_MAGIC;
  super.s_state = EXT2_VALID_FS;
  super.s_errors = EXT2_ERRORS_CONTINUE;
  super.s_minor_rev_level = 0;//not used
  super.s_lastcheck = 0;//not used
  super.s_checkinterval =0;//not used
  super.s_creator_os = EXT2_OS_LINUX; //posix linux ish

  super.s_rev_level = 0;// Not used
  super.s_def_resuid=0;  // Not used
  super.s_def_resgid=0; // Not used
  /* EXT2_DYNAMIC_REV Specific */
  super.s_first_ino = EXT2_GOOD_OLD_FIRST_INO; // First non-reserved inode
  super.s_inode_size = INODE_SIZE; // Size of inode structure
  super.s_block_group_nr=1; // only one group
  super.s_feature_compat = 0; // none of the features that we described are being used
  super.s_feature_incompat = 0xffff; // Incompatible feature set flags
  super.s_uuid[0] = 22; // random value
  return save_fs_block((char*)&super, sizeof(super_block), block_loc);
}
