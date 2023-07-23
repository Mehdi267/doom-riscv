#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include "string.h"
#include "fs_api.h"
#include "../process/process.h"
#include "../process/helperfunc.h"
#include "inode.h"
#include "../process/fs_bridge.h"


// Messages from users
int open(const char *file_name, int flags, mode_t mode){
  debug_print_fsapi("[FSAPI]Trying to open a file with the name %s flags %x\n",
         file_name, flags);
  if (file_name == 0){
    return -1;
  }
  if (flags > 0x2ff){
    printf("Bad flags");
    return -1;
  }
  path_fs* path_data = extract_files(file_name);
  if (path_data == 0){
    return -1;
  }
  inode_t* file_inode; 
  //We create the file 
  if ((flags & O_CREAT) != 0){
    inode_t* dir_inode = 0;
    if (is_absolute_directory(file_name)){
       dir_inode = get_inode(EXT2_GOOD_OLD_FIRST_INO);
      for (uint32_t file_iter = 0; file_iter < path_data->nb_files - 1; file_iter++) {
        dir_inode = get_inode(look_for_inode_dir(dir_inode, path_data->files[file_iter], 
            strlen(path_data->files[file_iter]))); 
      }
      if (dir_inode == 0){
        return -1;
      }
      file_inode = alloc_inode();   
      if (file_inode == 0){
        return -1;
      }
    }
    else{
      dir_inode = get_current_dir();
      char point_dir[1] = ".";
      if (!(memcmp(path_data->files[0], point_dir, 1))){
        for (uint32_t file_iter = 0; file_iter < path_data->nb_files - 1; file_iter++) {
          dir_inode = get_inode(look_for_inode_dir(dir_inode, path_data->files[file_iter], 
              strlen(path_data->files[file_iter]))); 
        }
      }
      if (dir_inode == 0){
        return -1;
      }
      file_inode = alloc_inode(); 
        file_inode->i_mode = EXT2_S_IFREG;
    }
    if (dir_inode && add_inode_directory(get_current_dir(), 
          get_inode_number(file_inode), 
          get_inode_number(dir_inode),
          path_data->files[path_data->nb_files -1],
          strlen(path_data->files[path_data->nb_files -1]))){
      return -1;
    }
  } 
  //File is already present
  else {
    if (is_absolute_directory(file_name)){
        inode_t* dir_inode = 
              get_inode(EXT2_GOOD_OLD_FIRST_INO);
        for (uint32_t file_iter = 0; file_iter < path_data->nb_files - 1; file_iter++) {
          dir_inode = get_inode(look_for_inode_dir(dir_inode, 
            path_data->files[file_iter], 
            strlen(path_data->files[file_iter]))); 
        }
        file_inode = get_inode(look_for_inode_dir(dir_inode, 
          path_data->files[path_data->nb_files-1], 
          strlen(path_data->files[path_data->nb_files-1])));  
    } else{
      file_inode = get_inode(look_for_inode_dir(get_current_dir(), 
        path_data->files[path_data->nb_files-1], 
        strlen(path_data->files[path_data->nb_files-1])));  
    }
  }
  if (file_inode == 0){
    return -1;
  }
  flip* new_file = add_new_element_open_files();
  if (new_file == 0){
    return -1;
  }
  //Permissions
  new_file->f_inode = file_inode;
  if ((flags & O_RDONLY) != 0){
    new_file->can_read = true;
    new_file->can_write = false;
  }
  else if ((flags & O_WRONLY) != 0){
    new_file->can_read = false;
    new_file->can_write = true;
  }
  else{
    new_file->can_read = true;
    new_file->can_write = true;
  }
  if ((flags & O_APPEND) != 0){
    new_file->append_on = true;
    new_file->position = file_inode->i_size;
  }
  if ((flags & O_SYNC) != 0){
    new_file->sync_directly = true;
  }
  new_file->inode_number = 
    get_inode_number(file_inode);
  debug_print_fsapi("[FSAPI]Filed opened with success %s flags %x\n",
        file_name, flags);
  free_path_fs(path_data);
  return new_file->fd;
}

int close(int file_descriptor){
  return remove_fd_list(file_descriptor);
}

ssize_t write(int file_descriptor, const void *buffer, size_t count){
 flip* fs_elt = get_fs_list_elt(file_descriptor); 
  if (fs_elt == 0){
    return -1;
  }
}

// int access(const char *file_name, int mode);
// int chdir(const char *new_directory);
// int chmod(const char *file_name, mode_t new_mode);
// int chroot(const char *new_root_directory);
// int create(const char *file_name, mode_t mode);
// int dup(int file_descriptor);
// int dup2(int file_descriptor, int new_file_descriptor);
// int fcntl(int file_descriptor, int function_code, int arg);
// int fstat(const char *file_name, struct stat *buffer);
// int ioctl(int file_descriptor, int function_code, int arg);
// off_t lseek(int file_descriptor, off_t offset, int whence);
// int mkdir(const char *dir_name, mode_t mode);
// //will try to implement but the the current design choices make this very hard to implement
// int mount(const char *special_file, const char *mount_point, int ro_flag);
// int pipe(int file_descriptors[2]);
// ssize_t read(int file_descriptor, void *buffer, size_t count);
// int rename(const char *old_name, const char *new_name);
// int rmdir(const char *dir_name);
// int stat(const char *file_name, struct stat *status_buffer);
// mode_t umask(mode_t mask);
// int umount(const char *special_file);
// int unlink(const char *file_name);
// int utime(const char *file_name, const struct utimbuf *times);

// // Messages from PM
// int exec(pid_t pid);
// void exit(pid_t pid);
// pid_t fork(pid_t parent_pid);
// pid_t setsid(pid_t pid);

