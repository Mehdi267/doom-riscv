#include "disk_buffer.h"
#include <stdbool.h>
#include <stdint.h>
#include "stdio.h"
#include "drivers/disk_device.h" //to do disk operations
#include "stdlib.h"
#include "string.h"
#include "fs.h"
#include "mbr.h"
#include "logger.h"

c_elt* global_cache_buf = 0; 
uint32_t nb_elements_cache = 0; 
uint32_t locked_files = 0; 
#define MAX_CACHE_SIZE 30

char* read_block_c(uint32_t disk_block_number, cache_op cah_op){
  debug_print_v_fs("[df]Reading relative disk block %d\n", disk_block_number);
  c_elt* cache_elt = look_up_c_elt(disk_block_number);
  if (cache_elt == 0){
    cache_elt = fetch_block(disk_block_number);
    if (cache_elt == 0){
      return 0; 
    }
  }
  if (cah_op == LOCK){
    cache_elt->usage++;
    if (cache_elt->usage == 1){
      locked_files++;
    }
  }
  return cache_elt->data;
}

int write_block(uint32_t disk_block_number, 
                  char* data,
                  size_t data_length,
                  write_type type,
                  cache_op cah_op
                  ){
  debug_print_v_fs("[df]Write block called on block %d\n", disk_block_number);
  if (data == 0 || data_length > 
            root_file_system->block_size){
    return -1;
  }
  c_elt* cache_elt = look_up_c_elt(disk_block_number);
  if (cache_elt == 0){
    cache_elt = fetch_block(disk_block_number);
    if (cache_elt == 0){
      return 0;
    }
  }
  // printf("-----disk write----\n");
  // printf("Data add = %p\n", data);
  // printf("cache Data add = %p\n", cache_elt->data);
  // printb(data, 8);
  // printb(cache_elt->data, 8);
  memcpy(cache_elt->data, data, data_length);
  // printb(cache_elt->data, 8);
  //If the user want to lock in the save
  //operation we don't don't anything
  if (cah_op == UNLOCK){
    cache_elt->usage--;
    if (cache_elt->usage == 0){
      locked_files--;
    }
  }
  if (type == WRITE_BACK){
    cache_elt->dirty = 1;
  }
  else if (type == WRITE_THROUGH){
    cache_elt->dirty = 1;
    if (sync_elt(cache_elt)<0){
      return -1;
    }
  }
  return 0;
}

int unlock_cache(uint32_t disk_block_number){
  c_elt* cache_elt = look_up_c_elt(disk_block_number);
  if (cache_elt == 0){
    return -1;
  }
  if (cache_elt->usage > 0){
    cache_elt->usage--;
    if (cache_elt->usage == 0){
      locked_files--;
    }
  }
  return 0;
}

int unlock_using_address(char* block){
  c_elt* cache_elt = look_up_c_elt_add(block);
  if (cache_elt == 0){
    return -1;
  }
  if (cache_elt->usage > 0){
    cache_elt->usage--;
    if (cache_elt->usage == 0){
      locked_files--;
    }
  }
  return 0;
}

c_elt* look_up_c_elt(uint32_t disk_block_number){
  if (global_cache_buf == 0){
    return 0;
  }
  c_elt* global_cache_iter = global_cache_buf;
  while(global_cache_iter != 0){
    if (global_cache_iter->blockNumber == disk_block_number){
      debug_print_v_fs("[df]element blk %d was found in cache\n",
             disk_block_number);
      return global_cache_iter;
    }
    global_cache_iter = global_cache_iter->next_c;
  }
  return 0;
}

c_elt* look_up_c_elt_add(char* add){
  if (global_cache_buf == 0 || add == 0){
    return 0;
  }
  c_elt* global_cache_iter = global_cache_buf;
  while(global_cache_iter != 0){
    if (global_cache_iter->data == add){
      debug_print_v_fs("[df]element blk %d was found in cache\n",
             global_cache_iter->blockNumber);
      return global_cache_iter;
    }
    global_cache_iter = global_cache_iter->next_c;
  }
  return 0;
}

int set_dirty_block(uint32_t disk_block_number){
  if (global_cache_buf == 0){
    return 0;
  }
  c_elt* global_cache_iter = global_cache_buf;
  while(global_cache_iter != 0){
    if (global_cache_iter->blockNumber == disk_block_number){
      debug_print_v_fs("[df]element blk %d was set to dirty \n",
             disk_block_number);
      global_cache_iter->dirty = true;
      return 0;
    }
    global_cache_iter = global_cache_iter->next_c;
  }
  debug_print_v_fs("[df]Set dirty block %d not found \n",
             disk_block_number);
  return -1;
}

void check_if_cache_is_full(){
  // printLinkedList(global_cache_buf);
  if (nb_elements_cache == MAX_CACHE_SIZE){
    free_cache_list();
  }
}

c_elt* fetch_block(uint32_t disk_block_number){
  check_if_cache_is_full();
  c_elt* elt = (c_elt*)malloc(sizeof(c_elt));
  if (elt == 0){
    return NULL;
  }
  if (root_file_system != 0 && disk_block_number*
      root_file_system->block_size/
      BLOCK_SIZE > get_partition_size(
      root_file_system->partition)){
    free(elt);
    return 0;
  }
  debug_print_v_fs("[df]###fetching relative block %d from disk \n", 
              disk_block_number);
  elt->blockNumber = disk_block_number;
  disk_op disk_fetch;
  int blk_ratio = root_file_system->block_size/BLOCK_SIZE;
  for (int i = 0; i<blk_ratio; i++){
    disk_fetch.blockNumber = global_mbr->partitionTable[root_file_system->partition]
      .startLBA+disk_block_number*blk_ratio+i;
    debug_print_v_fs("[df]Reading disk block %d from disk \n", 
              disk_fetch.blockNumber);
    disk_fetch.type = READ;  
    disk_fetch.data = elt->data + i*BLOCK_SIZE;
    elt->disk_res = 1;
    disk_dev->read_disk(&disk_fetch);
    elt->disk_res = 0;
  }
  // Perform disk read operation on the block taht contains the mbr
  elt->next_c = NULL;
  elt->before_c = NULL;
  if(global_cache_buf == 0){
    global_cache_buf = elt;
  }else{
    elt->next_c = global_cache_buf;
    global_cache_buf->before_c = elt;
    global_cache_buf = elt;
  }
  nb_elements_cache++;
  return elt;
}

int sync_elt(c_elt* cache_elt){
  if(cache_elt == 0){
    return -1;
  }
  print_fs_no_arg("[df]sync_elt was called on cache\n");
  debug_print_v_fs(" buffer containing block number = %d\n", 
            cache_elt->blockNumber);
  disk_op disk_wr;
  int blk_ratio = root_file_system->block_size/BLOCK_SIZE;
  for (int blk = 0; blk<blk_ratio; blk++){
    disk_wr.blockNumber = global_mbr->partitionTable[root_file_system->partition]
            .startLBA+cache_elt->blockNumber*blk_ratio+blk;
    debug_print_v_fs("[df]Saving disk block %d into disk\n", 
              disk_wr.blockNumber);
    disk_wr.type = WRITE;  
    disk_wr.data = cache_elt->data + blk*BLOCK_SIZE;
    if (disk_dev->write_disk(&disk_wr)<0){
      return -1;
    }
    cache_elt->disk_res = 0;
  }
  cache_elt->dirty = 0;
  return 0;
}

int sync(){
  print_fs_no_arg("[df]sync method was called\n");
  if (global_cache_buf ==0){
    return 0;
  }
  int res = 0;
  c_elt* buf_iter = global_cache_buf;
  while (buf_iter != NULL){
    if (buf_iter->dirty == true){
      if (sync_elt(buf_iter)<0){
        res = -1;
      }
    }
    buf_iter = buf_iter->next_c;
  }
  return res;
}

int free_cache_list(){
  print_fs_no_arg("[df]free_cache_list method was called\n");
  if (global_cache_buf == 0){
    return 0;
  }
  c_elt* buf_iter = global_cache_buf;
  c_elt* buf_iter_next = global_cache_buf->next_c;
  c_elt* buf_iter_before = global_cache_buf->before_c;
  while (buf_iter != NULL){
    buf_iter_next = buf_iter->next_c;
    buf_iter_before = buf_iter->before_c;
    if (buf_iter->usage == 0){
      if (buf_iter->dirty){
        if (sync_elt(buf_iter)<0){
          return -1;
        }  
      }
      if (buf_iter == global_cache_buf 
            && buf_iter_next == NULL 
            && buf_iter_before == NULL){
        global_cache_buf = NULL;
      } else{
        if (buf_iter_next != NULL){
          buf_iter_next->before_c = buf_iter_before;
        }
        if (buf_iter_before != NULL){
          buf_iter_before->next_c = buf_iter_next;
        }
        if (buf_iter == global_cache_buf){
          global_cache_buf = buf_iter_next;
        }
      }
      free(buf_iter);
      buf_iter = buf_iter_next;
      nb_elements_cache--;
    }
    else{
      buf_iter = buf_iter_next;
    }
  }
  return 0;
}

int save_fs_block(char* data,
                  uint32_t data_size,
                  uint32_t relative_b_nb,
                  cache_op cah_op
                  ){
  if (data == 0 || data_size > root_file_system->block_size){
    printf("disk save failed");
    return -1;
  }
  debug_print_v_fs("[df]Saving fs block %d into memory \n", relative_b_nb);
  if (write_block(relative_b_nb, data,
      data_size, WRITE_THROUGH, cah_op)<0){
    printf("A save operation failed");
    return -1;
  }
  return 0;
} 



char* disk_read_block(uint32_t relative_b_n, cache_op cah_op){
  return read_block_c(relative_b_n, cah_op);
}

void printLinkedList(c_elt* elt) {
  while (elt != NULL) {
    printf("Disk Res: %u\n", elt->disk_res);
    printf("Block Number: %u\n", elt->blockNumber);
    printf("Dirty: %s\n", elt->dirty ? "true" : "false");
    printf("Usage: %u\n", elt->usage);
    printf("Data: %p\n", elt->data);
    printf("Next: %p\n", elt->next_c);
    printf("\n");
    elt = elt->next_c;
  }
}
