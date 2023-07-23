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

