#include "inode.h"
#include "process/helperfunc.h"
#include "process/fs_bridge.h"
#include "fs.h"
#include "ext2.h"
#include "fs_api.h"
#include "dir_api.h"
#include "super_block.h"
#include "disk_buffer.h"
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "logger.h"
#include <assert.h>

void print_cache_details(inode_elt* list){
  print_inode_no_arg("-----[INODE CACHE DETAILS]---\n");
  if (list == 0){
    return ;
  }
  inode_elt* list_iter = list;
  while(list_iter != NULL){
    debug_print_inode("address = %p\n", list_iter->address);
    debug_print_inode("inode mode= %x\n", list_iter->address->i_mode);
    debug_print_inode("inode_id = %d\n", list_iter->inode_id);
    debug_print_inode("inode_usage = %d\n", list_iter->inode_usage);
    list_iter = list_iter->next_inode;
  }
  print_inode_no_arg("-----[INODE CACHE DETAILS END]---\n");
}


int free_inode_list(inode_elt* list){
  print_inode_no_arg("[IN]Freeing inode list\n");
    if (list == 0){
  return 0;
  }
  inode_elt* list_iter = list;
  inode_elt* list_prev = list;
  while(list_iter != NULL){
    if (list_iter->address != 0){
      free(list_iter->address);
    }
    list_prev = list_iter;
    list_iter = list_iter->next_inode;
    free(list_prev);
  }
  return 0;
}

int add_inode_list(inode_t* address, uint32_t inode_id){
  debug_print_inode("[IN]Inode list num %d is being added to cache\n", inode_id);
  inode_elt* table_elt = (inode_elt*) malloc(sizeof(inode_elt));
  if (table_elt == 0){
    return -1;
  }
  table_elt->address = address;
  table_elt->inode_id = inode_id;
  table_elt->inode_usage = 1;
  table_elt->previous_inode = NULL;
  table_elt->next_inode = NULL;
  if (root_file_system->inode_list == 0){
    root_file_system->inode_list = table_elt;
  }
  else{
    table_elt->next_inode = root_file_system->inode_list;
    root_file_system->inode_list->previous_inode
      = table_elt;
    root_file_system->inode_list = table_elt; 
  }
  // if (hash_set(
  //   root_file_system->inode_hash_table,
  //   (void*)address,
  //   (void*)table_elt) < 0){
  //    //No error because we have backup
  //    PRINT_RED("node table elt was not placed in hash table\n");
  // }
  debug_print_inode("[IN]Inode num %d was added to cache\n",
     inode_id);
  return 0;
}

int remove_inode_list(uint32_t inode_id, inode_t* inode_address){
  debug_print_inode("[IN]Inode list num %d is being removed from cache\n", inode_id);
  inode_elt* node = NULL;
  //We need to find the node in the linked list
  if (inode_id == 0 && inode_address != 0){
    inode_id = get_inode_number(inode_address);
    if (inode_id == 0){
      return -1;
    }
  }
  if (inode_address == 0){
    node = get_inode_t_elt(get_inode_from_node_id(inode_id));
  }
  else{
    node = get_inode_t_elt(inode_address);
  }
  if (node == 0){
    return -1;
  }
  assert(node->inode_id == inode_id);
  inode_elt* node_next = node->next_inode;
  inode_elt* node_previous = node->previous_inode;
  if (node == root_file_system->inode_list 
    && node_next == NULL 
    && node_previous == NULL){
    root_file_system->inode_list = NULL;
  }else{
    if (node_next != NULL){
      node_next->previous_inode = node_previous;
  }
  if (node_previous != NULL){
    node_previous->next_inode = node_next;
  }
  if (node == root_file_system->inode_list){
    root_file_system->inode_list = node_next;
  }
  }
  // // hash_del(root_file_system->inode_hash_table, 
  //       (void*)inode_address);
  free(inode_address);
  free(node);
  debug_print_inode("[IN]Inode list num %d was removed from cache and freed\n", inode_id);
  return 0;
}

void print_dir_entry_basic(dir_entry_basic* entry){
  if (entry == 0 ){
  return;
  }
  print_inode_no_arg("----entry no name ----\n");
  debug_print_inode("inode_n = %d", entry->inode_n);
  debug_print_inode("rec_len = %d", entry->rec_len);
  debug_print_inode("name_len = %d", entry->name_len);
  debug_print_inode("file_type = %d", entry->file_type);
  print_inode_no_arg("--------\n");
}

void print_dir_entry(dir_entry_basic* entry){
  if (entry == 0 ){
  return;
  }
  print_inode_no_arg("\033[0;32m----Printing disk entry----\n");
  debug_print_inode("inode_n %d\n", entry->inode_n);
  debug_print_inode("rec_len %d\n", entry->rec_len);
  debug_print_inode("name_len %d\n", entry->name_len);
  debug_print_inode("file_ type %d\n", entry->file_type);
  char filename[entry->name_len];
  memcpy(filename, (char*)entry + sizeof(dir_entry_basic), 
    entry->name_len);
  debug_print_inode("file_name %s\n", filename);
  print_inode_no_arg("--------\033[0;0m\n");
}

void print_dir_entry_obj(dir_entry* entry){
  if (entry == 0 ){
  return;
  }
  print_inode_no_arg("\033[0;32m----Printing disk entry----\n");
  debug_print_inode("inode_n %d\n", entry->inode_n);
  debug_print_inode("rec_len %d\n", entry->rec_len);
  debug_print_inode("name_len %d\n", entry->name_len);
  debug_print_inode("file_type %d\n", entry->file_type);
  debug_print_inode("file_name %s\n", entry->name);
  print_inode_no_arg("--------\033[0;0m\n");
}


inode_t* walk_and_get(const char* path, uint32_t backward_steps){
  debug_print_inode("\033[0;33m[IN]walk_and_get called on %s with back steps = %d\033[0;0m\n",
     path, backward_steps);
  path_fs* path_data = extract_files(path);
  if (path_data == 0){
    return 0;
  }
  inode_t* inode = 0;
  if (is_absolute_directory(path)){
    print_inode_no_arg("Path is an absolute dir \n");
    inode = get_inode(EXT2_GOOD_OLD_FIRST_INO);  
    if (strcmp(path, "/") == 0){
      free_path_fs(path_data);
      return inode;
    }
  } else {
    print_inode_no_arg("Path is a relative dir \n");
    inode = get_current_dir(NULL);
  }
  debug_print_inode("[IN]Walk Exploration depth = %d, cur dir inode %d\n", 
    path_data->nb_files - backward_steps, get_inode_number(inode));
  if (path_data->nb_files - backward_steps >=0){
    for (uint32_t file_iter = 0; file_iter < path_data->nb_files - backward_steps; file_iter++) {
      inode = get_inode(look_for_inode_dir(inode, 
      path_data->files[file_iter], 
      strlen(path_data->files[file_iter]))); 
    }
  }
  free_path_fs(path_data);
  debug_print_inode("[IN]Finished walk on path %s\n",path);
  return inode;
}

mode_t ext2_to_stat_mode(unsigned short ext2_mode) {
  mode_t stat_mode = 0;
  // Map file type from Ext2 to stat
  if ((ext2_mode & 0xF000) == EXT2_S_IFSOCK) {
    stat_mode |= __S_IFSOCK;
  } else if ((ext2_mode & 0xF000) == EXT2_S_IFLNK) {
    stat_mode |= __S_IFLNK;
  } else if ((ext2_mode & 0xF000) == EXT2_S_IFREG) {
    stat_mode |= __S_IFREG;
  } else if ((ext2_mode & 0xF000) == EXT2_S_IFBLK) {
    stat_mode |= __S_IFBLK;
  } else if ((ext2_mode & 0xF000) == EXT2_S_IFDIR) {
    stat_mode |= __S_IFDIR;
  } else if ((ext2_mode & 0xF000) == EXT2_S_IFCHR) {
    stat_mode |= __S_IFCHR;
  } else if ((ext2_mode & 0xF000) == EXT2_S_IFIFO) {
    stat_mode |= __S_IFIFO;
  }
  return stat_mode;
}

// Map EXT2 file types to DT constants
int map_ext2_to_dt(uint8_t ext2_type) {
  switch (ext2_type) {
    case EXT2_FT_UNKNOWN: return DT_UNKNOWN;
    case EXT2_FT_FIFO: return DT_FIFO;
    case EXT2_FT_CHRDEV: return DT_CHR;
    case EXT2_FT_DIR: return DT_DIR;
    case EXT2_FT_BLKDEV: return DT_BLK;
    case EXT2_FT_REG_FILE: return DT_REG;
    case EXT2_FT_SYMLINK: return DT_LNK;
    case EXT2_FT_SOCK: return DT_SOCK;
    default: return DT_UNKNOWN;
  }
}

//maps ext2 inode mode to ext2_type
int map_ext2_types_to_dir_type(uint32_t ext2_type) {
  switch (ext2_type) {
    case EXT2_S_IFIFO: return EXT2_FT_FIFO;
    case EXT2_S_IFCHR: return EXT2_FT_CHRDEV;
    case EXT2_S_IFDIR: return EXT2_FT_DIR;
    case EXT2_S_IFBLK: return EXT2_FT_BLKDEV;
    case EXT2_S_IFREG: return EXT2_FT_REG_FILE;
    case EXT2_S_IFLNK: return EXT2_FT_SYMLINK;
    case EXT2_S_IFSOCK: return EXT2_FT_SOCK;
    default: return DT_UNKNOWN;
  }
}