#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include "string.h"
#include "fs_api.h"
#include "ext2.h"
#include "fs.h"
#include "../process/process.h"
#include "../process/helperfunc.h"
#include "inode.h"
#include "inode_util.h"
#include "../process/fs_bridge.h"
#include "assert.h"

// Messages from users
int open(const char *file_name, int flags){
  debug_print_fsapi("\033[0;35m[FSAPI]Trying to open a file with the name %s flags %x\n\033[0;0m",
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
    inode_t* dir_inode = walk_and_get(file_name, 1);
    if (dir_inode == 0){
      free_path_fs(path_data);
      return -1;
    }
    file_inode = alloc_inode(); 
    if (file_inode != 0){
      file_inode->i_mode = EXT2_S_IFREG;
      if (dir_inode && add_inode_directory(dir_inode, 
            get_inode_number(file_inode), 
            get_inode_number(dir_inode),
            path_data->files[path_data->nb_files -1],
            strlen(path_data->files[path_data->nb_files -1]))){
        free_path_fs(path_data);
        return -1;
      }
    }
  } 
  //File is already present
  else {
    file_inode = walk_and_get(file_name, 0);
  }
  if (file_inode == 0){
    free_path_fs(path_data);
    return -1;
  }
  flip* new_file = add_new_element_open_files();
  if (new_file == 0){
    free_path_fs(path_data);
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
  debug_print_fsapi("\033[0;34m[FSAPI]{open}Filed opened with success %s flags %x\033[0;0m\n",
        file_name, flags);
  free_path_fs(path_data);
  return new_file->fd;
}

int close(int file_descriptor){
  debug_print_fsapi("\033[0;35m[FSAPI]Close file was called on fd = %d\n\033[0;0m",
       file_descriptor);
  return remove_fd_list(file_descriptor);
}

ssize_t write(int file_descriptor, 
              const void *buffer, size_t count){
  debug_print_fsapi("\033[0;34m[FSAPI] write syscall was called on fd %d, , write size = %ld\033[0;0m\n",
       file_descriptor, count);
  flip* fs_elt = get_fs_list_elt(file_descriptor); 
  if (fs_elt == 0 || count == 0){
    return 0;
  }
  uint32_t written_data = 0;
  int actual_blocks = get_actual_blocks(fs_elt->f_inode);
  if (actual_blocks < 0){
    return -1;
  }
  debug_print_fsapi("\033[0;34m[FSAPI] Actual blocks on fd %d are %d\033[0;0m\n",
       file_descriptor, actual_blocks);
  while (written_data<count){
    if (actual_blocks<(fs_elt->position/root_file_system->block_size + 1)){
      if (add_data_block_inode(fs_elt->f_inode)<0){
        PRINT_RED("Could not allocate new space, either file is full or disk is full");
        return written_data;
      } else{
        actual_blocks++;
      }
    }
    char* block_data = get_inode_relative_block(fs_elt->f_inode,
        fs_elt->position/root_file_system->block_size, WRITE_OP);
    if (block_data == 0){
      debug_print_fsapi("\033[0;31m[FSAPI]Relative block does want not found written data =  %d\033[0;0m\n",
         written_data);
      return written_data;
    }
    uint32_t write_space = root_file_system->block_size-
          (fs_elt->position%root_file_system->block_size);
    if (write_space > count - written_data){
      write_space = count - written_data;
    }
    memcpy(block_data+fs_elt->position%root_file_system->block_size,
      buffer + written_data, write_space);
    written_data += write_space;
    fs_elt->position += write_space;
    fs_elt->f_inode->i_size += write_space;
  }
  sync();
  debug_print_fsapi("\033[0;34m[FSAPI]Write ran normal and data written is equal to %d\033[0;0m\n",
       written_data); 
  assert(written_data <= count);
  return written_data;
}

ssize_t read(int file_descriptor, void *buffer, size_t count){
  if (file_descriptor<0){
    return -1;
  }
  debug_print_fsapi("\033[0;34m[FSAPI] read syscall was called on fd %d, read size = %ld\033[0;0m\n",
         file_descriptor, count);
  flip* fs_elt = get_fs_list_elt(file_descriptor); 
  if (fs_elt == 0 || count == 0){
    return 0;
  }
  uint32_t read_data = 0;
  
  int actual_blocks = get_actual_blocks(fs_elt->f_inode);
  if (actual_blocks < 0){
    return -1;
  }
  debug_print_fsapi("\033[0;34m[FSAPI] Actual blocks on fd %d are %d\033[0;0m\n",
      file_descriptor, actual_blocks);
  while (read_data<count){
    if (actual_blocks<
        (fs_elt->position/root_file_system->block_size + 1)){
      return read_data;
    }
    char* block_data = get_inode_relative_block(fs_elt->f_inode,
        fs_elt->position/root_file_system->block_size, READ_OP);
    if (block_data == 0){
      return read_data;
    }
    uint32_t read_iter = root_file_system->block_size-
          (fs_elt->position%root_file_system->block_size);
    if (read_iter > count - read_data){
      read_iter = count - read_data;
    }
    memcpy( buffer + read_data, 
            block_data+fs_elt->position%root_file_system->block_size,
            read_iter);
    read_data += read_iter;
    fs_elt->position += read_iter;
  }
  debug_print_fsapi("\033[0;34m[FSAPI]Read ran normal and data read is equal to %d\033[0;0m\n",
      read_data); 
  return read_data;
}

off_t lseek(int file_descriptor, off_t offset, int whence){
  if (whence <= 2 && whence >= 0){
    PRINT_RED("L SEEK OPERATION FAILED");
    return -1;
  }
  flip* fs_elt = get_fs_list_elt(file_descriptor); 
  if (fs_elt == 0){
    return 0;
  }
  if (whence == SEEK_SET) {
    fs_elt->position = offset;
  }
  else if (whence == SEEK_CUR){
    fs_elt->position += offset;
  }
  else if (whence == SEEK_END) {
    fs_elt->position = fs_elt->f_inode->i_size + offset;
  }
  if (fs_elt->position > fs_elt->f_inode->i_size){
    fs_elt->position = fs_elt->f_inode->i_size;
  } else if (fs_elt->position < 0){
    fs_elt->position = 0;
  }
  return fs_elt->position;
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
// int mkdir(const char *dir_name, mode_t mode);
// //will try to implement but the the current design choices make this very hard to implement
// int mount(const char *special_file, const char *mount_point, int ro_flag);
// int pipe(int file_descriptors[2]);
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


//Custom api
void print_dir_elements(const char* path){
  debug_print_fsapi("\033[0;33m[FSAPI]print_dir_elements path =  %s\n\033[0;0m", path);
  inode_t* dir =  walk_and_get(path, 0);
  if (dir != 0){
    print_dir_list(dir, true);
    return;
  }
}
