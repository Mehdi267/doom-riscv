#include "process.h"
#include "../fs/inode.h"


/**
 * @brief Get the inode of the current
 *  directory that we are in.
 * @return inode_t* the directory pointer
 */
inode_t* get_current_dir();

/**
 * @brief Adds a new elements to the link list
 * of of open files and returns an a pointer to 
 * the newly added element which is the head list 
 * @return flip* the newly create open file
 */
flip* add_new_element_open_files();

/**
 * @brief Remove the structure of the old file
 * from the open files
 * @param fd the file descriptor 
 * that we would like to close 
 * @return int function status 
 */
int remove_fd_list(int fd);


/**
 * @brief Get the open file system details
 * @param fd the file descriptor
 * @return flip* the open file sructure, and 0 if 
 * canot be found
 */
flip* get_fs_list_elt(int fd);
