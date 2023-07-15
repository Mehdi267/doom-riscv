#include "inode.h"
#include "fs.h"
#include "ext2.h"
#include "super_block.h"
#include "disk_buffer.h"
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "logger.h"


void print_cache_details(inode_elt* list){
  print_inode_no_arg("-----[INODE CACHE DETAILS]---\n");
  if (list == 0){
    return ;
  }
  inode_elt* list_iter = list;
  while(list_iter != NULL){
    debug_print_inode("address = %p\n", list_iter->address);
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
  debug_print_inode("[IN]Inode list n %d is being added to cache\n", inode_id);
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
  if (hash_set(
    root_file_system->inode_hash_table,
    (void*)address,
    (void*)table_elt) < 0){
      //No error because we have backup
      PRINT_RED("node table elt was not placed in hash table\n");
  }
  debug_print_inode("[IN]Inode n %d was added to cache\n",
       inode_id);
  return 0;
}

int remove_inode_list(uint32_t inode_id, inode_t* inode_address){
  debug_print_inode("[IN]Inode list n %d is being removed to cache\n", inode_id);
  inode_elt* node = NULL;
  if (inode_address == 0){
    node = get_inode_t_elt(get_inode_from_node_id(inode_id));
  }
  else{
    node = get_inode_t_elt(inode_address);
  }
  if (node == 0){
    return -1;
  }
  inode_elt* node_next = node->next_inode;
  inode_elt* node_previous = node->previous_inode;
  if (node == root_file_system->inode_list && node_next == NULL && node_previous == NULL){
    root_file_system->inode_list = NULL;
  }else{
    if (node_next != NULL){
      node_next->previous_inode = node_previous;
    }
    if (node_previous != NULL){
      node_previous->next_inode = node_next;
    }
  }
  hash_del(root_file_system->inode_hash_table, 
          (void*)inode_address);
  free(node);
  return 0;
}

inode_t* get_inode(uint32_t inode_number){  
  debug_print_inode("\033[0;36m[IN]Getting inode num %d\n\033[0;0m", inode_number);
  //print_cache_details(root_file_system->inode_list);
  super_block* super = (super_block*) get_super_block();
  block_group_descriptor* desc_table = 
    (block_group_descriptor*) get_desc_table();
  if (super == 0 || desc_table == 0){
    return 0;
  }
  if (inode_number<0 ||
    inode_number > super->s_inodes_count){
    return 0;
  }
  //We look for the node in the cache
  inode_elt* node =  get_inode_t_elt(get_inode_from_node_id(inode_number));
  if (node != 0){
    print_inode_no_arg("\033[0;32m[IN]get inode found in cache\033[0;0m\n");
    node->inode_usage++;
    return node->address;
  }
  else{
    print_inode_no_arg("\033[0;32m[IN]get inode not found in cache\033[0;0m\n");
    debug_print_inode("[IN]Read disk block %d while getting inode %d\n", desc_table->bg_inode_table 
                                        +inode_number*INODE_SIZE
                                        /root_file_system->block_size, inode_number);
    char *block_inode = disk_read_block(desc_table->bg_inode_table 
                                        +inode_number*INODE_SIZE
                                        /root_file_system->block_size);
    if (block_inode == 0){
      return 0;
    }
    inode_t* return_inode = (inode_t*) 
        malloc(sizeof(inode_t));
    if (return_inode == 0){
      PRINT_RED("No memory when allocating inode");
      return 0;
    }
    memcpy(return_inode, block_inode+inode_number*INODE_SIZE, INODE_SIZE);
    add_inode_list(return_inode, inode_number);
    return return_inode;
  }
  return 0;
}

inode_t* get_inode_from_node_id(uint32_t node_id){
  inode_elt* elt_iter = root_file_system->inode_list;
  while (elt_iter != 0){
    if (elt_iter->inode_id == node_id){
      return elt_iter->address;
    }
    elt_iter = elt_iter->next_inode;
  }
  return 0;
}

inode_elt* get_inode_t_elt(inode_t* inode){
  print_inode_no_arg("[IN]Looking for inode cache elt\n");
  if(root_file_system == 0){
    return 0;
  }
  if (root_file_system->inode_hash_table == 0){
    return 0;
  }
  return hash_get(root_file_system->inode_hash_table,
              (void*)inode,
              NULL);
}

uint32_t get_inode_number(inode_t* inode){
  print_inode_no_arg("[IN]Looking for inode number\n");
  inode_elt* node_elt = hash_get(root_file_system->inode_hash_table,
              (void*)inode,
              NULL);
  if (node_elt == 0){
    print_inode_no_arg("[IN]Inode number not found\n");
    //Table node not found
    return 0;
  }
  print_inode_no_arg("[IN]Inode number was found in hash table\n");
  return node_elt->inode_id;
}

int put_inode(inode_t* inode, uint32_t inode_number, put_op op_type){
  debug_print_inode("\033[0;32m[IN]Put inode called on %d\n\033[0;0m", inode_number);
  super_block* super = (super_block*) get_super_block();
  block_group_descriptor* desc_table = 
    (block_group_descriptor*) get_desc_table();
  if (super == 0 || desc_table == 0){
    return -1;
  }
  if (inode_number == 0){
    inode_number = get_inode_number(inode);
  }
  if (inode_number<=0 ||
    inode_number > super->s_inodes_count){
    return -1;
  }
  int block_number = desc_table->bg_inode_table 
                    + inode_number*INODE_SIZE
                    /root_file_system->block_size;
  char* block_inode = disk_read_block(block_number);
  if (block_inode ==0){
    return -1;
  }
  inode_t* inode_ptr =  (inode_t*)(block_inode+inode_number*INODE_SIZE);
  memcpy(inode_ptr, inode, sizeof(inode_t));
  inode_elt* node = get_inode_t_elt(inode);
  if (op_type == RELEASE_INODE){
    node->inode_usage--;
    if (node->inode_usage == 0){
      remove_inode_list(inode_number, inode);
    }
  }
  debug_print_inode("\033[0;34m[IN]Put inode reached end on %d\033[0;0m\n", inode_number);
  return save_fs_block(block_inode, 
          root_file_system->block_size,
          block_number
          );
}

int alloc_bit_bitmap(char* block){
  print_inode_no_arg("[IN]Trying to allocate a bit in the bitmap\n");
  if (block ==0 ){
    return -1;
  }
  uint64_t mask_max = 0xffffffff;
  for (int i = 0; i<root_file_system->block_size/sizeof(uint64_t); i++){
    char* iter_address = block+i*sizeof(uint64_t);
    // printf("iter_address %p\n", iter_address);
    uint64_t mask_op = mask_max&*((uint64_t*)(iter_address));
    if (mask_op < 0xffffffff){
      //We found an empty node
      uint64_t mask = 1; 
      for (int j = 0; j < 64; ++j) {
        // printb(&mask_op, 8);
        // printf("looking for bit j = %d\n",j);
        if ((mask_op & mask) == 0) {
            // Found the first zero bit
            return j+i*sizeof(uint64_t);
        }
        // Shift the mask to the next bit position
        mask <<= 1;
      }
    }
  } 
  return -1;
}

inode_t* alloc_inode(){
  print_inode_no_arg("\033[0;33m[IN]Allocate a new inode\n\033[0;0m");
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (desc_table == 0 ){
    return 0;
  }
  uint32_t inode_number = 0;
  for (int blk_iter = desc_table->bg_inode_bitmap;
    blk_iter<desc_table->bg_inode_table; blk_iter++){
    char* block_inode = disk_read_block(blk_iter);
    if (block_inode == 0){
      PRINT_RED("Failed to read op while alloc inode");
      return 0;
    }
    int free_inode = alloc_bit_bitmap(block_inode);
    if (free_inode != -1){
      debug_print_inode("[IN]Found free node n %d in relative block\n", free_inode);
      *(block_inode+free_inode/8) |= (1<<free_inode%8);
      if (save_fs_block(block_inode,
           root_file_system->block_size, 
           blk_iter) < 0){
        PRINT_RED("[IN]Failed to save inode bitmap changes\n");
        return 0;
      }
      inode_number = (blk_iter-
                      desc_table->bg_inode_bitmap)
                      *(root_file_system->block_size/INODE_SIZE)
                      +(uint32_t)free_inode;
      debug_print_inode("[IN]Final free node num %d \n", inode_number);
      break;
    }
    else{
      PRINT_RED("[IN]No inode was found\n");
      return 0;
    }
  }
  inode_t* inode = (inode_t*)malloc(sizeof(inode_t));
  if (inode == 0){
    PRINT_RED("No space for inode");
    return 0;
  }
  super->s_free_inodes_count--;
  desc_table->bg_free_inodes_count--;
  save_super_block();
  save_blk_desc_table();
  add_inode_list(inode, 
                inode_number);
  return inode;
}

int free_inode(inode_t* inode, uint32_t inode_number){
  print_inode_no_arg("\033[0;36m[IN]Freeing an inode\n\033[0;0m");
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (inode_number == 0 || super ==0 || desc_table == 0){
    return -1;
  }
  debug_print_inode("\033[0;36m[IN]inode->i_blocks = %d\n\033[0;0m", inode->i_blocks);
  if (inode->i_blocks<L_DIRECT){
    for (int i = 0; i<inode->i_blocks; i++){
      if (free_data_block(inode->i_block[i])<0){
        return -1;
      }
    }
  }
  if (inode->i_blocks<L_ONE_INDIRECT){
    char* indirect_block_data = disk_read_block(
                                  super->s_first_data_block+
                                  inode->i_block[INDIRECT_BLOCKS_INDEX]);
    for (int i = 0; i<inode->i_blocks-L_DIRECT-1; i++){
      if (free_data_block(
          *((uint32_t*)indirect_block_data+i))){
          return -1;
      }
    }
    if (free_data_block(inode->i_block[INDIRECT_BLOCKS_INDEX])){
      return -1;
    }
  }
  super->s_free_inodes_count--;
  desc_table->bg_free_inodes_count--;
  save_super_block();
  save_blk_desc_table();
  return 0;
}

int add_data_block_inode(inode_t* inode){
  print_inode_no_arg("\033[0;36m[IN]Adding a data block to an inode\n\033[0;0m");
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (inode == 0 || super == 0 || desc_table == 0){
    return -1;
  }
  if (inode->i_blocks == MAX_BLOCKS_FILE){
    PRINT_RED("Max file size was reached\n");
    return -1;
  }
  uint32_t blk = get_data_block();
  if (blk == 0 ){
    return -1;
  }
  debug_print_inode("\033[0;36m[IN]inode->i_blocks = %d\n\033[0;0m", inode->i_blocks);
  if (inode->i_blocks<L_DIRECT){
    print_inode_no_arg("\033[0;36m[IN]i blocks< Ldirect\n\033[0;0m");
    inode->i_block[inode->i_blocks] = blk;
    inode->i_blocks = inode->i_blocks+1;
    return 0;
  }
  if (inode->i_blocks<L_ONE_INDIRECT){
    uint32_t indirect_block;
    if (inode->i_blocks == L_DIRECT){
      indirect_block = get_data_block();
      if (indirect_block == 0 ){
        return -1;
      }
      inode->i_block[INDIRECT_BLOCKS_INDEX] = indirect_block;
      inode->i_blocks++;
    }
    char* indirect_block_data = disk_read_block(
                                  super->s_first_data_block+
                                  inode->i_block[INDIRECT_BLOCKS_INDEX]);
    if (indirect_block_data == 0){
      return -1;
    }
    *((uint32_t*)indirect_block_data+
                  inode->i_blocks
                  -L_DIRECT) = blk;
    inode->i_blocks++;
    return save_fs_block(indirect_block_data, 
        root_file_system->block_size,
        super->s_first_data_block+
        inode->i_block[INDIRECT_BLOCKS_INDEX]
        );
  }
  else{
    return -1;
  }
}

int write_dir_entry(char* data_block,
                    uint32_t location_b,
                    dir_entry* entry){
  if (data_block == 0 || location_b%4 !=0 || entry ==0){
    return -1;
  }
  debug_print_inode("[IN]write_dir_entry location_b = %d, \n", location_b);
  memcpy(data_block+location_b, entry, SIZE_DIR_NO_NAME);
  memcpy(data_block+location_b+SIZE_DIR_NO_NAME, 
          entry->name, entry->name_len);
  return 0;
}

int save_dir_entry( dir_entry* entry,
                    uint32_t location_b,//Will mostly be zero
                    uint32_t block_nb){
  super_block* super = (super_block*) get_super_block();
  char* block_data = disk_read_block(super->s_first_data_block
                      +block_nb);
  if (block_data == 0){
    return -1;
  }
  if (write_dir_entry(block_data, location_b, entry)<0){
    return -1;
  }
  return save_fs_block(block_data,
          root_file_system->block_size, 
          super->s_first_data_block+
                          +block_nb);
}

int add_inode_directory(inode_t* dir, 
  uint32_t inode_number,
  file_t type,
  char* name,
  size_t name_size){
  print_inode_no_arg("\033[0;35m[IN]Adding an inode to a dir\n\033[0;0m");
  debug_print_inode("[IN]dir->blocks = %d\n",dir->i_blocks);
  debug_print_inode("[IN]inode_number = %d\n",inode_number);
  debug_print_inode("[IN]type = %d\n",type);
  debug_print_inode("[IN]name = %s\n",name);
  debug_print_inode("[IN]name_size = %ld\n",name_size);  
  super_block* super = (super_block*) get_super_block();
  if (dir == 0 || super == 0 || inode_number == 0 || name_size <0){
    return -1;
  }
  if (dir->i_mode != EXT2_S_IFDIR){
    PRINT_RED("inode is not a directory");
    return -1;
  }
  dir_entry dir_entry;
  dir_entry.inode_n = inode_number ;  
  dir_entry.name = name;  
  dir_entry.name_len = name_size;  
  dir_entry.file_type = type;  
  //min size
  dir_entry.rec_len = SIZE_DIR_NO_NAME
                  +dir_entry.name_len;
  if (dir_entry.rec_len%4 != 0){
    dir_entry.rec_len += 4-dir_entry.rec_len%4; 
  }
  debug_print_inode("[IN]rec_lec = %d\n",dir_entry.rec_len);  
  if (dir->i_blocks == 0){
    print_inode_no_arg("[IN]---Adding inode to dir block 0--\n");
    if (add_data_block_inode(dir)<0){
      return -1;
    }
    dir_entry.rec_len = root_file_system->block_size - dir_entry.rec_len;
    debug_print_inode("[IN]Final rec_lec = %d\n",dir_entry.rec_len);  
    return save_dir_entry(&dir_entry, 0, dir->i_block[0]);
  }
  //search direct blocks
  else {
    //We look for elemnts in the present direct blocks
    for (int blk = 0; blk<dir->i_blocks; blk++){
      uint32_t pos = 0;
      char* block_data = disk_read_block(
            super->s_first_data_block+
            dir->i_block[blk]);
      if (block_data == 0){
        PRINT_RED("[IN]Failed to read op while adding dir\n");
        return -1;
      }
      dir_entry_basic* list_elt = 
              (dir_entry_basic*)block_data;
      char* limit = block_data+root_file_system->block_size
                -dir_entry.rec_len;
      bool hit = 0;
      while (((char*)list_elt)<limit){
        // printf("-------Inside %p \n",limit);
        // print_dir_entry(list_elt);
        if (list_elt->file_type == EXT2_FT_FREE
          && SIZE_DIR_NO_NAME+list_elt->name_len <
           dir_entry.rec_len  
          ){
          dir_entry.rec_len = list_elt->rec_len;
          hit = true;
        }
        else if (dir_entry.rec_len < list_elt->rec_len){
          uint32_t old_rec_len = list_elt->rec_len;
          list_elt->rec_len = 
              SIZE_DIR_NO_NAME + list_elt->name_len;
          if (list_elt->rec_len%4 != 0){
            list_elt->rec_len += 4-list_elt->rec_len%4; 
          }
          dir_entry.rec_len = old_rec_len - dir_entry.rec_len;
          debug_print_inode("[IN]Final rec_lec = %d\n",dir_entry.rec_len);
          print_dir_entry_obj(&dir_entry);
          pos += list_elt->rec_len;
          debug_print_inode("\033[0;35m[IN]Adding inode to dir blks n %d actual blk %d\n\033[0;0m", 
          blk, dir->i_block[blk]);
          hit = true;
        }
        if (hit){
          if (write_dir_entry(block_data, 
              pos,
              &dir_entry)<0){return -1;}
          return save_fs_block(block_data,
            root_file_system->block_size, 
            super->s_first_data_block+
                dir->i_block[blk]);
        }
        uint32_t jump_by = list_elt->rec_len;
        pos+=jump_by;
        list_elt = (dir_entry_basic*)((char*)list_elt+jump_by);
      }
    }
    //WE see if we can add a new direct block
    if (dir->i_blocks<L_DIRECT){
      debug_print_inode("[IN]Could not find loc in direct blk, n blk %d\n",dir->i_blocks); 
      print_inode_no_arg("[IN]Creating a new direct blk\n"); 
      if (add_data_block_inode(dir)<0){
        return -1;
      }
      dir_entry.rec_len = root_file_system->block_size - dir_entry.rec_len; 
      debug_print_inode("[IN]Final rec_lec = %d\n",dir_entry.rec_len);  
      return save_dir_entry(&dir_entry, 0, dir->i_block[dir->i_blocks-1]);
    }
  }
  return -1;
}

uint32_t look_for_inode_dir(inode_t* dir, 
        char* name,
        uint32_t name_len){
  if (dir == 0 || name == 0 || name_len == 0){
    return -1;
  }
  debug_print_inode("\033[0;35m[IN]Looking for inode with name %s\n\033[0;0m", name);
  super_block* super = (super_block*) get_super_block();
  if (dir->i_mode != EXT2_S_IFDIR){
    PRINT_RED("inode is not a directory");
    return -1;
  }
  if (dir->i_blocks == 0){
    print_inode_no_arg("[IN]---Directory has no blocks/inodes--\n");
    return 0;
  }
  //search direct blocks
  else {
    //We look for elemnts in the present direct blocks
    for (int blk = 0; blk<dir->i_blocks; blk++){
      uint32_t pos = 0;
      char* block_data = disk_read_block(
            super->s_first_data_block+
            dir->i_block[blk]);
      if (block_data == 0){
        PRINT_RED("[IN]Failed to read op while adding dir\n");
        return -1;
      }
      dir_entry_basic* list_elt = 
              (dir_entry_basic*)block_data;
    uint32_t addition = SIZE_DIR_NO_NAME+list_elt->name_len;
    if ((uint64_t)addition%4 !=0){
      addition += 4-(uint64_t)addition%4;
    }
    char* limit = block_data+root_file_system->block_size
        -(addition);
    while (((char*)list_elt)<limit){
        // print_dir_entry(list_elt);
        if (list_elt->file_type != EXT2_FT_FREE 
            && name_len == list_elt->name_len 
            &&memcmp((char*)list_elt+SIZE_DIR_NO_NAME, 
               name, name_len) == 0 ){
          return list_elt->inode_n;
        }
        uint32_t jump_by = list_elt->rec_len;
        pos+=jump_by;
        list_elt = (dir_entry_basic*)((char*)list_elt+jump_by);
      }
    }
  }
  return 0;
}

int remove_inode_dir(inode_t* dir, 
        char* name,
        uint32_t name_len){
  if (dir == 0 || name == 0 || name_len == 0){
    return -1;
  }
  debug_print_inode("\033[0;35m[IN]Deleting inode with name %s\n\033[0;0m", name);
  super_block* super = (super_block*) get_super_block();
  if (dir->i_mode != EXT2_S_IFDIR){
    PRINT_RED("inode is not a directory");
    return -1;
  }
  if (dir->i_blocks == 0){
    print_inode_no_arg("[IN]---Directory has no blocks/inodes--\n");
    return 0;
  }
  //search direct blocks
  else {
    //We look for elemnts in the present direct blocks
    for (int blk = 0; blk<dir->i_blocks; blk++){
      uint32_t pos = 0;
      char* block_data = disk_read_block(
            super->s_first_data_block+
            dir->i_block[blk]);
      if (block_data == 0){
        PRINT_RED("[IN]Failed to read op while adding dir\n");
        return -1;
      }
      dir_entry_basic* list_elt = 
              (dir_entry_basic*)block_data;
      dir_entry_basic* previous_elt = list_elt; 
      uint32_t size_struct = SIZE_DIR_NO_NAME + 
          list_elt->name_len;
      if ((uint64_t)size_struct%4 !=0){
        size_struct += 4-(uint64_t)size_struct%4;
      }
      uint32_t size_previous = size_struct;
      uint32_t addition = SIZE_DIR_NO_NAME+list_elt->name_len;
      if ((uint64_t)addition%4 !=0){
        addition += 4-(uint64_t)addition%4;
      }
      char* limit = block_data+root_file_system->block_size
          -(addition);
      bool delete = false;
      while (((char*)list_elt)<limit){
        // print_dir_entry(list_elt);
        size_previous = size_struct;
        size_struct = SIZE_DIR_NO_NAME + 
          list_elt->name_len;
        if ((uint64_t)size_struct%4 !=0){
          size_struct += 4-(uint64_t)size_struct%4;
        }
        if (list_elt->file_type != EXT2_FT_FREE 
            && name_len == list_elt->name_len 
            &&memcmp((char*)list_elt+SIZE_DIR_NO_NAME, 
               name, name_len) == 0 ){
          //First element in the block and holds all of it
          if (previous_elt == list_elt){
            if (root_file_system->block_size - size_struct 
                  == list_elt->rec_len){
              delete = true;
            }
            else{
              list_elt->file_type = EXT2_FT_FREE;
            }
          }
          else{
            previous_elt->rec_len = 
              previous_elt->rec_len+list_elt->rec_len; 
            if (previous_elt->file_type == EXT2_FT_FREE 
              && root_file_system->block_size - size_previous
              == previous_elt->rec_len){
              delete = true;
            }
          }
          if (delete){
            debug_print_inode("\033[0;35m[IN]Freeing data block %d\n\033[0;0m", 
                  dir->i_block[blk]);
            if (free_data_block(dir->i_block[blk])<0){
                return -1;
              }
            dir->i_blocks--;
            dir->i_block[blk] = 0;
            return 0;
          }
          return save_fs_block(block_data,
            root_file_system->block_size, 
            super->s_first_data_block+
                dir->i_block[blk]);
        }
        uint32_t jump_by = list_elt->rec_len;
        pos+=jump_by;
        previous_elt = list_elt;
        list_elt = (dir_entry_basic*)((char*)list_elt+jump_by);
      }
    }
  }
  //File does not exist
  return -1;
}

uint32_t get_data_block(){
  print_fs_no_arg("\033[0;32m[IN]Getting data block\n\033[0;m");
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (desc_table == 0 || super == 0 ){
    return 0;
  }
  uint32_t block_number = 0;
  for (int blk_iter = desc_table->bg_block_bitmap;
      blk_iter< desc_table->bg_inode_bitmap; blk_iter++){
    char* block_data_bitmap = disk_read_block(blk_iter);
    if (block_data_bitmap == 0){
      PRINT_RED("Failed to read op while alloc inode");
      return 0;
    }
    int free_block = alloc_bit_bitmap(block_data_bitmap);
    if (free_block != -1){
      debug_print_inode("[IN]Found data block n %d in relative bitmap\n", free_block);
      *(block_data_bitmap+free_block/8) |= (1<<free_block%8);
      if (save_fs_block(block_data_bitmap,
           root_file_system->block_size, 
           blk_iter) < 0){
        PRINT_RED("[IN]Failed to save data bitmap changes");
        return 0;
      }
      block_number = super->s_first_data_block + 
                  (uint32_t) free_block;
      break;
    }
  }
  char* block_data = disk_read_block(block_number);
  if (block_data == 0){
    return 0;
  }
  memset(block_data, 0, 
    root_file_system->block_size);
  int res = save_fs_block(block_data, 
    root_file_system->block_size,
    block_number
  );
  if (res<0){
    return -1;
  }
  super->s_free_blocks_count--;
  desc_table->bg_free_blocks_count--;
  save_super_block();
  save_blk_desc_table();
  debug_print_inode("\033[0;35m[IN]End block location %d \n\033[0;35m", block_number);
  //We return the relative block number
  return block_number-super->s_first_data_block;
}

//0 is reserved not used block use for error handeling
int free_data_block(uint32_t data_block){
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (desc_table == 0 || super == 0 ){
    return -1;
  }
  if (data_block > super->s_blocks_count || data_block == 0){
    return -1;
  }
  uint32_t block_bitmap = desc_table->bg_block_bitmap + 
                            data_block/(8*root_file_system->block_size);
  char* data_bitmap = disk_read_block(block_bitmap);
  if (data_bitmap == 0){
    return -1;
  }
  *(data_bitmap+data_block/8) &= 0xff - (1<<data_block%8);
  super->s_free_blocks_count--;
  desc_table->bg_free_blocks_count--;
  save_super_block();
  save_blk_desc_table();
  return save_fs_block(data_bitmap,
           root_file_system->block_size, 
           block_bitmap);
}

void print_dir_list(inode_t* dir){
  print_inode_no_arg("------Printing dir list------\n");
  super_block* super = (super_block*) get_super_block();
  if (dir == 0){
    return ;
  }
  if (dir->i_mode != EXT2_S_IFDIR){
    PRINT_RED("Inode is not a dir");
    return;
  }
  debug_print_inode("print dir list dir->i_blocks = %d\n", dir->i_blocks);
  if (dir->i_blocks == 0){
      printf("List is empty\n");
  } 
  for (int blk = 0; blk<dir->i_blocks; blk++){
    char* block_data = disk_read_block(
        super->s_first_data_block+dir->i_block[blk]);
    if (block_data == 0){
      PRINT_RED("Failed to read op while printing dir");
      return ;
    }
    dir_entry_basic* list_elt = 
          (dir_entry_basic*)block_data;
    uint32_t addition = SIZE_DIR_NO_NAME+list_elt->name_len;
    if ((uint64_t)addition%4 !=0){
      addition += 4-(uint64_t)addition%4;
    }
    char* limit = block_data+root_file_system->block_size
        -(addition);
    while (((char*)list_elt)<limit){
      if (list_elt->file_type != EXT2_FT_FREE){
        printf("------------\n");
        printf("inode_n %d\n",list_elt->inode_n);
        printf("rec_len %d\n",list_elt->rec_len);
        printf("name_len %d\n",list_elt->name_len);
        printf("file_ type %d\n",list_elt->file_type);
        char filename[list_elt->name_len];
        memcpy(filename, (char*)list_elt + sizeof(dir_entry_basic),
             list_elt->name_len);
        printf("file_name %s\n",filename);
      }
      uint32_t jump_by = list_elt->rec_len;
      list_elt = (dir_entry_basic*)((char*)list_elt+jump_by);
    }
  }
  print_inode_no_arg("------Printing dir list end------\n");
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
