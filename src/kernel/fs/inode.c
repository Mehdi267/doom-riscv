#include "inode.h"
#include "fs.h"
#include "ext2.h"
#include "super_block.h"
#include "disk_buffer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "logger.h"
#include <assert.h>
#include "inode_util.h"
#include <dirent.h> //for the dir struct


inode_t* get_inode(uint32_t inode_number){  
  debug_print_inode("\033[0;36m[IN]Getting inode num %d\n\033[0;0m", inode_number);
  //print_cache_details(root_file_system->inode_list);
  super_block* super = (super_block*) get_super_block();
  block_group_descriptor* desc_table = 
    (block_group_descriptor*) get_desc_table();
  if (super == 0 || desc_table == 0){
    return 0;
  }
  if (inode_number<=0 ||
    inode_number > super->s_inodes_count){
    return 0;
  }
  //We look for the node in the cache
  inode_elt* node =  get_inode_t_elt_from_id(inode_number);
  if (node != 0){
    print_inode_no_arg("\033[0;32m[IN]get inode found in cache\033[0;0m\n");
    node->inode_usage++;
    return node->address;
  }
  else{
    print_inode_no_arg("\033[0;32m[IN]get inode not found in cache\033[0;0m\n");
    debug_print_inode("[IN]Read disk block %d while getting inode %d\n", desc_table->bg_inode_table 
                                        +(inode_number*INODE_SIZE)
                                        /root_file_system->block_size, inode_number);
    uint32_t block_nb = desc_table->bg_inode_table 
                                        +(inode_number*INODE_SIZE)
                                        /root_file_system->block_size;
    char *block_inode = disk_read_block(block_nb, LOCK);
    if (block_inode == 0){
      return 0;
    }
    inode_t* return_inode = (inode_t*) 
        malloc(sizeof(inode_t));
    if (return_inode == 0){
      PRINT_RED("No memory when allocating inode");
      return 0;
    }
    memcpy(return_inode, 
          block_inode+(inode_number*INODE_SIZE)%root_file_system->block_size, 
          INODE_SIZE);
    add_inode_list(return_inode, inode_number);
    unlock_cache(block_nb);
    return return_inode;
  }
  return 0;
}

inode_t* get_inode_from_node_id(uint32_t node_id){
  inode_elt* elt_iter = root_file_system->inode_list;
  while (elt_iter != 0){
    if (elt_iter->inode_id == node_id){
      print_inode_no_arg("[IN]Node id was found in cache, returning inode\n");
      return elt_iter->address;
    }
    elt_iter = elt_iter->next_inode;
  }
  return 0;
}

inode_elt* get_inode_t_elt_from_id(uint32_t node_id){
  inode_elt* elt_iter = root_file_system->inode_list;
  while (elt_iter != 0){
    if (elt_iter->inode_id == node_id){
      print_inode_no_arg("[IN]Node id was found in cache, returning inode\n");
      return elt_iter;
    }
    elt_iter = elt_iter->next_inode;
  }
  return 0;
}

int sync_inodes(){
  print_inode_no_arg("[IN]Saving inode data into disk\n");
  if(root_file_system == 0){
    return -1;
  }
  if (root_file_system->inode_list == 0){
    return -1;
  }
  inode_elt* list_iter = root_file_system->inode_list;
  while (list_iter != 0){
    put_inode(list_iter->address, list_iter->inode_id, SAVE_INODE);
    list_iter=list_iter->next_inode;
  }
  return 0;
}

inode_elt* get_inode_t_elt(inode_t* inode){
  print_inode_no_arg("[IN]Looking for inode cache elt\n");
  if(root_file_system == 0){
    return 0;
  }
  // if (root_file_system->inode_hash_table == 0){
  //   return 0;
  // }
  // return hash_get(root_file_system->inode_hash_table,
  //             (void*)inode,
  //             NULL);
  if (root_file_system->inode_list == 0){
    return 0;
  }
  inode_elt* list_iter = root_file_system->inode_list;
  while (list_iter != 0){
    if (list_iter->address == inode){
      return list_iter;
    }
    list_iter=list_iter->next_inode;
  }
  return 0;
}

uint32_t get_inode_number(inode_t* inode){
  print_inode_no_arg("[IN]Looking for an inode's number\n");
  // inode_elt* node_elt = hash_get(root_file_system->inode_hash_table,
  //             (void*)inode,
  //             NULL);
  inode_elt* node_elt = get_inode_t_elt(inode);
  if (node_elt == 0){
    print_inode_no_arg("[IN]Inode number not found\n");
    //Table node not found
    return 0;
  }
  debug_print_inode("[IN]Inode number %d was found in hash table\n", 
            node_elt->inode_id);
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
                    + (inode_number*INODE_SIZE)
                    /root_file_system->block_size;
  debug_print_inode("[IN]Saving Inode num %d, inode mode %x\n",
             inode_number, inode->i_mode);
  char* block_inode = disk_read_block(block_number, LOCK);
  if (block_inode ==0){
    return -1;
  }
  inode_t* inode_ptr =  (inode_t*)(block_inode+(inode_number*INODE_SIZE)
                            %root_file_system->block_size); 
  memcpy(inode_ptr, inode, sizeof(inode_t));
  inode_elt* node = get_inode_t_elt(inode);
  if (op_type == RELEASE_INODE){
    node->inode_usage--;
    if (node->inode_usage == 0){
      remove_inode_list(inode_number, inode);
    }
  } 
  if (save_fs_block(block_inode, 
          root_file_system->block_size,
          block_number,
          UNLOCK)<0){
    return -1;
  }
  debug_print_inode("\033[0;34m[IN]Put inode reached end on %d, \033[0;0m\n", inode_number);
  return 0;
}

int alloc_bit_bitmap(char* block){
  print_inode_no_arg("[IN]Trying to allocate a bit in the bitmap\n");
  if (block ==0 ){
    return -1;
  }
  uint64_t mask_max = 0xffffffffffffffff;
  for (int i = 0; i<root_file_system->block_size/sizeof(uint64_t); i++){
    char* iter_address = block+i*sizeof(uint64_t);
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

inode_t* alloc_inode(){
  print_inode_no_arg("\033[0;33m[IN]Allocate a new inode\n\033[0;0m");
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (super == 0 || desc_table == 0 ){
    return 0;
  }
  if (super->s_free_inodes_count == 0 || 
      desc_table->bg_free_inodes_count == 0){
        return 0;
  }
  uint32_t inode_number = 0; 
  for (int blk_iter = desc_table->bg_inode_bitmap;
    blk_iter<desc_table->bg_inode_table; blk_iter++){
    char* block_inode = disk_read_block(blk_iter, LOCK);
    if (block_inode == 0){
      PRINT_RED("Failed to read op while alloc inode");
      return 0;
    }
    int free_inode = alloc_bit_bitmap(block_inode);
    if (free_inode != -1){
      debug_print_inode("[IN]Found free node num %d in relative block\n", free_inode);
      *(block_inode+free_inode/8) |= (1<<(free_inode%8));
      if (save_fs_block(block_inode,
           root_file_system->block_size, 
           blk_iter, UNLOCK) < 0){
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
  if (inode_number == 0){
    return 0;
  }
  inode_t* inode = (inode_t*)malloc(sizeof(inode_t));
  if (inode == 0){
    return 0;
  }
  memset(inode,0, sizeof(inode_t));
  inode->i_links_count = 1;
  if (inode == 0){
    PRINT_RED("No space for inode");
    return 0;
  }
  int block_number = desc_table->bg_inode_table 
                    + (inode_number*INODE_SIZE)
                    /root_file_system->block_size;
  char* block_inode = disk_read_block(block_number, LOCK);
  if (block_inode ==0){
    return 0;
  }
  inode_t* inode_ptr =  (inode_t*)(block_inode+(inode_number*INODE_SIZE)
                            %root_file_system->block_size); 
  memcpy(inode_ptr, inode, sizeof(inode_t));
  if (save_fs_block(block_inode, 
          root_file_system->block_size,
          block_number, UNLOCK)<0){return 0;}
  super->s_free_inodes_count--;
  desc_table->bg_free_inodes_count--;
  save_super_block();
  save_blk_desc_table();
  add_inode_list(inode, 
                inode_number);
  debug_print_inode("\033[0;33m[IN]Inode num %d was allocated \n\033[0;0m", 
        inode_number);
  return inode;
}

int free_inode(inode_t* inode, uint32_t inode_number){
  debug_print_inode("\033[0;36m[IN]Freeing an inode %d\n\033[0;0m", inode_number);
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (inode_number == 0 || super ==0 || desc_table == 0){
    return -1;
  }
  if (inode == 0){
    inode = get_inode(inode_number);
    if (inode == 0){return -1;}
  }
  if (inode->i_mode == EXT2_S_IFDIR && 
      inode->i_size > BASIC_DOT_DIR_SIZE){
    PRINT_RED("Inode is a directory with data\n");
    return 0;
  }
  inode->i_links_count--;
  if (inode->i_links_count > 0){
    return 0;
  }
  else{
    if (free_inode_data(inode)<0){
      return -1;
    }
    //Free inode in the bit map
    uint32_t inode_bitmap = desc_table->bg_inode_bitmap + 
                              inode_number/(8*root_file_system->block_size);
    char* inode_bitmap_data = disk_read_block(inode_bitmap, LOCK);
    if (inode_bitmap == 0){
      return -1;
    }
    *(inode_bitmap_data+(inode_number%(root_file_system->block_size*8))/8) 
                  &= 0xff - (1<<(inode_number%8));
    if (save_fs_block(inode_bitmap_data,
            root_file_system->block_size, 
            inode_bitmap, UNLOCK) < 0){
      return -1;
    }
    super->s_free_inodes_count++;
    desc_table->bg_free_inodes_count++;
    save_super_block();
    save_blk_desc_table();
  }
  return remove_inode_list(inode_number, inode);
}

int free_inode_data(inode_t* inode){
  debug_print_inode("\033[0;36m[IN]Freeing inode data = %d\n\033[0;0m", inode->i_blocks);
  super_block* super = (super_block*) get_super_block();
  if (inode == 0){
    return -1;
  }
  uint32_t freed_blocks = 0;
  uint32_t disk_blks = inode->i_blocks;
  debug_print_inode("\033[0;36m[IN]Freeing direct blocks inode->i_blocks = %d\n\033[0;0m", inode->i_blocks);
  if (disk_blks>0){
    uint32_t direct = disk_blks>L_DIRECT ? L_DIRECT : inode->i_blocks;
    for (int i = 0; i<direct; i++){
      if (free_data_block(inode->i_block[i])<0){
        return -1;
      }
      freed_blocks++;
    }
    debug_print_inode("Free direct %d\n", freed_blocks);
  }
  if (disk_blks > L_DIRECT){
    debug_print_inode("\033[0;36m[IN]Freeing indirect blocks inode->i_blocks = %d\n\033[0;0m", inode->i_blocks);
    char* indirect_block_data = disk_read_block(
                                  super->s_first_data_block+
                                  inode->i_block[INDIRECT_BLOCKS_INDEX], LOCK);
    if (indirect_block_data == 0){
      return -1;
    }
    uint32_t indirect_blocks = disk_blks >= L_ONE_INDIRECT  ? REL_NB_INDIRECT : inode->i_blocks - L_DIRECT -1;
    for (int i = 0; i<indirect_blocks; i++){
      if (free_data_block(
          *((uint32_t*)indirect_block_data+i))<0){
          return -1;
      }
      freed_blocks++;
    }
    unlock_cache(super->s_first_data_block+inode->i_block[INDIRECT_BLOCKS_INDEX]);
    if (free_data_block(inode->i_block[INDIRECT_BLOCKS_INDEX])<0){
      return -1;
    }
    freed_blocks++;
    debug_print_inode("Free indirect %d\n", freed_blocks);
  }
  if (disk_blks > L_ONE_INDIRECT){
    debug_print_inode("\033[0;36m[IN]Freeing double indirect block = %d\n\033[0;0m", inode->i_blocks);
    uint32_t double_ind_blk_nb = 0;
    uint32_t nb_elts_blk = root_file_system->block_size/sizeof(uint32_t);
    if (inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX] == 0){
      return -1;
    }
    char* main_double_block = disk_read_block(super->s_first_data_block+
          inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX], LOCK);
    if (main_double_block == 0){return -1;}
    for (int itr = 0; itr < nb_elts_blk; itr++){
      if (*(((uint32_t*)main_double_block)+itr) != 0){
        double_ind_blk_nb++;
      } else {break; }
    }
    for (int itr = 0; itr < double_ind_blk_nb; itr++){
      //Not possible but used in case there is a bug somewhere
      if ((*(((uint32_t*)main_double_block)+itr)) == 0){
        PRINT_RED("SOMETHING IS WRONG");
        continue;
      }
      //Get indirect block
      char* blk_data_indir = disk_read_block(super->s_first_data_block+
            *(((uint32_t*)main_double_block)+itr), LOCK);
      if (blk_data_indir == 0){
        continue;
      }
      //Free elements of the indirect block
      for (int blk = 0; blk <nb_elts_blk; blk++){
        uint32_t blk_number = *(((uint32_t*)blk_data_indir)+blk);
        if (blk_number != 0){
          if (free_data_block(blk_number)<0){return -1;}
          freed_blocks++;
        }
      }
      unlock_cache(super->s_first_data_block+
            *(((uint32_t*)main_double_block)+itr));
      if (free_data_block(*(((uint32_t*)main_double_block)+itr))<0){
        return -1;
      }
      debug_print_inode("Free double indirect exit %d\n", freed_blocks);
      freed_blocks++;
    }
    unlock_cache(super->s_first_data_block+
        inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX]);
    if (free_data_block(inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX])<0){
      return -1;
    }
    freed_blocks++;
    debug_print_inode("Free double indirect %d\n", freed_blocks);
  }
  debug_print_inode("All elements were freed free %d,  total %d\n", freed_blocks, disk_blks);
  if (freed_blocks != disk_blks){
    return -1;
  }
  assert(freed_blocks == disk_blks);
  inode->i_size = 0;
  inode->i_blocks = 0;
  memset(inode->i_block, 0, 15*sizeof(uint32_t));
  return 0;
}

int add_data_block_inode(inode_t* inode){
  debug_print_inode("\033[0;36m[IN]Adding a data block to inode %d\n\033[0;0m", 
      get_inode_number(inode));
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
  uint32_t nb_elt_block =  root_file_system->block_size/sizeof(uint32_t);
  debug_print_inode("\033[0;36m[IN]inode->i_blocks = %d\n\033[0;0m", inode->i_blocks);
  if (inode->i_blocks<L_DIRECT){
    print_inode_no_arg("\033[0;36m[IN]i blocks< Ldirect\n\033[0;0m");
    inode->i_block[inode->i_blocks] = blk;
    inode->i_blocks++;
    return 0;
  }
  else if (inode->i_blocks>=L_DIRECT &&
     inode->i_blocks<L_ONE_INDIRECT){
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
                                  inode->i_block[INDIRECT_BLOCKS_INDEX], LOCK);
    if (indirect_block_data == 0){
      return -1;
    }
    *((uint32_t*)indirect_block_data+(
                  inode->i_blocks
                  -DIRECT_AND_INDEX_INDIRECT)%nb_elt_block) = blk;
    debug_print_inode("\033[0;36m[IN] indirect block Allocated = %d //final block %d\n\033[0;0m", 
          inode->i_blocks, blk);
    if (save_fs_block(indirect_block_data, 
        root_file_system->block_size,
        super->s_first_data_block+
        inode->i_block[INDIRECT_BLOCKS_INDEX],
        UNLOCK
        )<0){
      return -1;
    }
    inode->i_blocks++;
    return 0;
  }
  else if (inode->i_blocks>=L_ONE_INDIRECT &&
     inode->i_blocks<L_DOUBLE_INDIRECT){
    debug_print_inode("\033[0;36m[IN]Allocating a double indirect block = %d\n\033[0;0m", inode->i_blocks);
    bool double_written = false;
    char* main_double_block = 0;
    if (inode->i_blocks == L_ONE_INDIRECT){
      uint32_t double_direct_block = get_data_block();
      if (double_direct_block == 0){return -1;}
      inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX] =
        double_direct_block;
      main_double_block = disk_read_block(super->s_first_data_block+
        inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX], LOCK);
      if (main_double_block == 0){return -1;}
      memset(main_double_block, 0, root_file_system->block_size);
      inode->i_blocks++;
    }
    //If we are not allocating for the first time 
    //we read it directly
    uint32_t double_ind_blk_nb = 0;
    if (main_double_block == 0){
      main_double_block = disk_read_block(super->s_first_data_block+
            inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX], LOCK);
    }
    if (main_double_block == 0){return -1;}
    for (int itr = 0; itr < nb_elt_block;
         itr++){
      if (*(((uint32_t*)main_double_block)+itr) != 0){
        double_ind_blk_nb++;
      } else {
        break;
      }
    }
    uint32_t can_use_blocks = (inode->i_blocks -
           BASIC_DOUBLE_INDIRECT - double_ind_blk_nb)%
           (nb_elt_block);
    char* indirect_block_block = 0;
    if (can_use_blocks == 0) {
      if (double_ind_blk_nb != nb_elt_block){
        uint32_t indirect_blk = get_data_block();
        if (indirect_blk == 0){return -1;}
        *(((uint32_t*)main_double_block)+double_ind_blk_nb) = indirect_blk;
        inode->i_blocks++;
        double_ind_blk_nb++;
        double_written = true;
        indirect_block_block = disk_read_block(super->s_first_data_block+
          *(((uint32_t*)main_double_block)+double_ind_blk_nb-1), LOCK);
        memset(indirect_block_block, 0, root_file_system->block_size);
      }
      else{
        unlock_cache(super->s_first_data_block+
            inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX]);
        printf("File reached max size");
        return -1;
      }
    }
    if (indirect_block_block == 0){
      indirect_block_block = disk_read_block(super->s_first_data_block+
              *(((uint32_t*)main_double_block)+double_ind_blk_nb-1), LOCK);      
    }
    if (indirect_block_block == 0){
      return -1;
    }
    *(((uint32_t*)indirect_block_block)+can_use_blocks) = blk;
    if (double_written){
      if (save_fs_block(main_double_block, 
          root_file_system->block_size,
          super->s_first_data_block+
          inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX], UNLOCK)<0){
          return -1;
        }  
    }else{
      unlock_cache(super->s_first_data_block+
            inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX]);
    }
    debug_print_inode("\033[0;36m[IN] double indirect block Allocated = %d //final block %d\n\033[0;0m", 
          inode->i_blocks, blk);
    if (save_fs_block(indirect_block_block, 
        root_file_system->block_size,
        super->s_first_data_block+
        *(((uint32_t*)main_double_block)+
          double_ind_blk_nb-1), UNLOCK)<0){
          return -1;
    }
    inode->i_blocks++;
    return 0;
  }
  else{
    return -1;
  }
  return -1;
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
                      +block_nb, LOCK);
  if (block_data == 0){
    return -1;
  }
  if (write_dir_entry(block_data, location_b, entry)<0){
    return -1;
  }
  return save_fs_block(block_data,
          root_file_system->block_size, 
          super->s_first_data_block+
                          +block_nb, UNLOCK);
}

int add_inode_directory(inode_t* dir, 
  uint32_t inode_number,
  file_t type,
  char* name,
  size_t name_size){
  super_block* super = (super_block*) get_super_block();
  if (dir == 0 || super == 0 || inode_number == 0 || name_size <0){
    PRINT_RED("[IN][add_inode_directory failed] An element is null, dir inode add %p\n", dir);
    return -1;
  }
  debug_print_inode("\033[0;35m[IN]Adding an inode to a dir %d\n\033[0;0m", 
      get_inode_number(dir));
  debug_print_inode("[IN]dir->blocks = %d\n",dir->i_blocks);
  debug_print_inode("[IN]inode_number = %d\n",inode_number);
  debug_print_inode("[IN]type = %d\n",type);
  debug_print_inode("[IN]name = %s\n",name);
  debug_print_inode("[IN]name_size = %ld\n",name_size);  
  if (dir->i_mode != EXT2_S_IFDIR){
    PRINT_RED("%s\n", __func__); 
    PRINT_RED("inode is not a directory\n");
    return -1;
  }
  if (look_for_inode_dir(dir, name, name_size)>0){
    PRINT_RED("File already exits\n");
    return -1;
  }
  dir_entry dir_entry;
  dir_entry.inode_n = inode_number ;  
  dir_entry.name = name;  
  dir_entry.name_len = name_size;  
  dir_entry.file_type = type;  
  //size config
  dir_entry.rec_len = SIZE_DIR_NO_NAME
                  +dir_entry.name_len;
  if (dir_entry.rec_len%4 != 0){
    dir_entry.rec_len += 4-dir_entry.rec_len%4; 
  }
  //To improve later because if there is no space 
  //we will not be able to add a directory entry
  uint32_t file_size = dir_entry.rec_len;
  dir->i_size += file_size;
  debug_print_inode("[IN]Init rec_lec = %d\n",dir_entry.rec_len);  
  if (dir->i_blocks == 0){
    print_inode_no_arg("[IN]---Adding inode to dir block 0--\n");
    if (add_data_block_inode(dir)<0){
      return -1;
    }
    dir_entry.rec_len = root_file_system->block_size;
    debug_print_inode("[IN]Final rec_lec = %d\n",dir_entry.rec_len);  
    return save_dir_entry(&dir_entry, 0, dir->i_block[0]);
  }
  //search direct blocks
  else {
    //We look for elemnts in the present direct blocks
    for (int blk = 0; blk<dir->i_blocks; blk++){
      uint32_t pos = 0;
      uint32_t blk_num = super->s_first_data_block+dir->i_block[blk];
      char*  block_data = disk_read_block(blk_num, LOCK);
      if (block_data == 0){
        PRINT_RED("[IN]Failed to read op while adding dir\n");
        return -1;
      }
      dir_entry_basic* list_elt = 
              (dir_entry_basic*)block_data;
      char* limit = block_data+root_file_system->block_size;
      bool hit = 0;
      while (((char*)list_elt)<limit){
        //We look for a freed spot first
        uint32_t list_elt_size = SIZE_DIR_NO_NAME +
               list_elt->name_len;
        if (list_elt->name_len%4 != 0){
          list_elt_size += 4-list_elt->name_len%4; 
        }
        if (list_elt->file_type == EXT2_FT_FREE
          && SIZE_DIR_NO_NAME+list_elt->name_len <
           dir_entry.rec_len  
          ){
          dir_entry.rec_len = list_elt->rec_len;
          hit = true;
        }
        else if (dir_entry.rec_len < list_elt->rec_len
                                      -list_elt_size){
          uint32_t old_rec_len = list_elt->rec_len;
          //We empty a space for the new element
          list_elt->rec_len = 
              SIZE_DIR_NO_NAME + list_elt->name_len;
          if (list_elt->rec_len%4 != 0){
            list_elt->rec_len += 4-list_elt->rec_len%4; 
          }
          dir_entry.rec_len = old_rec_len - list_elt->rec_len;
          debug_print_inode("[IN]Final rec_lec = %d\n",dir_entry.rec_len);
          //print_dir_entry_obj(&dir_entry);
          pos += list_elt->rec_len;
          debug_print_inode("\033[0;35m[IN]Adding inode to dir blks num %d actual blk %d\n\033[0;0m", 
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
                dir->i_block[blk], UNLOCK);
        }
        uint32_t jump_by = list_elt->rec_len;
        pos+=jump_by;
        list_elt = (dir_entry_basic*)((char*)list_elt+jump_by);
      }
      unlock_cache(blk_num);
    }
    //WE see if we can add a new direct block
    if (dir->i_blocks<L_DIRECT){
      debug_print_inode("[IN]Could not find loc in direct blk, num blk %d\n",dir->i_blocks); 
      print_inode_no_arg("[IN]Creating a new direct blk\n"); 
      if (add_data_block_inode(dir)<0){
        return -1;
      }
      dir_entry.rec_len = root_file_system->block_size; 
      debug_print_inode("[IN]Final rec_lec = %d\n",dir_entry.rec_len);  
      return save_dir_entry(&dir_entry, 0, dir->i_block[dir->i_blocks-1]);
    }
  }
  PRINT_RED("No space left\n");
  return -1;
}

uint32_t look_for_inode_dir(inode_t* dir, 
        char* name,
        uint32_t name_len){
  if (dir == 0 || name == 0 || name_len == 0){
    return -1;
  }
  debug_print_inode("\033[0;35m[IN]Looking for inode with name %s, len = %d\n\033[0;0m",
         name, name_len);
  super_block* super = (super_block*) get_super_block();
  if (dir->i_mode != EXT2_S_IFDIR){
    PRINT_RED("%s\n", __func__); 
    PRINT_RED("inode is not a directory\n");
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
      uint32_t blk_num = super->s_first_data_block+
            dir->i_block[blk];
      char* block_data = disk_read_block(blk_num, LOCK);
      if (block_data == 0){
        PRINT_RED("[IN]Failed to read op while adding dir\n");
        return -1;
      }
      dir_entry_basic* list_elt = 
              (dir_entry_basic*)block_data;
      char* limit = block_data+root_file_system->block_size;
      while (((char*)list_elt)<limit){
          if (list_elt->file_type != EXT2_FT_FREE 
              && name_len == list_elt->name_len 
              &&memcmp((char*)list_elt+SIZE_DIR_NO_NAME, 
                name, name_len) == 0 ){
            debug_print_inode("\033[0;35m[IN]Inode with name %s, len = %d was found\n\033[0;0m",
                name, name_len);
            return list_elt->inode_n;
          }
          uint32_t jump_by = list_elt->rec_len;
          pos+=jump_by;
          list_elt = (dir_entry_basic*)((char*)list_elt+jump_by);
      }
      unlock_cache(blk_num);
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
    PRINT_RED("%s\n", __func__); 
    PRINT_RED("inode is not a directory\n");
    return -1;
  }
  //We make sure that the last element is 
  //is null of the string
  char temp_name[name_len];
  memcpy(temp_name, name, name_len);
  temp_name[name_len] = 0;
  if (name_len <= 2 &&(
     strcmp(temp_name, ".") == 0 ||
     strcmp(temp_name, "..") == 0)){
    PRINT_RED("Cannot delete . or ..\n");
    return -1;
  }
  if (dir->i_blocks == 0){
    print_inode_no_arg("[IN]---Directory has no blocks/inodes--\n");
    return 0;
  }
  //search direct blocks
  else {
    if (dir->i_blocks > L_DIRECT){
      assert(0); //Something went wrong we cannot reach this size
      return -1;
    }
    //We look for elements in the present direct blocks
    for (int blk = 0; blk<dir->i_blocks; blk++){
      uint32_t pos = 0;
      uint32_t block_num = super->s_first_data_block+
            dir->i_block[blk];
      char* block_data = disk_read_block(block_num, LOCK);
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
      // uint32_t size_previous = size_struct;
      char* limit = block_data+root_file_system->block_size;
      bool delete = false;
      while (((char*)list_elt)<limit){
        // print_dir_entry(list_elt);
        // size_previous = size_struct;
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
              && root_file_system->block_size
              == previous_elt->rec_len){
              delete = true;
            }
          }
          if (delete){
            debug_print_inode("\033[0;35m[IN]Freeing data block %d\n\033[0;0m", 
                  dir->i_block[blk]);
            //If the block is on the edge we free it otherwise
            //We just set it variables to zero
            if (blk == dir->i_blocks - 1){
              unlock_cache(block_num);
              if (free_data_block(dir->i_block[blk])<0){
                  return -1;
                }
              dir->i_blocks--;
              dir->i_block[blk] = 0;
              return 0;
            }
            else{
              list_elt->file_type = EXT2_FT_FREE;
              list_elt->name_len = 0;
              list_elt->rec_len = root_file_system->block_size;
            }
          }
          dir->i_size -= size_struct;
          return save_fs_block(block_data,
            root_file_system->block_size, 
            super->s_first_data_block+
                dir->i_block[blk], UNLOCK);
        }
        uint32_t jump_by = list_elt->rec_len;
        pos+=jump_by;
        previous_elt = list_elt;
        list_elt = (dir_entry_basic*)((char*)list_elt+jump_by);
      }
      unlock_cache(block_num);
    }
  }
  //File does not exist
  return -1;
}

uint32_t get_data_block(){
  print_fs_no_arg("\033[0;32m[IN]Getting data block\n\033[0;m");
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (desc_table == 0 || super == 0 
        || super->s_free_blocks_count == 0
        || desc_table->bg_free_blocks_count == 0){
    return 0;
  }
  uint32_t block_number = 0;
  for (int blk_iter = desc_table->bg_block_bitmap;
      blk_iter< desc_table->bg_inode_bitmap; blk_iter++){
    char* block_data_bitmap = disk_read_block(blk_iter, LOCK);
    if (block_data_bitmap == 0){
      PRINT_RED("Failed to read op while alloc inode");
      return 0;
    }
    int free_block = alloc_bit_bitmap(block_data_bitmap);
    if (free_block != -1){
      debug_print_inode("[IN]Found data block num %d in relative bitmap %d\n", 
          free_block, blk_iter);
      *(block_data_bitmap+free_block/8) |= (1<<(free_block%8));
      if (save_fs_block(block_data_bitmap,
           root_file_system->block_size, 
           blk_iter, UNLOCK) < 0){
        PRINT_RED("[IN]Failed to save data bitmap changes");
        return 0;
      }
      block_number = super->s_first_data_block +
                    root_file_system->block_size*8*(blk_iter-desc_table->bg_block_bitmap)+ 
                    (uint32_t) free_block;
      break;
    }else{
      unlock_cache(blk_iter);
    }
  }
  if (block_number == 0){
    return 0;
  }
  char* block_data = disk_read_block(block_number,LOCK);
  if (block_data == 0){
    return 0;
  }
  memset(block_data, 0, 
    root_file_system->block_size);
  if (save_fs_block(block_data, 
    root_file_system->block_size,
    block_number,
    UNLOCK)<0){
    return -1;
  }
  super->s_free_blocks_count--;
  desc_table->bg_free_blocks_count--;
  save_super_block();
  save_blk_desc_table();
  debug_print_inode("\033[0;35m[IN]End block location %d \n\033[0;0m", block_number);
  //We return the relative bitmap block number because it is simpler to use 
  return block_number - super->s_first_data_block;
}

int get_actual_blocks(inode_t* inode){
  print_inode_no_arg("\033[0;36m[IN]Getting actual relative blocks\n\033[0;0m");
  super_block* super = (super_block*) get_super_block();
  if (super == 0){
    return -1;
  }
  if (inode->i_blocks<=L_DIRECT){
    print_inode_no_arg("\033[0;36m[IN]i blocks< Ldirect\n\033[0;0m");
    return inode->i_blocks;
  }
  else if (inode->i_blocks>L_DIRECT &&
     inode->i_blocks<=L_ONE_INDIRECT){
    return inode->i_blocks-1;
  }
  else if (inode->i_blocks>L_ONE_INDIRECT &&
     inode->i_blocks<=L_DOUBLE_INDIRECT){
    uint32_t double_ind_blk_nb = 0;
    char* main_double_block = disk_read_block(super->s_first_data_block+
          inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX], LOCK);
    if (main_double_block == 0){return -1;}
    for (int itr = 0; itr < root_file_system->block_size/sizeof(uint32_t);
         itr++){
      if (*(((uint32_t*)main_double_block)+itr) != 0){
        double_ind_blk_nb++;
      } else {
        break;
      }
    }
    unlock_cache(super->s_first_data_block+
          inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX]);
    //-1 is for the doule indirect blocks and single indirect block;
    return inode->i_blocks-double_ind_blk_nb-1-1;
  }
  return -1;
}

char* get_inode_relative_block(inode_t* inode, 
    uint32_t relative_block, disk_get op){
  debug_print_inode("\033[0;35m[IN]Trying to inode's relative block num %d \n\033[0;0m", 
      relative_block);
  super_block* super = (super_block*) get_super_block();
  if (inode == 0 || super == 0){
    return 0;
  }
  uint32_t block_number = 0;
  if (relative_block<REL_NB_DIRECT){
    block_number = inode->i_block[relative_block];
  }
  else if (relative_block>=REL_NB_DIRECT && 
      relative_block<REL_NB_INDIRECT+REL_NB_DIRECT){
    uint32_t rel_indirect = relative_block-REL_NB_DIRECT;
    char* indirect_block_data = 
      disk_read_block(super->s_first_data_block+
          inode->i_block[INDIRECT_BLOCKS_INDEX], LOCK);
    if (indirect_block_data == 0){
      return 0;
    }
    block_number = *(((uint32_t*)indirect_block_data)+rel_indirect);
    unlock_cache(super->s_first_data_block+
          inode->i_block[INDIRECT_BLOCKS_INDEX]);
  }
  else if (relative_block>= REL_NB_INDIRECT + REL_NB_DIRECT && 
      relative_block<REL_NB_DOUB_INDIRECT+REL_NB_INDIRECT+REL_NB_DIRECT){
    uint32_t double_indirect = relative_block - 
                                (REL_NB_INDIRECT + REL_NB_DIRECT);
    uint32_t indirect_block = double_indirect / 
            (root_file_system->block_size/sizeof(uint32_t)); 
    uint32_t indirect_index = double_indirect %
            (root_file_system->block_size/sizeof(uint32_t)); 
    char* double_block_data = 
      disk_read_block(super->s_first_data_block+
          inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX], LOCK);
    if (double_block_data == 0){
      return 0;
    }
    char* indirect_block_data = 
      disk_read_block(super->s_first_data_block+
          *(((uint32_t*)double_block_data)
          +indirect_block), LOCK);
    if (indirect_block_data == 0){
      return 0;
    }
    unlock_cache(super->s_first_data_block+
          *(((uint32_t*)double_block_data)));
    unlock_cache(super->s_first_data_block+
          inode->i_block[DOUBLE_DIRECT_BLOCKS_INDEX]);
    block_number = 
        *(((uint32_t*)indirect_block_data)+indirect_index);
  }else{
    printf("block number is too big");
    return 0;
  }
  if (block_number != 0){
    block_number += super->s_first_data_block;
    char* block_data = 
        disk_read_block(block_number, LOCK);
    if (block_data == 0){
      return 0;
    }
    if (op == WRITE_OP){
      set_dirty_block(block_number);
    }
    return block_data;
  }
  else{
    debug_print_inode("[IN]Relative block %d is to big \n", relative_block);
    return 0;
  }
  return 0;
}

//0 is reserved not used block use for error handeling
int free_data_block(uint32_t data_block){
  debug_print_inode("\033[0;35m[IN]Trying to free a data blk %d \n\033[0;35m", 
      data_block);
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
  char* data_bitmap = disk_read_block(block_bitmap, LOCK);
  if (data_bitmap == 0){
    return -1;
  }
  //We look for the block and the location 
  //in the block we use the address
  *(data_bitmap+(data_block%(root_file_system->block_size*8))/8) &= 
              0xff - (1<<(data_block%8));
  super->s_free_blocks_count++;
  desc_table->bg_free_blocks_count++;
  save_super_block();
  save_blk_desc_table();
  if (save_fs_block(data_bitmap,
           root_file_system->block_size, 
           block_bitmap, UNLOCK)<0){
    return -1;
  }
  return 0;
}

void print_dir_list(inode_t* dir, bool verbose){
  print_inode_no_arg("------Printing dir list------\n");
  super_block* super = (super_block*) get_super_block();
  if (dir == 0){
    PRINT_RED("Inode is null\n");
    return ;
  }
  if (dir->i_mode != EXT2_S_IFDIR){
    PRINT_RED("Inode is not a dir\n");
    return;
  }
  debug_print_inode("print dir list dir->i_blocks = %d\n", dir->i_blocks);
  if (dir->i_blocks == 0){
      printf("Dir is empty\n");
  } 
  for (int blk = 0; blk<dir->i_blocks; blk++){
    uint32_t block_nb = super->s_first_data_block+dir->i_block[blk];
    char* block_data = disk_read_block(block_nb, LOCK);
    if (block_data == 0){
      PRINT_RED("Failed to read op while printing dir");
      return ;
    }
    dir_entry_basic* list_elt = 
          (dir_entry_basic*)block_data;
    char* limit = block_data+root_file_system->block_size;
    // int i = 0;
    while (((char*)list_elt)<limit){
      if (list_elt->file_type != EXT2_FT_FREE){
        if (verbose){
          printf("------------\n");
          printf("inode_n %d\n",list_elt->inode_n);
          printf("rec_len %d\n",list_elt->rec_len);
          printf("name_len :%d\n",list_elt->name_len);
          printf("file_type :%d\n",list_elt->file_type);
          printf("block nb :%d\n",blk);
        }
        char filename[list_elt->name_len + 1];
        memcpy(filename, (char*)list_elt + sizeof(dir_entry_basic),
             list_elt->name_len);
        filename[list_elt->name_len] = '\0';
        if (list_elt->file_type == EXT2_FT_REG_FILE){
          printf("file_name = ");
        } else if (list_elt->file_type == EXT2_FT_DIR){
          printf("dir_name = ");
        }
        else if (list_elt->file_type == EXT2_FT_CHRDEV){
          printf("chdev_name = ");
        }
        printf("%s\n",filename);
      }
      uint32_t jump_by = list_elt->rec_len;
      list_elt = (dir_entry_basic*)((char*)list_elt+jump_by);
    }
    unlock_cache(block_nb);
  }
  print_inode_no_arg("------Printing dir list end------\n");
}

int getdents_i(inode_t* dir, struct dirent *dirp,
            unsigned int count){
  if (dirp == 0 || count  == 0 || dir == 0){
    return -1;
  }
  super_block* super = (super_block*) get_super_block();
  if (dir->i_mode != EXT2_S_IFDIR){
    PRINT_RED("Inode is not a dir\n");
    return -1;
  }
  debug_print_inode("print dir list dir->i_blocks = %d\n", dir->i_blocks);
  if (dir->i_blocks == 0){
      printf("Dir is empty\n");
  }
  int nb_read = 0;
  for (int blk = 0; blk<dir->i_blocks; blk++){
    uint32_t block_nb = super->s_first_data_block+dir->i_block[blk];
    char* block_data = disk_read_block(block_nb, LOCK);
    if (block_data == 0){
      PRINT_RED("Failed to read op while printing dir");
      return -1;
    }
    dir_entry_basic* list_elt = 
          (dir_entry_basic*)block_data;
    char* limit = block_data+root_file_system->block_size;
    uint64_t size_ent = 0;
    while (((char*)list_elt)<limit){
      if (list_elt->file_type != EXT2_FT_FREE){
          size_ent = sizeof(dirent_basic) +
               list_elt->name_len + 1;
          ALIGN_SIZE_8(size_ent);
          if (count < size_ent){
            break;
          }
          dirp->d_ino = list_elt->inode_n;
          // printf("size %ld\n", size_ent);
          dirp->d_off = ((uint64_t)list_elt - (uint64_t)block_data) + 
                        blk*EXT2_BLOCK_SIZE + list_elt->rec_len;
          dirp->d_reclen = size_ent;
          dirp->d_type = map_ext2_to_dt(list_elt->file_type);
          char filename[list_elt->name_len + 1];
          memcpy(filename, (char*)list_elt + sizeof(dir_entry_basic),
              list_elt->name_len);
          filename[list_elt->name_len] = '\0';  
          memcpy(dirp->d_name, filename,
                list_elt->name_len + 1);
          dirp = (struct dirent *)((uint64_t)dirp +
                   size_ent); 
          count -= size_ent;
          nb_read += size_ent;
      }
      uint32_t jump_by = list_elt->rec_len;
      list_elt = (dir_entry_basic*)((char*)list_elt+jump_by);
    }
    unlock_cache(block_nb);
  }
  print_inode_no_arg("------Dirent reach the end------\n");
  return nb_read + 1; //FIX THIS IS VERY BAD
}

int add_dot_directories(inode_t* dir, inode_t* previous_directory){
  print_inode_no_arg("------ADDING DOT DIRECTORIES------\n");
  if (dir == 0 || previous_directory == 0){
    print_inode_no_arg("add dot dirs failed because arg is null\n");
    return -1;
  }
  debug_print_inode("[IN] DOT DIR main = %d, prev dir %d\n", 
        get_inode_number(dir), get_inode_number(previous_directory));
  if (dir->i_mode != EXT2_S_IFDIR || previous_directory->i_mode != EXT2_S_IFDIR){
    print_inode_no_arg("add dot dirs failed because an inode is not a dir\n");
    return -1;
  }
  char dot[] =  ".";
  char doubledot[] =  "..";
  if (add_inode_directory(dir, get_inode_number(dir), 
                    EXT2_FT_DIR, dot, 1)<0){
    PRINT_RED("Failed Addin dir 1\n");
    return -1;
  }
  if (add_inode_directory(dir, get_inode_number(previous_directory), 
                    EXT2_FT_DIR, doubledot, 2)<0){
    PRINT_RED("Failed Addin dir 2\n");
    return -1;
  }
  print_inode_no_arg("------DOT DIRECTORIES WERE ADDED------\n");
  return 0;
}


void print_data_bitmap(){
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (desc_table == 0 || super == 0 ){
    return;
  }
  PRINT_GREEN("-----Data bit map----\n");
  for (int blk_iter = desc_table->bg_block_bitmap;
      blk_iter< desc_table->bg_inode_bitmap; blk_iter++){
    char* block_data_bitmap = disk_read_block(blk_iter, LOCK);
    if (block_data_bitmap == 0){
      PRINT_RED("Failed to read op while print bitmap");
      return;
    }
    for (int i = 0; i<root_file_system->block_size/sizeof(uint64_t); i++){
      char* iter_address = block_data_bitmap+i*sizeof(uint64_t);
      printf("Block num = %d, line = %d\n", 
            blk_iter-desc_table->bg_block_bitmap, i);
      printb(((uint64_t*)(iter_address)), sizeof(uint64_t));
      break;
    }
    unlock_cache(blk_iter); 
  }
}


void print_inode_bitmap(){
  block_group_descriptor* desc_table = get_desc_table();
  super_block* super = (super_block*) get_super_block();
  if (desc_table == 0 || super == 0 ){
    return;
  }
  PRINT_GREEN("----Inode bit map----\n");
  for (int blk_iter = desc_table->bg_inode_bitmap;
      blk_iter< desc_table->bg_inode_table; blk_iter++){
    char* block_data_bitmap = disk_read_block(blk_iter, LOCK);
    if (block_data_bitmap == 0){
      PRINT_RED("Failed to read op while print bitmap");
      return;
    }
    for (int i = 0; i<root_file_system->block_size/sizeof(uint64_t); i++){
      char* iter_address = block_data_bitmap+i*sizeof(uint64_t);
      printf("Inode num = %d, line = %d\n", 
            blk_iter-desc_table->bg_block_bitmap, i);
      printb(((uint64_t*)(iter_address)), sizeof(uint64_t));
      break;
    } 
    unlock_cache(blk_iter);
  }
}

