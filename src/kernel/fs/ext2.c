#include "fs.h"
#include <stdint.h>
#include "super_block.h"
#include "mbr.h"
#include "ext2.h"
#include "stdio.h"
#include "string.h"
#include "disk_buffer.h"

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
  if (global_mbr == 0){
    return -1;
  }
  uint32_t size = global_mbr->partitionTable[partition].sizeLBA;
  configure_root_file_system(
    EXT2_PARTITION,//partition type number
    SUPER_BLOCK_LOC,//super block location
    BLOCK_TABLE_BLOCK,
    EXT2_BLOCK_SIZE,
    partition
  );
  save_boot_record(BOOT_RECORD_LOC);
  PRINT_GREEN("Boot record saved\n");
  superblock_conf(SUPER_BLOCK_LOC, size); 
  PRINT_GREEN("Super block saved\n");
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
  PRINT_GREEN("Configuring super block\n");
  //The number of inodes is equal to 10% of the 
  //size of the partition
  int blk_ratio = root_file_system->block_size/BLOCK_SIZE;
  uint32_t number_of_inodes = (disk_size*BLOCK_SIZE/10)/INODE_SIZE;
  uint32_t nb_blocks_inodes = (number_of_inodes*INODE_SIZE)
                                  /root_file_system->block_size;
  if ((nb_blocks_inodes*root_file_system->block_size 
        - number_of_inodes*INODE_SIZE)
        %root_file_system->block_size > 0){
    nb_blocks_inodes++;
  }
  uint32_t nb_block_inode_bitmap = nb_blocks_inodes/ 
                                  root_file_system->block_size;
  if (nb_blocks_inodes%root_file_system->block_size > 0)
  {nb_block_inode_bitmap++;}
  uint32_t nb_block_data_bitmap = (disk_size/blk_ratio
                                  - RESERVED_BLOCKS //Root and super user blocks 
                                  - nb_block_inode_bitmap
                                  - nb_blocks_inodes)
                                  /root_file_system->block_size;
  if ((disk_size/blk_ratio
      - RESERVED_BLOCKS 
      - nb_block_inode_bitmap
      - nb_blocks_inodes)
      %root_file_system->block_size>0){
    nb_block_data_bitmap++;
  }
  uint32_t nb_blocks_data = disk_size/blk_ratio - (
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
  super.s_free_inodes_count = number_of_inodes;
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
  super.s_checkinterval = 0;//not used
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
  if(save_fs_block((char*)&super, sizeof(super_block), block_loc)<0){
    PRINT_RED("fs superblock failed to save\n");
    return -1;
  }
  BlockGroupDescriptor blk_des;
  blk_des.bg_block_bitmap = BLOCK_TABLE_BLOCK+1;
  blk_des.bg_inode_bitmap = blk_des.bg_block_bitmap+
                            nb_block_data_bitmap;
  blk_des.bg_inode_table = blk_des.bg_inode_bitmap + 1;  
  blk_des.bg_free_blocks_count = nb_blocks_data;
  blk_des.bg_free_inodes_count = nb_blocks_inodes;
  blk_des.bg_used_dirs_count = 0;
  if(save_fs_block((char*)&blk_des, sizeof(BlockGroupDescriptor), block_loc+1)<0){
    PRINT_RED("fs superblock failed to save\n");
    return -1;
  }
  return 0;
}



void print_super_block(super_block* sb){
  if (sb == 0){
    return;
  }
  printf("-----------super block------------\n");
  printf("s_inodes_count = %d\n", sb->s_inodes_count);                // Total number of inodes
  printf("s_blocks_count = %d\n", sb->s_blocks_count);                // Total number of blocks
  printf("s_r_blocks_count = %d\n", sb->s_r_blocks_count);              // Number of reserved blocks for super use
  printf("s_free_blocks_count = %d\n", sb->s_free_blocks_count);           // Number of free blocks

  printf("s_free_inodes_count = %d\n", sb->s_free_inodes_count);           // Number of free inodes
  printf("s_first_data_block = %d\n", sb->s_first_data_block);            // Block number of first data block
  printf("s_log_block_size = %d\n", sb->s_log_block_size);              // Block size (log2)
  printf("s_log_frag_size = %d\n", sb->s_log_frag_size);               // Fragment size (log2)
  printf("s_blocks_per_group = %d\n", sb->s_blocks_per_group);            // Blocks per group
  printf("s_frags_per_group = %d\n", sb->s_frags_per_group);             // Fragments per group
  printf("s_inodes_per_group = %d\n", sb->s_inodes_per_group);            // Inodes per group
  printf("s_mtime = %d\n", sb->s_mtime);                       // Mount time
  printf("s_wtime = %d\n", sb->s_wtime);                       // Write time
  printf("s_mnt_count = %d\n", sb->s_mnt_count);                   // Mount count
  printf("s_max_mnt_count = %d\n", sb->s_max_mnt_count);               // Maximal mount count
  printf("s_magic = %d\n", sb->s_magic);                       // Magic signature
  printf("s_state = %d\n", sb->s_state);                       // File system state
  printf("s_errors = %d\n", sb->s_errors);                      // Behaviour when detecting errors
  printf("s_minor_rev_level = %d\n", sb->s_minor_rev_level);             // Minor revision level
  printf("s_lastcheck = %d\n", sb->s_lastcheck);                   // Last check time
  printf("s_checkinterval = %d\n", sb->s_checkinterval);               // Check interval
  printf("s_creator_os = %d\n", sb->s_creator_os);                  // OS ID
  printf("s_rev_level = %d\n", sb->s_rev_level);                   // Revision level
  printf("s_def_resuid = %d\n", sb->s_def_resuid);                  // Default uid for reserved blocks
  printf("s_def_resgid = %d\n", sb->s_def_resgid);                  // Default gid for reserved blocks

  printf("s_first_ino = %d\n", sb->s_first_ino);                   // First non-reserved inode
  printf("s_inode_size = %d\n", sb->s_inode_size);                  // Size of inode structure
  printf("s_block_group_nr = %d\n", sb->s_block_group_nr);              // Block group number of this superblock
  printf("s_feature_compat = %d\n", sb->s_feature_compat);              // Compatible feature set flags
  printf("s_feature_incompat = %d\n", sb->s_feature_incompat);            // Incompatible feature set flags
  printf("s_feature_ro_compat = %d\n", sb->s_feature_ro_compat);           // Readonly-compatible feature set flags
  printf("-----------super block end------------\n");
}

void printBlockGroupDescriptor(BlockGroupDescriptor* bgd) {
  if (bgd == 0){
    return;
  }
  printf("----------Block Group Descriptor:----\n");
  printf("Block Bitmap: %u\n", bgd->bg_block_bitmap);
  printf("Inode Bitmap: %u\n", bgd->bg_inode_bitmap);
  printf("Inode Table: %u\n", bgd->bg_inode_table);
  printf("Free Blocks Count: %hu\n", bgd->bg_free_blocks_count);
  printf("Free Inodes Count: %hu\n", bgd->bg_free_inodes_count);
  printf("Used Directories Count: %hu\n", bgd->bg_used_dirs_count);
  printf("Padding: %hu\n", bgd->bg_pad);
  printf("Reserved: ");
  for (int i = 0; i < 12; i++) {
    printf("%u ", bgd->bg_reserved[i]);
  }
  printf("\n----------Block Group end:----\n");
  printf("\n");
}
