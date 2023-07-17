#include "fs.h"
#include <stdint.h>
#include "super_block.h"
#include "mbr.h"
#include "ext2.h"
#include "stdio.h"
#include "string.h"
#include "disk_buffer.h"
#include "inode.h"
#include "logger.h"
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
  print_fs_no_arg("[ext2]conf ext2 method was called\n");
  if (global_mbr == 0){
    print_fs_no_arg("[ext2] global mbr not found\n");
    return -1;
  }
  uint32_t size = global_mbr->partitionTable[partition].sizeLBA;
  if (configure_root_file_system(
    EXT2_PARTITION,//partition type number
    SUPER_BLOCK_LOC,//super block location
    BLOCK_TABLE_BLOCK,
    EXT2_BLOCK_SIZE,
    partition
  )<0){
    PRINT_RED("[ext2]conf root fs FAILED\n");
    return -1;
  }
  PRINT_GREEN("[ext2]conf root fs success\n");
  if (save_boot_record(BOOT_RECORD_LOC)<0){
    PRINT_RED("[EXT2] save_boot_record failed\n");
    return -1;
  }
  PRINT_GREEN("Boot record saved\n");
  if (superblock_conf(SUPER_BLOCK_LOC, size)<0){
    PRINT_RED("[EXT2] superblock_conf failed\n");
    return -1;
  }
  PRINT_GREEN("Super block saved\n");
  if (configure_reserved_inodes()<0){
    PRINT_RED("[EXT2] configure_reserved_inodes failed\n");
    return -1;
  }
  PRINT_GREEN("configure_reserved_inodes saved\n");
  if (configure_reserved_data()<0){
    PRINT_RED("[EXT2] configure_reserved_data failed\n");
    return -1;
  }
  print_fs_details();
  if (configure_root_inode()<0){
    PRINT_RED("[EXT2] configure_root_inode failed\n");
    return -1;
  }
  PRINT_GREEN("Root directory was built\n");
  print_fs_no_arg("[ext2]conf finished\n");
  return 0;
}



int configure_reserved_data(){
  block_group_descriptor* desc_table = get_desc_table();
  uint32_t block_bitmap = desc_table->bg_block_bitmap;
  char* data_bitmap = disk_read_block(block_bitmap);
  *(data_bitmap) = 0x01;
  super_block* super = (super_block*) get_super_block();
  super->s_free_blocks_count--;
  desc_table->bg_free_blocks_count--;
  if (save_super_block()<0){
    return -1;
  }
  if (save_blk_desc_table()<0){
    return -1;
  }
  return save_fs_block(data_bitmap,
           root_file_system->block_size, 
           block_bitmap);
}

int configure_reserved_inodes(){
  block_group_descriptor* desc_table = get_desc_table();
  uint32_t inode_bitmap_b = desc_table->bg_inode_bitmap;
  char* block_inode = disk_read_block(inode_bitmap_b);
  *(block_inode) = 0xff;
  *(block_inode+1) = 0x0f;
  super_block* super = (super_block*) get_super_block();
  super->s_free_inodes_count -= NUMBER_OF_RESERVED_INODES;
  desc_table->bg_free_inodes_count -= NUMBER_OF_RESERVED_INODES;
  if (save_super_block()<0){
    return -1;
  }
  if (save_blk_desc_table()<0){
    return -1;
  }
  return save_fs_block(block_inode,
           root_file_system->block_size, 
           inode_bitmap_b);
}

int configure_root_inode(){
  inode_t* inode = get_inode(EXT2_GOOD_OLD_FIRST_INO);
  inode->i_mode = EXT2_S_IFDIR;             
  inode->i_uid = 0;              
  inode->i_size = 0;             
  inode->i_atime = 0;            
  inode->i_ctime = 0;            
  inode->i_mtime = 0;            
  inode->i_dtime = 0;            
  inode->i_gid = 0;              
  inode->i_links_count = 0;      
  inode->i_blocks = 0;//these options where not used           
  inode->i_flags = 0xfffffff;            
  inode->i_osd1 = 0;             
  inode->i_generation = 1;       
  inode->i_file_acl = 0;         
  inode->i_dir_acl = 0;          
  inode->i_faddr = 0;
  if (put_inode(inode, 0, RELEASE_INODE) < 0){
    return -1;
  }
  block_group_descriptor* desc_table = 
    (block_group_descriptor*) get_desc_table();
  desc_table->bg_used_dirs_count++;
  return 0;
}

int save_boot_record(uint32_t boot_loc){
  //Master boot record
  char* block =(char*) malloc(EXT2_BLOCK_SIZE);
  memset(block, 0, EXT2_BLOCK_SIZE);
  if (save_fs_block(block, EXT2_BLOCK_SIZE, boot_loc)<0){
    free(block);
    return -1;
  }
  free(block);
  return 0;
};

int superblock_conf(uint32_t block_loc, 
                    uint32_t disk_size){
  PRINT_GREEN("Configuring super block\n");
  //The number of inodes is equal to 10% of the 
  //size of the partition
  int blk_ratio = root_file_system->block_size/BLOCK_SIZE;
  uint32_t number_of_inodes = (disk_size*BLOCK_SIZE/10)/INODE_SIZE;
  uint32_t nb_blocks_inodes = (number_of_inodes*INODE_SIZE)
                                  /(root_file_system->block_size);
  if ((nb_blocks_inodes*root_file_system->block_size 
        - number_of_inodes*INODE_SIZE)
        %(root_file_system->block_size) > 0){
    nb_blocks_inodes++;
  }
  uint32_t nb_block_inode_bitmap = nb_blocks_inodes/ 
                                  (8*root_file_system->block_size);
  if (nb_blocks_inodes%root_file_system->block_size > 0)
  {nb_block_inode_bitmap++;}
  uint32_t nb_block_data_bitmap = (disk_size/blk_ratio
                                  - RESERVED_BLOCKS //Root and super user blocks 
                                  - nb_block_inode_bitmap
                                  - nb_blocks_inodes)
                                  /(8*root_file_system->block_size);
  if ((disk_size/blk_ratio
      - RESERVED_BLOCKS 
      - nb_block_inode_bitmap
      - nb_blocks_inodes)
      %(8*root_file_system->block_size)>0){
    nb_block_data_bitmap++;
  }
  uint32_t nb_blocks_data = disk_size/blk_ratio - (
                              nb_block_data_bitmap
                            + nb_blocks_inodes
                            + nb_block_inode_bitmap
                            + RESERVED_BLOCKS);
  //Super block
  super_block* super = malloc(sizeof(super_block));
  memset(super, 0, sizeof(super_block));
  super->s_inodes_count = number_of_inodes;
  debug_print_v_fs("[ext2]super->s_inodes_count = %d\n", super->s_inodes_count);
  super->s_blocks_count = nb_blocks_data;                
  debug_print_v_fs("[ext2]super->s_blocks_count = %d\n", super->s_blocks_count);
  super->s_r_blocks_count = 0; //not used
  debug_print_v_fs("[ext2]super->s_r_blocks_count = %d\n", super->s_r_blocks_count);
  super->s_free_blocks_count = nb_blocks_data;
  debug_print_v_fs("[ext2]super->s_free_blocks_count = %d\n", super->s_free_blocks_count);
  super->s_free_inodes_count = number_of_inodes;
  debug_print_v_fs("[ext2]super->s_free_inodes_count = %d\n", super->s_free_inodes_count);
  super->s_first_data_block = disk_size/blk_ratio-nb_blocks_data;
  debug_print_v_fs("[ext2]super->s_first_data_block = %d\n", super->s_first_data_block);
  super->s_log_block_size = 2;
  debug_print_v_fs("[ext2]super->s_log_block_size = %d\n", super->s_log_block_size);
  super->s_log_frag_size = 0;//Not used
  debug_print_v_fs("[ext2]super->s_log_frag_size = %d\n", super->s_log_frag_size);
  super->s_blocks_per_group = nb_blocks_data; // only one group is used
  debug_print_v_fs("[ext2]super->s_blocks_per_group = %d\n", super->s_blocks_per_group);
  super->s_frags_per_group = 0; //not used             
  debug_print_v_fs("[ext2]super->s_frags_per_group = %d\n", super->s_frags_per_group);
  super->s_inodes_per_group = number_of_inodes; //only one group 
  debug_print_v_fs("[ext2]super->s_inodes_per_group = %d\n", super->s_inodes_per_group);
  super->s_mtime = 0;//timer is not being used, 
  debug_print_v_fs("[ext2]super->s_mtime = %d\n", super->s_mtime);
                    //i am not sure if it is even possile to get this value
  super->s_wtime = 0;
  debug_print_v_fs("[ext2]super->s_wtime = %d\n", super->s_wtime);
  super->s_mnt_count = 0;//not used
  debug_print_v_fs("[ext2]super->s_mnt_count = %d\n", super->s_mnt_count);
  super->s_max_mnt_count = 0;//not used
  debug_print_v_fs("[ext2]super->s_max_mnt_count = %d\n", super->s_max_mnt_count);
  super->s_magic = EXT2_SUPER_MAGIC;
  debug_print_v_fs("[ext2]super->s_magic = %d\n", super->s_magic);
  super->s_state = EXT2_VALID_FS;
  debug_print_v_fs("[ext2]super->s_state = %d\n", super->s_state);
  super->s_errors = EXT2_ERRORS_CONTINUE;
  debug_print_v_fs("[ext2]super->s_errors = %d\n", super->s_errors);
  super->s_minor_rev_level = 0;//not used
  debug_print_v_fs("[ext2]super->s_minor_rev_level = %d\n", super->s_minor_rev_level);
  super->s_lastcheck = 0;//not used
  debug_print_v_fs("[ext2]super->s_lastcheck = %d\n", super->s_lastcheck);
  super->s_checkinterval = 0;//not used
  debug_print_v_fs("[ext2]super->s_checkinterval = %d\n", super->s_checkinterval);
  super->s_creator_os = EXT2_OS_LINUX; //posix linux ish
  debug_print_v_fs("[ext2]super->s_creator_os = %d\n", super->s_creator_os);

  super->s_rev_level = 0;// Not used
  debug_print_v_fs("[ext2]super->s_rev_level = %d\n", super->s_rev_level);
  super->s_def_resuid=0;  // Not used
  debug_print_v_fs("[ext2]super->s_def_resuid = %d\n", super->s_def_resuid);
  super->s_def_resgid=0; // Not used
  debug_print_v_fs("[ext2]super->s_def_resgid = %d\n", super->s_def_resgid);
  /* EXT2_DYNAMIC_REV Specific */
  super->s_first_ino = EXT2_GOOD_OLD_FIRST_INO; // First non-reserved inode
  debug_print_v_fs("[ext2]super->s_first_ino = %d\n", super->s_first_ino);
  super->s_inode_size = INODE_SIZE; // Size of inode structure
  debug_print_v_fs("[ext2]super->s_inode_size = %d\n", super->s_inode_size);
  super->s_block_group_nr=1; // only one group
  debug_print_v_fs("[ext2]super->s_block_group_nr = %d\n", super->s_block_group_nr);
  super->s_feature_compat = 0; // none of the features that we described are being used
  debug_print_v_fs("[ext2]super->s_feature_compat = %d\n", super->s_feature_compat);
  super->s_feature_incompat = 0xffff; // Incompatible feature set flags
  debug_print_v_fs("[ext2]super->s_feature_incompat = %d\n", super->s_feature_incompat);
  super->s_uuid[0] = 22; // random value
  if(save_fs_block((char*)super, sizeof(super_block), block_loc)<0){
    PRINT_RED("fs superblock failed to save\n");
    return -1;
  }
  block_group_descriptor* blk_des = (block_group_descriptor*) malloc(sizeof(block_group_descriptor));
  memset(blk_des, 0, sizeof(block_group_descriptor));
  blk_des->bg_block_bitmap = BLOCK_TABLE_BLOCK+1;
  debug_print_v_fs("[ext2]blk_des->bg_block_bitmap = %d\n", blk_des->bg_block_bitmap);
  blk_des->bg_inode_bitmap = blk_des->bg_block_bitmap+               
                            nb_block_data_bitmap;
  debug_print_v_fs("[ext2]blk_des->bg_inode_bitmap = %d\n", blk_des->bg_inode_bitmap);
  blk_des->bg_inode_table = blk_des->bg_inode_bitmap + 1;  
  debug_print_v_fs("[ext2]blk_des->bg_inode_table = %d\n", blk_des->bg_inode_table);
  blk_des->bg_free_blocks_count = nb_blocks_data;
  debug_print_v_fs("[ext2]blk_des->bg_free_blocks_count = %d\n", blk_des->bg_free_blocks_count);
  blk_des->bg_free_inodes_count = number_of_inodes;
  debug_print_v_fs("[ext2]blk_des->bg_free_inodes_count = %d\n", blk_des->bg_free_inodes_count);
  blk_des->bg_used_dirs_count = 0;
  debug_print_v_fs("[ext2]blk_des->bg_used_dirs_count = %d\n", blk_des->bg_used_dirs_count);

  root_file_system->super_block = super;
  root_file_system->desc_table = blk_des;
  if(save_fs_block((char*)blk_des,
     sizeof(block_group_descriptor),
      block_loc+1)<0){
    PRINT_RED("fs block_group_descriptor failed to save\n");
    return -1;
  }
  char empty_block[root_file_system->block_size];
  memset(empty_block,0,root_file_system->block_size);
  for (int blk=blk_des->bg_block_bitmap;
        blk<blk_des->bg_inode_table; blk++){
    if(save_fs_block((char*)empty_block,
      root_file_system->block_size,
        blk)<0){
      PRINT_RED("Failed to save a bit map\n");
      return -1;
    }
  }
  return 0;
}


super_block* get_super_block(){
  if (root_file_system ==0){
    return 0;
  }
  return root_file_system->super_block;
}

block_group_descriptor* get_desc_table(){
  if (root_file_system ==0){
    return 0;
  }
  return root_file_system->desc_table;
}



int config_super_block(){
  print_fs_no_arg("\033[0;32m--conf super block--\033[0;0m\n");
  if (root_file_system != 0&& root_file_system->super_block == 0){
    char* block = 
        disk_read_block(root_file_system->superblock_loc);
    if (block == 0){
      return -1;
    }
    root_file_system->super_block = (super_block*) 
        malloc(sizeof(super_block));
    if (root_file_system->super_block == 0){
      return -1;
    }
    memcpy(root_file_system->super_block, 
      block, sizeof(super_block));
    print_fs_no_arg("\033[0;32m--conf super block end--\033[0m\n");
    return 0;
  }
  return -1;
}

int config_blk_desc_table(){
  print_fs_no_arg("\033[0;32m--conf desc table--\n\033[0;0m");
  if (root_file_system != 0&& root_file_system->desc_table == 0){
    char* block = 
          disk_read_block(root_file_system->desc_table_loc);
    if (block == 0){
      return -1;
    }
    root_file_system->desc_table = (block_group_descriptor*) 
        malloc(sizeof(block_group_descriptor));
    if (root_file_system->super_block == 0){
      return -1;
    }
    memcpy(root_file_system->desc_table, 
        block, sizeof(block_group_descriptor));
    print_fs_no_arg("\033[0;32m--conf desc table end--\033[0;0m\n");
    return 0;
  }
  return -1;
}

int save_super_block(){
  print_fs_no_arg("\033[0;33m--saving super block--\033[0;0m\n");
  if (root_file_system->super_block !=0){
    if(save_fs_block((char*)root_file_system->super_block,
        sizeof(super_block),
        root_file_system->superblock_loc)<0){
      return -1;
    }
    print_fs_no_arg("\033[0;33m--saving super block end--\033[0m\n");
    return 0;
  }
  return -1;
}

int save_blk_desc_table(){
    print_fs_no_arg("\033[0;35m--saving desc table--\033[0;0m\n");
  if (root_file_system->desc_table !=0){
    if(save_fs_block((char*)root_file_system->desc_table,
        sizeof(block_group_descriptor),
        root_file_system->desc_table_loc)<0){
      return -1;
    }
    print_fs_no_arg("\033[0;35m--saving desc table end--\033[0;0m\n");
    return 0;
  }
  return -1;
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

void printblock_group_descriptor(block_group_descriptor* bgd) {
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
