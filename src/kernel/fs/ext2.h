#include "stdint.h"
#include "super_block.h"

#ifndef EXT2_H 
#define EXT2_H

#define EXT2_BLOCK_SIZE 2048
#define INODE_SIZE 128
#define RESERVED_BLOCKS 3 //boot record block
                          //super block
                          //Block Group Descriptor Table
#define BOOT_RECORD_LOC 0
#define SUPER_BLOCK_LOC 1
#define BLOCK_TABLE_BLOCK 2
#define EXT2_SUPER_MAGIC 0xEF53
//Disk state
#define EXT2_VALID_FS 1 // Unmounted cleanly
#define EXT2_ERROR_FS 2 // Errors detected
//disk Errors
#define EXT2_ERRORS_CONTINUE 1 // continue as if nothing happened
#define EXT2_ERRORS_RO 2 // remount read-only
#define EXT2_ERRORS_PANIC 3 // cause a kernel panic
//Creator os super block 
#define EXT2_OS_LINUX 0 // Linux
#define EXT2_OS_HURD 1 // GNU HURD
#define EXT2_OS_MASIX 2 // MASIX
#define EXT2_OS_FREEBSD 3 // FreeBSD
#define EXT2_OS_LITES 4 // Lites
//Revision level
#define EXT2_GOOD_OLD_REV 0 //Revision 0
#define EXT2_DYNAMIC_REV 1 //Revision 1 with variable inode sizes, extended attributes, etc.
//First Inode
//this is the value of the first inode that we can use
//The first 10 inodes are reserved for special use
#define EXT2_GOOD_OLD_FIRST_INO 11
/**
 * @brief Saves the master boot record into the disk
 * @param boot_loc the location as to where we can save the boot record
 * @return int informs of the status of the operation
 */
int save_boot_record(uint32_t boot_loc);

/**
 * @brief Configuration of the super block for the
 * ext2 file system
 * @param block_loc the location as to where the super block will be saved 
 * @param disk_size the total size of the disk
 * @return int 
 */
int superblock_conf(uint32_t block_loc, uint32_t disk_size);

/**
 * @brief prints the super block given as funtion argument
 * @param super the super block
 */
void print_super_block(super_block* super);

/**
 * @brief Prints BlockGroupDescriptor details 
 * @param bgd the BlockGroupDescriptor data
 */
void printBlockGroupDescriptor(BlockGroupDescriptor* bgd);
#endif