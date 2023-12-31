#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include <stdint.h>
#include "ext2.h"
#include "inode.h"
#ifndef DISK_BUFFER_H 
#define DISK_BUFFER_H

typedef enum write_type{
  WRITE_BACK,//saves the data in the disk cache 
             //without going to the disk directly
  WRITE_THROUGH //Saves data in the disk directly
}write_type;

typedef enum cache_statut_type{
  LOCK,//Means that the current usage locks the block
       //and the cache cannot freely remove it
  UNLOCK //Means that the current use case has finished
         //we can free the cache
} cache_op;

typedef struct cache_element{
  uint8_t disk_res; //indicates if this buffer is reserved 
                    //by the disk
  uint32_t blockNumber; //the disk block being used
  bool dirty; //indicates if the buffer is being used
  uint32_t usage; // how many are using the current disk
  char data[EXT2_BLOCK_SIZE]; // the data of the buffer
  bool locked;//Inidicated if it can be freed or not
  struct cache_element* next_c;
  struct cache_element* before_c;
} c_elt;

extern c_elt* global_cache_buf; 

/**
 * @brief Checks if a block is in the disk buffer
 * @param disk_block_number the number of the disk block
 * @return c_elt* the dick buffer element
 */
c_elt* look_up_c_elt(uint32_t disk_block_number);

/**
 * @brief Checks if a block is in the disk buffer using the 
 * block address
 * @param add address of the data block
 * @return c_elt* the dick buffer element
 */
c_elt* look_up_c_elt_add(char* add);

/**
 * @brief Fetch the block number from the disk
 * @param disk_block_number the block number that we want to fetch
 * @return c_elt* the block descrption that was fetched
 */
c_elt* fetch_block(uint32_t disk_block_number);

/**
 * @brief Writes the data in data pointer into the disk
 * the data is cannot have a size superiour then the BLOCK_SIZE(512 bytes) 
 * @param disk_block_number the disk block number at which we will write the data
 * @param data the data pointer
 * @param data_length the length of the data
 * @param type the type of the write operation write through or write back
 * @return int operation status 
 */
int write_block(uint32_t disk_block_number, char* data,
     size_t data_length, write_type type, cache_op cah_op);

/**
 * @brief Save buffer to disk
 * @param cache_elt the element buffer that we will save into the disk
 * @return int status
 */
int sync_elt(c_elt* cache_elt);

/**
 * @brief calls the sync_elt on every dirty block 
 * @return int status 
 */
int sync();

/**
 * @brief grabs from disk the data located in the disk_block_number
 * @param disk_block_number the relative disk block number
 * @return char* the pointer to the data equal to the file system
 * block size
 */
char* read_block_c(uint32_t disk_block_number, cache_op cah_op);

/**
 * @brief Saves the data equal to the size of block used in the 
 * the file system
 * 
 * @param data 
 * @param size size of data must be between 
 * 0 and the size of the block
 * @param block_number the memory block in which we 
 * will store the data
 * @return int operation status 
 */
int save_fs_block(char* data,
                  uint32_t data_size,
                  uint32_t relative_b_nb,
                  cache_op cah_op
                  );

/**
 * @brief Reads the relative block number from the disk
 * @param relative_b_n the relaive file system block number
 * @return char* the pointer to the data that was read or 0
 * if no data was found
 */
char* disk_read_block(uint32_t relative_b_n, cache_op cah_op);


/**
 * @brief Frees the list of blocks that are in the cache 
 * and places them into the disk
 * @return int function status
 */
int free_cache_list();

/**
 * @brief Prints the details of the current cache buffer
 * @param elt the list of the cache buffer
 */
void printLinkedList(c_elt* elt);


/**
 * @brief Set the cache element to dirty manually 
 * @param disk_block_number the block number
 * @return int 
 */
int set_dirty_block(uint32_t disk_block_number);

/**
 * @brief Check if the cache has reached max capacity 
 * and clears all of the elements located in the cache,
 * this verison is simple, more advanced version will look
 * of the individual usage of elements in the list 
 * and then least used in order. 
 */
void check_if_cache_is_full();


/**
 * @brief Unlocks the cache elements by decreasing it usage
 * if the usage reaches zero that means we can free it
 * @return int status 
 */
int unlock_cache(uint32_t disk_block_number);

/**
 * @brief Does the same thing as unlock_cache but this time
 * it uses the address of the data block to locate the cache block 
 * @param block address of the data block of a cache element
 * @return int status
 */
int unlock_using_address(char* block);

#endif