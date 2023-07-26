#include "process.h"
#include "fs_bridge.h"
#include "helperfunc.h"
#include "logger.h"
#include "../fs/inode_util.h"

inode_t* get_current_dir(){
 process* proc =  get_process_struct_of_pid(getpid()); 
  if (proc == 0){
    return 0;
  }
  return proc->cur_dir.inode;
}


flip* add_new_element_open_files(){
  flip* open_file = (flip*) 
            malloc(sizeof(flip));
  if (open_file == 0){
    return 0;
  } 
  open_file->fd = increment_fd_counter();
  open_file->next_file = 0;
  open_file->file_previous = 0;
  open_file->inode_number = 0;
  open_file->usage_counter = 1;
  open_file->position = 0;
  open_file->f_inode = 0;
  open_file->f_inode = 0;
  open_file->can_read = 0;
  open_file->can_write = 0;
  open_file->append_on = 0;
  open_file->sync_directly = 0; 
  if (proc_mang_g.open_files_table == 0){
    proc_mang_g.open_files_table = open_file;
    if (proc_mang_g.open_files_table == 0){
      return 0; 
    }
    proc_mang_g.open_files_table->next_file = 0;
    proc_mang_g.open_files_table->file_previous = 0;
    return proc_mang_g.open_files_table;
  }
  else {
    open_file->next_file = proc_mang_g.open_files_table;
    proc_mang_g.open_files_table->file_previous
      = open_file;
    proc_mang_g.open_files_table = open_file; 
    return  proc_mang_g.open_files_table;
  }
}

flip* get_fs_list_elt(int fd){
  debug_print_fsapi("[FSAPI]Get fs api elt num %d\n", fd);
  if (fd<0){
    return 0;
  }
  flip* fd_list_iter =  proc_mang_g.open_files_table;
  while (fd_list_iter != 0){
    if (fd_list_iter->fd == fd){
      debug_print_fsapi("[FSAPI]Get fs api num %d was found\n", fd);
      return fd_list_iter;
    }
    fd_list_iter = fd_list_iter->next_file;
  }
  return 0;
}

int remove_fd_list(int fd){
  if (fd < 0){
    return -1;
  }
  debug_print_fsapi("[FSAPI]Remove fd list called on fd = %d\n", fd);
  flip* fd_elt = get_fs_list_elt(fd);
  fd_elt->usage_counter--;
  if (fd_elt->usage_counter != 0){
    debug_print_fsapi("[FSAPI]Fd usage is not equal to zero, = %d\n", 
        fd_elt->usage_counter);
    return 0;
  }
  debug_print_fsapi("[FSAPI]Tyring to remove inode number = %d\n", fd_elt->inode_number);
  if (put_inode(fd_elt->f_inode, 0, RELEASE_INODE)<0){
    return -1;
  }
  flip* fd_elt_next = fd_elt->next_file;
  flip* fd_elt_previous = fd_elt->file_previous;
  if (fd_elt == proc_mang_g.open_files_table
        && fd_elt_next == NULL 
        && fd_elt_previous == NULL){
    proc_mang_g.open_files_table = NULL;
  }else{
    if (fd_elt_next != NULL){
      fd_elt_next->file_previous = fd_elt_previous;
    }
    if (fd_elt_previous != NULL){
      fd_elt_previous->next_file = fd_elt_next;
    }
    if (fd_elt == proc_mang_g.open_files_table){
      proc_mang_g.open_files_table = fd_elt_next;
    }
  }
  free(fd_elt);
  debug_print_fsapi("[FSAPI]Open files list num %d was removed\n", fd);
  return 0;
}