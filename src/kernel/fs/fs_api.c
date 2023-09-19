#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include "string.h"
#include "fs_api.h"
#include "ext2.h"
#include "fs.h"
#include "process/process.h"
#include "process/helperfunc.h"
#include "inode.h"
#include "pipe.h"
#include "inode_util.h"
#include "process/fs_bridge.h"
#include "assert.h"

// Messages from users
int open(process* proc, const char *file_name, int flags, mode_t mode){
  debug_print_fsapi("\033[0;35m[FSAPI]Trying to open a file with the name %s flags %x\n\033[0;0m",
         file_name, flags);
  if (file_name == 0){
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
    if (dir_inode == 0 || dir_inode->i_mode != EXT2_S_IFDIR){
      free_path_fs(path_data);
      return -1;
    }
    if (look_for_inode_dir(dir_inode, path_data->files[path_data->nb_files -1],
            strlen(path_data->files[path_data->nb_files -1])) != 0){
      if ((flags & O_EXCL) != 0){
        printf("File is still present !!!\n");
        file_inode = 0;
      }else{
        print_fsapi_no_arg("File already exists, deleting its data\n");
        file_inode = walk_and_get(file_name, 0);
        if (file_inode->i_mode == EXT2_S_IFREG){
          if (free_inode_data(file_inode)<0){
            return -1;
          }
        }
      }
    }
    else{
      file_inode = alloc_inode(); 
      if (file_inode != 0){
        file_inode->i_mode = EXT2_S_IFREG;
        file_inode->i_links_count = 1;
        if (add_inode_directory(dir_inode, 
              get_inode_number(file_inode), 
              EXT2_FT_REG_FILE,
              path_data->files[path_data->nb_files -1],
              strlen(path_data->files[path_data->nb_files -1]))<0){
          free_inode(file_inode, get_inode_number(file_inode));
          free_path_fs(path_data);
          return -1;
        }
      }
    }
  } 
  //File is already present
  else {
    //We look for the file
    file_inode = walk_and_get(file_name, 0);
  }
  if (file_inode == 0){
    free_path_fs(path_data);
    return -1;
  }
  if ((flags & O_DIRECTORY) != 0){
    if (file_inode->i_mode != EXT2_S_IFDIR){
      free_path_fs(path_data);
      return -1;
    }
  } 
  open_fd* new_file = add_new_element_open_files(proc);
  if (new_file == 0){
    free_path_fs(path_data);
    return -1;
  }
  //Permissions
  new_file->file_info->f_inode = file_inode;
  if (file_inode->i_mode == EXT2_S_IFREG || 
      file_inode->i_mode == EXT2_S_IFDIR){
    new_file->file_info->type = FS_FT_INODE_FILE; 
  }
  else if (file_inode->i_mode == EXT2_S_IFCHR){
    new_file->file_info->type = FS_FT_CHRDEV; 
  }
  if (file_inode->i_mode == EXT2_S_IFREG && 
      (flags && O_TRUNC) != 0){
    print_fsapi_no_arg("Deleting file data\n");
    if (free_inode_data(file_inode)<0){
      return -1;
    }
  }
  if ((flags & 3) == O_RDONLY){
    new_file->file_info->can_read = true;
    new_file->file_info->can_write = false;
  }
  else if ((flags & 3) == O_WRONLY){
    new_file->file_info->can_read = false;
    new_file->file_info->can_write = true;
  }
  else{
    new_file->file_info->can_read = true;
    new_file->file_info->can_write = true;
  }
  if ((flags & O_APPEND) != 0){
    new_file->file_info->append_on = true;
    new_file->file_info->position = file_inode->i_size;
  }
  if ((flags & O_SYNC) != 0){
    new_file->file_info->sync_directly = true;
  }
  new_file->file_info->inode_number = 
    get_inode_number(file_inode);
  debug_print_fsapi("\033[0;34m[FSAPI]{open}File opened with success %s fd : %d flags %x\033[0;0m\n",
        file_name, new_file->fd, flags);
  free_path_fs(path_data);
  return new_file->fd;
}

int close(int file_descriptor){
  debug_print_fsapi("\033[0;35m[FSAPI]Close file was called on fd = %d\n\033[0;0m",
       file_descriptor);
  sync_all();
  return remove_fd_list(NULL, file_descriptor, REMOVE_ALL);
}

ssize_t write(int file_descriptor, 
              const void *buffer, size_t count){
  debug_print_fsapi("\033[0;34m[FSAPI] write syscall was called on fd %d, write size = %ld\033[0;0m\n",
       file_descriptor, count);
  flip* fs_elt = get_fs_list_elt(NULL, file_descriptor); 
  if (fs_elt == 0 || count == 0 || fs_elt->can_write == false){
    return -1;
  }
  if (fs_elt->type == FS_FT_CHRDEV){
    return dev_op[(fs_elt->f_inode->i_osd1 & 0xffff0000) >> 16].write((uint64_t)buffer, count); 
  } 
  else if (fs_elt->type == FS_FT_PIPE){
    return write_pipe(fs_elt->f_pipe, buffer, count); 
  } else if(fs_elt->type == FS_FT_INODE_FILE) {
    uint32_t written_data = 0;
    int actual_blocks = get_actual_blocks(fs_elt->f_inode);
    if (actual_blocks < 0){
      PRINT_RED("Failed while trying to get file size");
      return -1;
    }
    debug_print_fsapi("\033[0;34m[FSAPI] Actual blocks on fd %d are %d\033[0;0m\n",
        file_descriptor, actual_blocks);
    while (written_data<count){
      if (actual_blocks<(fs_elt->position/root_file_system->block_size + 1)){
        if (add_data_block_inode(fs_elt->f_inode)<0){
          PRINT_RED("Could not allocate new space, either file is full or disk is full\n");
          return written_data;
        } else{
          put_inode(fs_elt->f_inode, 0, SAVE_INODE);
          actual_blocks++;
        }
      }
      char* block_data = get_inode_relative_block(fs_elt->f_inode,
          fs_elt->position/root_file_system->block_size, WRITE_OP);
      if (block_data == 0){
        PRINT_RED("Block is null\n");
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
      unlock_using_address(block_data);
    }
    if (fs_elt->sync_directly){
      sync_all();
    }
    debug_print_fsapi("\033[0;34m[FSAPI]Write ran normal and data written is equal to %d\033[0;0m\n",
        written_data); 
    assert(written_data <= count);
    return written_data;
  }
  return -1;
}

ssize_t read(int file_descriptor, void *buffer, size_t count){
  if (file_descriptor<0){
    return -1;
  }
  debug_print_fsapi("\033[0;34m[FSAPI] read syscall was called on fd %d, read size = %ld\033[0;0m\n",
         file_descriptor, count);
  flip* fs_elt = get_fs_list_elt(NULL, file_descriptor);
  if (fs_elt == 0) {
      printf("File system element is NULL.\n");
      return -1;
  }
  if (count == 0) {
      printf("Count is zero.\n");
      return -1;
  }
  if (fs_elt->can_read == false) {
      printf("Cannot read from the file system element.\n");
      return -1;
  }
  if (fs_elt->type == FS_FT_CHRDEV){
    return dev_op[(fs_elt->f_inode->i_osd1 & 0xffff0000) >> 16].read((uint64_t)buffer, count); 
  } else if (fs_elt->type == FS_FT_PIPE){
    return read_pipe(fs_elt->f_pipe, buffer, count); 
  } else if(fs_elt->type == FS_FT_INODE_FILE) {
    uint32_t read_data = 0;
    int actual_blocks = get_actual_blocks(fs_elt->f_inode);
    if (actual_blocks < 0){
      return -1;
    }
    debug_print_fsapi("\033[0;34m[FSAPI] Actual blocks on fd %d are %d\033[0;0m\n",
        file_descriptor, actual_blocks);
    while (read_data<count &&
          fs_elt->position<fs_elt->f_inode->i_size){
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
      unlock_using_address(block_data);
    }
    debug_print_fsapi("\033[0;34m[FSAPI]Read ran normal and data read is equal to %d\033[0;0m\n",
        read_data); 
    return read_data;
  } else{
    //Bad type
    printf("fs_elt->type %d pos %ld inode num %d\n", fs_elt->type, fs_elt->position, fs_elt->inode_number);
  }
  return -1;
}

off_t lseek(int file_descriptor, off_t offset, int whence){
  if (!(whence <= 2 && whence >= 0)){
    PRINT_RED("L SEEK OPERATION FAILED");
    return -1;
  }
  flip* fs_elt = get_fs_list_elt(NULL, file_descriptor); 
  if (fs_elt == 0 || fs_elt->type != FS_FT_INODE_FILE){
    return -1;
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

int unlink(const char *file_name){
  debug_print_fsapi("\033[0;35m[FSAPI]Trying to unlink a file with the name %s\n\033[0;0m",file_name);
  if (file_name == 0){
    return -1;
  }
  path_fs* path_data = extract_files(file_name);
  if (path_data == 0){
    return -1;
  }
  inode_t* file_inode; 
  //We create the file 
  inode_t* dir_inode = walk_and_get(file_name, 1);
  if (dir_inode == 0 || dir_inode->i_mode != EXT2_S_IFDIR){
    free_path_fs(path_data);
    return -1;
  }
  if (look_for_inode_dir(dir_inode, path_data->files[path_data->nb_files -1],
          strlen(path_data->files[path_data->nb_files -1])) != 0){
      print_fsapi_no_arg("File found, unlinking it\n");
      file_inode = walk_and_get(file_name, 0);
      if (file_inode == 0){
        free_path_fs(path_data);
        return -1;
      }
      bool used = check_if_inode_is_being_used(NULL, get_inode_number(file_inode));
      if (file_inode->i_mode == EXT2_S_IFREG && 
           used == false){
        if (remove_inode_dir(dir_inode, path_data->files[path_data->nb_files -1],
            strlen(path_data->files[path_data->nb_files -1]))<0){
          free_path_fs(path_data);
          return -1;
        }
        if (free_inode(file_inode, get_inode_number(file_inode))<0){
          free_path_fs(path_data);
          return -1;
        }
        sync_all();
        printf("DELETED FILE\n");
      }
      else if (used == true){
        printf("File is being used, cannot delete\n");
        free_path_fs(path_data);
        return -1;
      }
  }else{
    free_path_fs(path_data);
    return -1;
  }
  free_path_fs(path_data);
  return 0;
}

int sys_link(const char *oldpath, const char *newpath){
  if (oldpath == 0 || newpath == 0){
    return -1;
  }
  debug_print_fsapi("\033[0;35m[FSAPI]Trying to create a new hard link old path %s new path %s\n\033[0;0m",
      oldpath, newpath); 
  inode_t* main_file = walk_and_get(oldpath, 0);
  if (main_file == 0 || main_file->i_mode != EXT2_S_IFREG){
    return -1;
  }
  inode_t* dir_inode = walk_and_get(newpath, 1);
  if (dir_inode == 0 || dir_inode->i_mode != EXT2_S_IFDIR){
    return -1;
  }
  path_fs* path_data = extract_files(newpath);
  if (path_data == 0){
    return -1;
  }
  if (look_for_inode_dir(dir_inode, path_data->files[path_data->nb_files -1],
          strlen(path_data->files[path_data->nb_files -1])) != 0){
    PRINT_RED("Cannot create link, file already exists, \n");
    free_path_fs(path_data);
    return -1;
  }
  main_file->i_links_count++;
  if (add_inode_directory(dir_inode, 
        get_inode_number(main_file), 
        EXT2_FT_REG_FILE,
        path_data->files[path_data->nb_files -1],
        strlen(path_data->files[path_data->nb_files -1]))<0){
    free_path_fs(path_data);
    return -1;
  }
  free_path_fs(path_data);
  debug_print_fsapi("\033[0;34m[FSAPI]{link}Link created with success %s -> %s\033[0;0m\n",
        oldpath, newpath);
  printf("\033[0;34m[FSAPI]{link}Link created with success %s -> %s\033[0;0m\n",
        oldpath, newpath);
  return 0;
}

static int conf_stat_buf(struct stat *buf, inode_t* file_inode){
  if (root_file_system == 0 && file_inode == 0){
    return -1;
  }
  buf->st_dev = 1; //We are using one device currently         
  buf->st_ino = get_inode_number(file_inode);         
  buf->st_mode = 777; //Random value might add modes later        
  buf->st_nlink = file_inode->i_links_count;       
  buf->st_uid = 0;         
  buf->st_gid = 0;         
  buf->st_rdev = EXT2_S_IFBLK;        
  buf->st_size = file_inode->i_size;  
  buf->st_blksize = root_file_system->block_size;     
  buf->st_blocks = file_inode->i_blocks*(
          root_file_system->block_size/512);      
  buf->st_atime = 0;       
  buf->st_mtime = 0;       
  buf->st_ctime = 0;
  return 0;
}

int stat(const char *pathname, struct stat *buf){
  if (pathname == 0 || buf == 0){
    return -1;
  }
  inode_t* file_inode = walk_and_get(pathname, 0);  
  return conf_stat_buf(buf, file_inode);
}

int fstat(unsigned int fd, struct stat *buf){
  if (fd == 0 || buf == 0){
    return -1;
  }
  flip* fs_elt = get_fs_list_elt(NULL, fd); 
  if (fs_elt == 0){
    return -1;
  }
  return conf_stat_buf(buf, fs_elt->f_inode);
}

int dup(process* proc, int file_descriptor){
  if (file_descriptor<0){
    return -1;
  }
  open_fd* op_file = 
      dup_open_file(NULL, get_fs_list_elt(proc, file_descriptor), 0);
  if(op_file!=0){
    return op_file->fd;
  }else{
    return -1;
  }
}

int dup2(process* proc, int file_descriptor, int new_file_descriptor){
  if (file_descriptor<0 || new_file_descriptor<0 ||
       file_descriptor>=MAX_FS || new_file_descriptor>=MAX_FS){
    return -1;
  }
  flip* file_dup = get_fs_list_elt(proc, file_descriptor);
  if (file_dup == 0){
    //File is not open
    return -1;
  }
  //We check if it is already open for ths process
  open_fd* new_file = get_open_fd_elt(proc, new_file_descriptor);
  if (new_file == 0){
    new_file = dup_open_file(proc, file_dup, new_file_descriptor);
    if (new_file == 0){
      return -1;
    }
    file_dup->usage_counter++;
    return new_file_descriptor;
  } else{
    if (remove_fd_list(proc, file_descriptor, ONLY_CLOSE_FILE)<0){
      return -1;
    }
    file_dup->usage_counter++;
    new_file->file_info = file_dup;
    return new_file_descriptor;
  }
  return -1;
}

int rename(const char *old_name, const char *new_name){
  return 0; 
}

int sys_pipe(int file_descriptors[2]){
  if (file_descriptors == 0){
    return -1;
  }
  open_fd* file0 = add_new_element_open_files(NULL);
  if (file0 == 0){goto fail;}
  open_fd* file1 = add_new_element_open_files(NULL);
  if (file1 == 0){goto fail;}
  pipe* pipe = create_pipe();
  if (pipe == 0){goto fail;}
  file0->file_info->type = FS_FT_PIPE;
  file1->file_info->type = FS_FT_PIPE;
  file0->file_info->can_read = true;
  file1->file_info->can_write = true;
  file0->file_info->f_pipe = pipe;
  file1->file_info->f_pipe = pipe;
  file_descriptors[0] = file0->fd;
  file_descriptors[1] = file1->fd;
  return 0;
  fail:
    if (file0){free(file0);};
    if (file1){free(file1);};
    if (pipe){close_pipe(pipe, CLOSE_ALL);};
    return -1;
}

//Custom api
void print_dir_elements(const char* path){
  debug_print_fsapi("\033[0;33m[FSAPI]print_dir_elements path = %s\n\033[0;0m", path);
  inode_t* dir = walk_and_get(path, 0);
  if (dir != 0){
    debug_print_fsapi("\033[0;33m[FSAPI] inode found dir with id = %d\n\033[0;0m", get_inode_number(dir));
    print_dir_list(dir, false);
    return;
  }
  else{
    debug_print_fsapi("\033[0;33m[FSAPI]print_dir_elements no path was found on path = %s\n\033[0;0m", path);
  }
}

void fs_info(disk_info* info){
  super_block* super = (super_block*) get_super_block();
  //excelent attack vector but cysec in not critical here :(
  if (info != 0 && super != 0){
   info->total_blocks = super->s_blocks_count;
   info->free_blocks = super->s_free_blocks_count;
   info->total_inodes = super->s_inodes_count;
   info->free_inodes = super->s_free_inodes_count;
  }
}

int access(const char *f, int flag){
  //it is the wild west
  //No protections in the os
  return 0;
}