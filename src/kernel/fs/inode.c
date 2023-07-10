#include "inode.h"
#include "fs.h"
#include "ext2.h"
#include "super_block.h"
#include "disk_buffer.h"
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "stdio.h"

int free_inode_list(inode_elt* list){
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
  inode_elt* table_elt = (inode_elt*) malloc(sizeof(inode_elt));
  table_elt->address = address;
  table_elt->inode_id = inode_id;
  table_elt->inode_usage = 1;
  if (table_elt<0){
    return -1;
  }
  if (root_file_system->inode_list == 0){
    root_file_system->inode_list = table_elt;
    table_elt->previous_inode = NULL;
    table_elt->next_inode = NULL;
  }
  else{
    table_elt->next_inode = root_file_system->inode_list;
    root_file_system->inode_list->previous_inode 
      = table_elt->next_inode;
  }
  if (hash_set(
    root_file_system->inode_hash_table,
    (void*)address,
    (void*)table_elt) < 0){
      //No error because we have backup
      PRINT_RED("node table elt was not placed in hash table");
  }
  return 0;
}

int remove_inode_list(uint32_t inode_id, inode_t* inode_address){
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
  if (node_next != NULL){
    node_next->previous_inode = node_previous;
  }
  if (node_previous != NULL){
    node_previous->next_inode = node_next;
  }
  if (node == root_file_system->inode_list && node_next == NULL){
    root_file_system->inode_list = NULL;
  }
  free(node);
  return 0;
}

inode_t* get_inode(uint32_t inode_number){  
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
    node->inode_usage++;
    return node->address;
  }
  else{
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
  inode_elt* node_elt = hash_get(root_file_system->inode_hash_table,
              (void*)inode,
              NULL);
  if (node_elt == 0){
    //Table node not found
    return 0;
  }
  return node_elt->inode_id;
}

int put_inode(inode_t* inode, uint32_t inode_number){
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
  node->inode_usage--;
  if (node->inode_usage == 0){
    remove_inode_list(inode_number, inode);
  }
  return save_fs_block(block_inode, 
          root_file_system->block_size,
          block_number
          );
}

int alloc_bit_bitmap(char* block){
  if (block ==0 ){
    return -1;
  }
  uint64_t mask_max = 0xffffffff;
  for (int i = 0; i<root_file_system->block_size/sizeof(uint64_t); i++){
    uint64_t mask_op = mask_max&*((uint64_t*)(block+i*sizeof(uint64_t)));
    if (mask_op < 0xffffffff){
      //We found an empty node
      uint64_t mask = 1; 
      for (int j = 0; i < 64; ++i) {
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
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (desc_table == 0 ){
    return 0;
  }
  uint32_t inode_number = 0;
  for (int blk_iter = desc_table->bg_inode_bitmap;
   blk_iter<root_file_system->desc_table_loc; blk_iter++){
    char* block_inode = disk_read_block(blk_iter);
    if (block_inode == 0){
      PRINT_RED("Failed to read op while alloc inode");
      return 0;
    }
    int free_inode = alloc_bit_bitmap(block_inode);
    if (free_inode != -1){
      *(block_inode+free_inode/8) &= (1<<free_inode%8);
      if (save_fs_block(block_inode,
           root_file_system->block_size, 
           blk_iter) < 0){
        PRINT_RED("Failed to save inode bitmap changes");
        return 0;
      }
      inode_number = (blk_iter-
                      desc_table->bg_inode_bitmap)
                      *(root_file_system->block_size/INODE_SIZE)
                      +(uint32_t)free_inode;
      break;
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

void free_inode(uint32_t inode_number){
  // char* block_inode = disk_read_block(inode_number);
}

int add_data_block_inode(inode_t* inode){
  super_block* super = (super_block*) get_super_block();
  if (inode == 0 || super ==0){
    return -1;
  }
  if (inode->i_blocks == MAX_BLOCKS_FILE){
    PRINT_RED("Max file size was reached");
    return -1;
  }
  uint32_t blk = get_data_block();
  if (blk == 0 ){
    return -1;
  }
  if (inode->i_blocks<L_DIRECT){
    inode->i_block[inode->i_blocks] = blk;
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
    return save_fs_block(indirect_block_data, 
        root_file_system->block_size,
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
  super_block* super = (super_block*) get_super_block();
  if (dir == 0 || super == 0){
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
  dir_entry.rec_len += dir_entry.rec_len%4; 
  if (dir->i_blocks == 0){
    if (add_data_block_inode(dir)<0){
      return -1;
    }
    dir_entry.rec_len = root_file_system->block_size - dir_entry.rec_len; 
    return save_dir_entry(&dir_entry, 0, dir->i_block[0]);
  }
  //search direct blocks
  else {
    //We look for elemnts in the present direct blocks
    for (int blk = 0; blk<dir->i_blocks; blk++){
      uint32_t pos = 0;
      char* block_data = disk_read_block(
            super->s_first_data_block+blk);
      if (block_data == 0){
        PRINT_RED("Failed to read op while adding dir");
        return -1;
      }
      dir_entry_basic* list_elt = 
              (dir_entry_basic*)block_data;
      while (((char*)list_elt)<block_data
                          +root_file_system->block_size
                          -dir_entry.rec_len){
        if (list_elt->rec_len < dir_entry.rec_len){
          list_elt->rec_len = 
              SIZE_DIR_NO_NAME + list_elt->name_len;
          pos = list_elt->rec_len;
          return write_dir_entry(block_data, 
                pos,
              &dir_entry);

        }
        uint32_t jump_by = list_elt->rec_len;
        list_elt = (dir_entry_basic*)((char*)list_elt+jump_by);
      }
    }
    //WE see if we can add a new direct block
    if (dir->i_blocks<L_DIRECT){
      if (add_data_block_inode(dir)<0){
        return -1;
      }
      dir_entry.rec_len = root_file_system->block_size - dir_entry.rec_len; 
      return save_dir_entry(&dir_entry, 0, dir->i_block[dir->i_blocks-1]);
    }
  }
  return -1;
}

uint32_t get_data_block(){
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (desc_table == 0 || super == 0 ){
    return 0;
  }
  uint32_t block_number = 0;
  for (int blk_iter = desc_table->bg_block_bitmap;
   blk_iter< desc_table->bg_block_bitmap; blk_iter++){
    char* block_inode = disk_read_block(blk_iter);
    if (block_inode == 0){
      PRINT_RED("Failed to read op while alloc inode");
      return 0;
    }
    int free_block = alloc_bit_bitmap(block_inode);
    if (free_block != -1){
      *(block_inode+free_block/8) &= (1<<free_block%8);
      if (save_fs_block(block_inode,
           root_file_system->block_size, 
           blk_iter) < 0){
        PRINT_RED("Failed to save data bitmap changes");
        return 0;
      }
      block_number = super->s_first_data_block + 
                  (uint32_t) free_block;
      break;
    }
  }
  char* block_data = disk_read_block(
                            super->s_first_data_block+
                            block_number);
  if (block_data == 0){
    return 0;
  }
  memset(block_data, 0, 
    root_file_system->block_size);
  return save_fs_block(block_data, 
    root_file_system->block_size,
    block_number
  );
  //empty block before
  return block_number;
}

//0 is reserved not used block use for error handeling
int free_data_block(uint32_t data_block){
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (desc_table == 0 || super == 0 ){
    return -1;
  }
  if (data_block> super->s_blocks_count || data_block<0){
    return -1;
  }
  uint32_t block_bitmap = desc_table->bg_block_bitmap + 
                            data_block/(8*root_file_system->block_size);
  char* data_bitmap = disk_read_block(block_bitmap);
  if (data_bitmap == 0){
    return -1;
  }
  *(data_bitmap+data_block/8) = 1<<data_block%8;
  return save_fs_block(data_bitmap,
           root_file_system->block_size, 
           block_bitmap);
}


