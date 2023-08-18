
#ifndef FS_BRIDGE 
#define FS_BRIDGE

#include "process.h"
#include "../fs/inode.h"

/**
 * @brief Adds a new elements to the link list
 * of of open files and returns an a pointer to 
 * the newly added element which is the head list 
 * @return flip* the newly create open file
 */
open_fd* add_new_element_open_files(process* proc);

typedef enum rem_type{
  REMOVE_ALL = 0, //Closes the inode 
                  //and removes the chain link 
  ONLY_CLOSE_FILE = 1, //Does not remove the link 
                       //of the list 
} rem_type;


/**
 * @brief Remove the structure of the old file
 * from the open files
 * @param fd the file descriptor 
 * that we would like to close 
 * @return int function status 
 */
int remove_fd_list(process*, int fd, rem_type op_type);

/**
 * @brief Get the open file system details
 * @param fd the file descriptor
 * @return flip* the open file sructure, and 0 if 
 * canot be found
 */
flip* get_fs_list_elt(process*, int fd);

/**
 * @brief Create a new open file 
 * in the current process and alloctes the file given
 * as argument to it 
 * @param open_file 
 * @return open_fd* 
 */
open_fd* dup_open_file(process*, flip* open_file, int custom_fd);

/**
 * @brief Checks if the inode number given as argument is currently being used i.e.
 * that inode number is being used by an open file
 * @param inode_number the inode that we are checking
 * @return true the inode is being used
 * @return false the inode is not being used
 */
bool check_if_inode_is_being_used(process*, uint32_t inode_number);

/**
 * @brief Set the diectory of the current process to the 
 * paramter given, the path(new_name) must an absolute path 
 * @param new_name 
 * @param name_size 
 * @param inode 
 * @return int function status 
 */
int set_current_dir(process*,char* new_name, uint32_t name_size, inode_t* inode);

/**
 * @brief Get the open fd object for the file descriptor given
 * @param fd 
 * @return open_fd* 
 */
open_fd* get_open_fd_elt(process*, int fd);

/**
 * @brief Get the inode of the current
 *  directory that we are in.
 * @return inode_t* the directory pointer
 */
inode_t* get_current_dir(process*);

/**
 * @brief Get the name of the current directory
 * @return char* the pointer to the new file name
 */
char* get_current_dir_name(process*);

/**
 * @brief Get the root inode
 * @return inode_t* 
 */
inode_t* get_root_dir(process*);

/**
 * @brief Get the name of the root directory
 * @return char* the pointer to root dir
 */
char* get_root_dir_name(process*);


/**
 * @brief Get the size of the name of the root directory for the current process.
 *
 * The get_root_dir_name_size() function retrieves the size of the name of the root directory
 * associated with the current process.
 *
 * @return The size of the root directory name in bytes.
 *         If the process structure for the current process is not found or invalid, returns 0.
 */
uint32_t get_root_dir_name_size(process*);

/**
 * @brief Get the size of the name of the current directory for the current process.
 *
 * The get_current_dir_name_size() function retrieves the size of the name of the current directory
 * associated with the current process.
 *
 * @return The size of the current directory name in bytes.
 *         If the process structure for the current process is not found or invalid, returns 0.
 */
uint32_t get_current_dir_name_size(process*);

/**
 * @brief Close all of the files that are occupied by the process
 * given as function argument
 * @param proc the process that we wish to close its files
 * @return int function status
 */
int close_all_files(process* proc);

/**
 * @brief Copies the file descriptors from the source process 
 * to the destination process 
 * @param dest_proc the process to which we will copy the files 
 * @param src_proc the process tha contains the file that we wish to copy
 * @return int status
 */
int copy_proc_fds(process* dest_proc, process* src_proc);

#endif