#include "process.h"
#include "fs_bridge.h"
#include "helperfunc.h"
#include "logger.h"
#include "fs/inode_util.h"
#include "assert.h"

uint32_t get_root_dir_name_size(process* proc_arg){
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (proc == 0){return 0;}
  return proc->root_dir.name_size;
}

uint32_t get_current_dir_name_size(process* proc_arg){
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (proc == 0){return 0;}
  return proc->cur_dir.name_size;
}

inode_t* get_current_dir(process* proc_arg){
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (proc == 0){return 0;}
  if (get_inode_number(proc->cur_dir.inode) != 0){
    return proc->cur_dir.inode;
  }
  return walk_and_get(proc->cur_dir.dir_name, 0);
}

char* get_current_dir_name(process* proc_arg){
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (proc == 0){return 0;}
  return proc->cur_dir.dir_name;
}


inode_t* get_root_dir(process* proc_arg){
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (proc == 0){return 0;}
  if (get_inode_number(proc->root_dir.inode) != 0){
    return proc->root_dir.inode;
  }
  return walk_and_get(proc->root_dir.dir_name, 0);
}

char* get_root_dir_name(process* proc_arg){
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (proc == 0){return 0;}
  return proc->root_dir.dir_name;
}

int set_current_dir(process * proc_arg, char* new_name, uint32_t name_size, inode_t* inode){
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (proc == 0){return 0;}
  if (new_name == 0 || name_size == 0 || inode == 0){
    return -1;
  }
  if (!is_absolute_directory(new_name)){
    PRINT_RED("Must be an ablsolute directory \n");
  }
  if (proc->cur_dir.dir_name != 0){
    free(proc->cur_dir.dir_name);
  }
  proc->cur_dir.dir_name = new_name;
  proc->cur_dir.name_size = name_size;
  proc->cur_dir.inode = inode;
  return 0;
}


static int alloc_bit_fdmap(char* fd_map){
  print_inode_no_arg("[IN]Trying to allocate a bit in the fd bitmap\n");
  if (fd_map ==0 ){
    return -1;
  }
  uint64_t mask_max = 0xffffffffffffffff;
  for (int i = 0; i<SIZE_BIT_MAP/sizeof(uint64_t); i++){
    char* iter_address = fd_map+i*sizeof(uint64_t);
    uint64_t mask_op = mask_max&*((uint64_t*)(iter_address));
    if (mask_op < 0xffffffffffffffff){
      //We found an empty node
      uint64_t mask = 1; 
      for (int j = 0; j < 64; ++j) {
        if ((mask_op & mask) == 0) {
            // Found the first zero bit
            return j+i*8*sizeof(uint64_t);
        }
        // Shift the mask to the next bit position
        mask <<= 1;
      }
    }
  } 
  return -1;
}


open_fd* dup_open_file(process* proc_arg, flip* open_file, int custom_fd){
  debug_print_fsapi("\033[0;34m[FSAPI]{dup_open_file} was called custom fd : %d;\033[0;0m\n",
        custom_fd);
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (open_file == 0 || proc == 0 || custom_fd>=MAX_FS){
    return 0;
  }
  open_fd* open_file_proc = (open_fd*) 
            malloc(sizeof(open_fd));
  if (open_file_proc == 0){
    return 0;
  }
  //Not a specific fd
  if (custom_fd < 0){
    open_file_proc->fd = alloc_bit_fdmap(proc->fd_bitmap);
    assert(proc->fd_bitmap + (open_file_proc->fd/8) < proc->fd_bitmap + SIZE_BIT_MAP);
    *((char*)((proc->fd_bitmap + (open_file_proc->fd/8)))) |=
         (1<<(open_file_proc->fd%8));
  }else{
    assert(proc->fd_bitmap + (custom_fd/8) < proc->fd_bitmap + SIZE_BIT_MAP);
    *((char*)((proc->fd_bitmap + (custom_fd/8)))) |=
        (1<<(custom_fd%8));
    //We assume that the file has already been allocated 
    open_file_proc->fd = custom_fd;
  }
  open_file_proc->next_file = 0;
  open_file_proc->file_previous = 0;
  open_file->usage_counter++;
  open_file_proc->file_info = open_file;
  if (proc->open_files_table == 0){
    proc->open_files_table = open_file_proc;
    if (proc->open_files_table == 0){
      return 0; 
    }
    proc->open_files_table->next_file = 0;
    proc->open_files_table->file_previous = 0;
  }
  else {
    open_file_proc->next_file = proc->open_files_table;
    proc->open_files_table->file_previous
      = open_file_proc;
    proc->open_files_table = open_file_proc; 
  }
  debug_print_fsapi("\033[0;34m[FSAPI]{dup_open_file} ran well, new file fd: %d; usage of fd : %d\033[0;0m\n",
        custom_fd, open_file->usage_counter);
  return open_file_proc;
}



open_fd* add_new_element_open_files(process* proc_arg){
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (proc == 0){return 0;}
  flip* open_file = (flip*) 
            malloc(sizeof(flip));
  open_fd* open_file_proc = (open_fd*) 
            malloc(sizeof(open_fd));
  if (open_file == 0 || open_file_proc == 0){
    return 0;
  }
  open_file_proc->fd = alloc_bit_fdmap(proc->fd_bitmap);
  assert(proc->fd_bitmap + (open_file_proc->fd/8) < proc->fd_bitmap + SIZE_BIT_MAP);
  *((char*)((proc->fd_bitmap + (open_file_proc->fd/8)))) |=
         (1<<open_file_proc->fd%8);
  open_file_proc->next_file = 0;
  open_file_proc->file_previous = 0;
  open_file_proc->file_info = open_file;
  open_file->inode_number = 0;
  open_file->usage_counter = 1;
  open_file->position = 0;
  open_file->f_inode = 0;
  open_file->f_inode = 0;
  open_file->can_read = 0;
  open_file->can_write = 0;
  open_file->append_on = 0;
  open_file->sync_directly = 0; 
  if (proc->open_files_table == 0){
    proc->open_files_table = open_file_proc;
    if (proc->open_files_table == 0){
      return 0; 
    }
    proc->open_files_table->next_file = 0;
    proc->open_files_table->file_previous = 0;
    return open_file_proc;
  }
  else {
    open_file_proc->next_file = proc->open_files_table;
    proc->open_files_table->file_previous
      = open_file_proc;
    proc->open_files_table = open_file_proc; 
    return open_file_proc;
  }
}

flip* get_fs_list_elt(process* proc_arg, int fd){
  debug_print_fsapi("[FSAPI]Get fs api elt num %d\n", fd);
  if (fd<0){
    return 0;
  }
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (proc == 0){return 0;}
  open_fd* fd_list_iter = proc->open_files_table;
  while (fd_list_iter != 0){
    if (fd_list_iter->fd == fd){
      debug_print_fsapi("[FSAPI]Get fs api num %d was found\n", fd);
      return fd_list_iter->file_info;
    }
    fd_list_iter = fd_list_iter->next_file;
  }
  debug_print_fsapi("[FSAPI]Get fs api num %d was not found\n", fd);
  return 0;
}

open_fd* get_open_fd_elt(process* proc_arg, int fd){
  debug_print_fsapi("[FSAPI]Get fs api elt num %d\n", fd);
  if (fd<0 || fd>=MAX_FS){
    return 0;
  }
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (proc == 0){return 0;}
  open_fd* fd_list_iter = proc->open_files_table;
  while (fd_list_iter != 0){
    if (fd_list_iter->fd == fd){
      debug_print_fsapi("[FSAPI]Get fs api num %d was found\n", fd);
      return fd_list_iter;
    }
    fd_list_iter = fd_list_iter->next_file;
  }
  return 0;
}


bool check_if_inode_is_being_used(process* proc_arg, uint32_t inode_number){
  if (inode_number == 0){
    return 0;
  }
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {
    proc = proc_arg;
  }
  if (proc == 0){return 0;}
  debug_print_fsapi("[FSAPI]Cheking if inode n %d is being used\n",
       inode_number);
  open_fd* fd_list_iter =  proc->open_files_table;
  while (fd_list_iter != 0){
    if (fd_list_iter->file_info->inode_number == inode_number){
      debug_print_fsapi("[FSAPI]Inode is still being used%d, fd %d \n",
           inode_number, fd_list_iter->fd);
      return true;
    }
    fd_list_iter = fd_list_iter->next_file;
  }
  return false;
}

int remove_fd_list(process* proc_arg, int fd, rem_type op_type){
  if (fd < 0){
    return -1;
  }
  debug_print_fsapi("[FSAPI]Remove fd list called on fd = %d\n", fd);
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (proc == 0){return -1;}
  open_fd* fd_elt = get_open_fd_elt(proc, fd);
  if (fd_elt == 0){
    return -1;
  }
  fd_elt->file_info->usage_counter--;
  if (fd_elt->file_info->usage_counter != 0){
    debug_print_fsapi("[FSAPI]Fd usage is not equal to 0, rm it only from the cur proc; usage = %d\n", 
        fd_elt->file_info->usage_counter);
  }
  else{
    debug_print_fsapi("[FSAPI]Fd usage equal to 0, removing it completly from proc; usage = %d\n", 
        fd_elt->file_info->usage_counter);
    if (fd_elt->file_info->type == FS_FT_PIPE){
        close_type close_t = fd_elt->file_info->can_read ?
            CLOSE_READ : CLOSE_WRITE;   
        close_pipe(fd_elt->file_info->f_pipe, close_t);
    }
    else { //Inode and device files that both use inodes
      debug_print_fsapi("[FSAPI]Tyring to remove inode number = %d\n",
        fd_elt->file_info->inode_number);
      if (put_inode(fd_elt->file_info->f_inode, 0, RELEASE_INODE)<0){
        return -1;
      }
    }
    free(fd_elt->file_info);
  }
  fd_elt->file_info = 0;
  if (op_type == REMOVE_ALL){
    open_fd* fd_elt_next = fd_elt->next_file;
    open_fd* fd_elt_previous = fd_elt->file_previous;
    if (fd_elt == proc->open_files_table
          && fd_elt_next == NULL 
          && fd_elt_previous == NULL){
      proc->open_files_table = NULL;
    }else{
      if (fd_elt_next != NULL){
        fd_elt_next->file_previous = fd_elt_previous;
      }
      if (fd_elt_previous != NULL){
        fd_elt_previous->next_file = fd_elt_next;
      }
      if (fd_elt == proc->open_files_table){
        proc->open_files_table = fd_elt_next;
      }
    }
    assert(proc->fd_bitmap + (fd_elt->fd/8) < 
        proc->fd_bitmap + SIZE_BIT_MAP);
    *((char*)(proc->fd_bitmap + (fd_elt->fd/8)))
          &= 0xff -(1<<fd_elt->fd%8);
    free(fd_elt);
    debug_print_fsapi("[FSAPI]Open files list num %d was removed\n", fd);
  }
  //This is used so that we can use the link 
  //to attach an other file to it
  else if (op_type == ONLY_CLOSE_FILE){
    debug_print_fsapi("[FSAPI]Close the file but did not remove the link, fd = %d\n", fd);
  }
  return 0;
}

int close_all_files(process* proc_arg){
  printf("close_all_files was called \n");
  process* proc;
  if (proc_arg == NULL){
    proc = get_process_struct_of_pid(getpid()); 
  } else {proc = proc_arg;}
  if (proc == 0){return -1;}
  int res = 0;
  open_fd* fd_list_iter = proc->open_files_table;
  while (fd_list_iter != 0){
    open_fd* temp_next = fd_list_iter->next_file;
    if (remove_fd_list(proc, fd_list_iter->fd, REMOVE_ALL)<0){
      res = -1;
    }
    fd_list_iter = temp_next;
  }
  return res;
}

int copy_proc_fds(process* dest_proc, process* src_proc){
  if (src_proc == 0 || dest_proc == 0){
    return -1;
  }
  debug_print_fsapi("[FSAPI]Copying fds: %s from %s\n", 
      dest_proc->process_name ,src_proc->process_name);
  open_fd* fd_src_iter = src_proc->open_files_table;
  while (fd_src_iter != 0){
    open_fd* open_file_proc = (open_fd*) 
            malloc(sizeof(open_fd));
    if (open_file_proc == 0){return -1;}
    assert(fd_src_iter->file_info !=0);
    open_file_proc->fd = fd_src_iter->fd;
    open_file_proc->file_info = fd_src_iter->file_info;
    fd_src_iter->file_info->usage_counter++;
    open_file_proc->next_file = NULL;
    open_file_proc->file_previous = NULL;
    if (dest_proc->open_files_table == NULL){
      dest_proc->open_files_table =  open_file_proc;
    } else {
      open_file_proc->next_file = dest_proc->open_files_table;
      dest_proc->open_files_table->file_previous = open_file_proc;
      dest_proc->open_files_table = open_file_proc;
    }
    fd_src_iter = fd_src_iter->next_file;
  }
  debug_print_fsapi("[FSAPI]Copy completed : %s from %s\n", 
      dest_proc->process_name ,src_proc->process_name);
  return 0;
}