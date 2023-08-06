#include "chdev.h"
#include "inode.h"
#include "fs_api.h"
#include "fs.h"
#include "inode_util.h"
#include "../process/helperfunc.h"
#include "../input-output/cons_read.h"
#include "../input-output/cons_write.h"
#include "string.h"

//Equal to the number of virtio ports 
//even though i don't think that we will use all of them 
#define NUM_DEV 11 //0 is used for error checking
chdevop dev_op[NUM_DEV]; 

static long cons_read_wrapper(uint64_t buf, int size){
  return cons_read((const char*) buf, size);
}

static long cons_write_wrapper(uint64_t buf, int size){
  return cons_write((const char*) buf, size);
}

int init_fs_drivers(){
  //Initializes the structures of the file system drivers
  dev_op[TERMINAL_DEVICE].reserved = true;
  dev_op[TERMINAL_DEVICE].read = cons_read_wrapper; 
  dev_op[TERMINAL_DEVICE].write = cons_write_wrapper;
  return 0;
}

int mknod(const char *pathname, mode_t mode, dev_t dev){
  if (pathname == 0){
    return -1;
  }
  uint16_t major = (dev & 0xffff0000)>> 16;
  if (major >= NUM_DEV || major == 0){
    return -1;
  }
  // uint16_t minor = (dev & 0x0000ffff); //Not used atm
  if ((root_file_system != 0) &&
     (dev_op[major].reserved != true)){
    return -1;
  }
  path_fs* path_data = extract_files(pathname);
  if (path_data == 0){
    return -1;
  }
  inode_t* dir_inode = walk_and_get(pathname, 1);
  if (dir_inode == 0 || dir_inode->i_mode != EXT2_S_IFDIR){
    free_path_fs(path_data);
    return -1;
  }
  if (look_for_inode_dir(dir_inode, path_data->files[path_data->nb_files -1],
          strlen(path_data->files[path_data->nb_files -1])) != 0){
    print_fsapi_no_arg("File already exists, deleting its data\n");
    return -1;
  }
  else{
    inode_t* file_inode;
    file_inode = alloc_inode(); 
    if (file_inode != 0){
      file_inode->i_mode = EXT2_S_IFCHR;
      file_inode->i_links_count = 1;
      file_inode->i_osd1 = dev;
      if (add_inode_directory(dir_inode, 
            get_inode_number(file_inode), 
            EXT2_FT_CHRDEV,
            path_data->files[path_data->nb_files -1],
            strlen(path_data->files[path_data->nb_files -1]))<0){
        free_inode(file_inode, get_inode_number(file_inode));
        free_path_fs(path_data);
        return -1;
      }
    }
  free_path_fs(path_data);
  }
  return 0;
}
