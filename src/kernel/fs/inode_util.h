#include "inode.h"
#include "disk_buffer.h"
#include <stddef.h>
#include <stdint.h>

#ifndef INODE_UTIL_H
#define INODE_UTIL_H


/**
 * @brief Print details of the inode cache.
 * 
 * @param list Pointer to the first inode_elt in the cache.
 */
void print_cache_details(inode_elt* list);

/**
 * @brief Free the inode cache.
 * 
 * @param list Pointer to the first inode_elt in the cache.
 * @return Returns 0 on success, -1 on failure.
 */
int free_inode_list(inode_elt* list);

/**
 * @brief Add an inode to the cache.
 * 
 * @param address Pointer to the inode address.
 * @param inode_id Inode identifier.
 * @return Returns 0 on success, -1 on failure.
 */
int add_inode_list(inode_t* address, uint32_t inode_id);

/**
 * @brief Remove an inode from the cache.
 * 
 * @param inode_id Inode identifier.
 * @param inode_address Pointer to the inode address.
 * @return Returns 0 on success, -1 on failure.
 */
int remove_inode_list(uint32_t inode_id, inode_t* inode_address);

/**
 * @brief Print basic information about a directory entry.
 * 
 * @param entry Pointer to the directory entry.
 */
void print_dir_entry_basic(dir_entry_basic* entry);

/**
 * @brief Print detailed information about a directory entry.
 * 
 * @param entry Pointer to the directory entry.
 */
void print_dir_entry(dir_entry_basic* entry);

/**
 * @brief Print detailed information about a directory entry object.
 * 
 * @param entry Pointer to the directory entry object.
 */
void print_dir_entry_obj(dir_entry* entry);

/**
 * @brief Returns the inode located in the path directed by the back steps
 * if the value backward_steps is equal to zeo returns the final file
 * @param path the path that we would like to extract an inode from
 * @param backward_steps the number of backward steps taht we shoud limit
 * ourselves to.
 * @return inode_t* the inode that we collected 
 */
inode_t* walk_and_get(const char* path, uint32_t backward_steps);

#endif /* INODE_H */


