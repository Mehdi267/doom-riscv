#include "stdint.h"
#include <stdint.h>

#ifndef FS_H 
#define FS_H


typedef struct file_system_struct {
  uint8_t fs_type;
  uint32_t superblock_loc;
  uint32_t disk_size;
  uint32_t block_size;
  uint32_t fs_size;
  uint8_t partition;
} file_system_t;

/**
 * @brief The global file system variable
 */
extern file_system_t* root_file_system;

/**
 * @brief Set the up file system, 
 * 
 * @return int 
 */
extern int set_up_file_system();

//################
//partition system calls
//##################
/**
 * @brief Prints information about the disk 
 * and current partitions that are present
 */
void print_partition_status();

/**
 * @brief Creates a partition in the Master Boot Record (MBR).
 *
 * This function creates a new partition in the MBR with the specified start
 * and size, and assigns the given partition type to it.
 *
 * @param start The starting Logical Block Address (LBA) of the partition.
 * @param size The size of the partition in LBAs.
 * @param partition_type The type of the partition.
 * @return 0 if the partition is created successfully, -1 otherwise.
 */
int create_partition(uint32_t start, uint32_t size, uint8_t partition_type);

/**
 * @brief Deletes a partition from the Master Boot Record (MBR).
 *
 * This function deletes the partition with the specified partition number
 * from the MBR.
 *
 * @param partition_number The number of the partition to delete.
 * @return 0 if the partition is deleted successfully, -1 otherwise.
 */
int delete_partition(uint8_t partition_number);

/**
 * @brief Set up the MBR by allocating memory for the global MBR struct and setting the signature.
 */
int set_up_mbr();

/**
 * @brief Saves the data equal to the size of block used in the 
 * the file system
 * 
 * @param data 
 * @param size size of data must be between 
 * 0 and the size of the block
 * @param block_number the memory block in which we 
 * will store the data
 * @return int 
 */
int save_fs_block(char* data,
                  uint32_t data_size,
                  uint32_t block_number
                  );

/**
 * @brief configures ext2 file system
 * @param  partition the partition at which we will place the file system
 * @return 0 or -1 to indicate operation status
 */
int configure_ext2_file_system(uint8_t partition);

/**
 * @brief configures the root fs global structure 
 * so that it can deal with the different types 
 * @param fs_type type of the root file system given by the numebr of the 
 * associated to the type for example ext 131
 * @param superblock_loc the location of the superblock within the system it self
 * @param block_size the size of the file ystem block
 * @param partition the partition number where the 
 * file system is being stored
 * @return int 
 */
int configure_root_file_system(uint8_t fs_type,uint32_t superblock_loc,uint32_t block_size,uint8_t partition);

#endif