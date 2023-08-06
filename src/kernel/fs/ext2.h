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
#define NUMBER_OF_RESERVED_INODES 12

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
 * @brief Allocates in the inode bit map the reserved inodes
 * ids
 * @return int function status 
 */
int configure_reserved_inodes();

/**
 * @brief Allocates in the data bit map the reserved data blocks
 * which is only the first data block now(it is reserved 
 * because i might need it later)
 * @return int function status 
 */
int configure_reserved_data();

/**
 * @brief configures the root inode in ext2 file system
 * @return int function status
 */
int configure_root_inode();

/**
 * @brief prints the super block given as funtion argument
 * @param super the super block
 */
void print_super_block(super_block* super);

/**
 * @brief Prints block_group_descriptor details 
 * @param bgd the block_group_descriptor data
 */
void printblock_group_descriptor(block_group_descriptor* bgd);

/**
 * @brief Get the super block.
 *
 * This function returns a pointer to the super block.
 *
 * @return A pointer to the super block.
 */
super_block* get_super_block();

/**
 * @brief Get the block group descriptor table.
 *
 * This function returns a pointer to the block group descriptor table.
 *
 * @return A pointer to the block group descriptor table.
 */
block_group_descriptor* get_desc_table();

/**
 * @brief Save the super block.
 *
 * This function saves the super block to the disk.
 *
 * @return 0 if successful, -1 if an error occurred.
 */
int save_super_block();

/**
 * @brief Save the block group descriptor table.
 *
 * This function saves the block group descriptor table to the disk.
 *
 * @return 0 if successful, -1 if an error occurred.
 */
int save_blk_desc_table();

/**
 * @brief Loads the super block from disk
 * and saves it the root_file_system global variable
 * @return int funtion status 
 */
int config_super_block();

/**
 * @brief Loads the block group descriptor table from disk
 * and saves it the root_file_system global variable
 * @return int funtion status 
 */
int config_blk_desc_table();

/**
 * @brief Adds basic directories to the file system
 * @return int status
 */
int create_base_dirs();

/**
 * @brief Create a basic files like the terminal device driver
 * and many other that will added later on
 * @return int status
 */
int create_base_files();

#endif /* INODE_H */
