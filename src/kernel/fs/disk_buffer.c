#include "disk_buffer.h"
#include <stdbool.h>
#include <stdint.h>
#include "drivers/disk_device.h" //to do disk operations
#include "stdlib.h"
#include "string.h"
c_elt* global_cache_buf = 0; 

unsigned char* read_block_c(uint32_t disk_block_number){
  c_elt* cache_elt = look_up_c_elt(disk_block_number);
  if (cache_elt == 0){
    cache_elt = fetch_block(disk_block_number);
    if (cache_elt == 0){
      return 0;
    }
  }
  cache_elt->usage++;
  return cache_elt->data;
}

int write_block(uint32_t disk_block_number, 
                  char* data,
                  size_t data_length,
                  write_type type
                  ){
  if (data == 0 || data_length > BLOCK_SIZE){
    return -1;
  }
  c_elt* cache_elt = look_up_c_elt(disk_block_number);
  if (cache_elt == 0){
    cache_elt = fetch_block(disk_block_number);
    if (cache_elt == 0){
      return 0;
    }
  }
  memcpy(cache_elt->data, data, data_length);
  cache_elt->usage++;
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


c_elt* look_up_c_elt(uint32_t disk_block_number){
  if (global_cache_buf == 0){
    return 0;
  }
  c_elt* global_cache_iter = global_cache_buf;
  while(global_cache_iter){
    if (global_cache_iter->blockNumber == disk_block_number){
      return global_cache_iter;
    }
  }

  return 0;
}

c_elt* fetch_block(uint32_t disk_block_number){
  c_elt* elt = (c_elt*)malloc(sizeof(c_elt));
  if (elt == 0){
    return NULL;
  }
  elt->blockNumber = disk_block_number;
  disk_op disk_fetch;
  disk_fetch.blockNumber = disk_block_number;
  disk_fetch.type = READ;  
  disk_fetch.data = (unsigned char*) elt->data;
  // Perform disk read operation on the block taht contains the mbr
  elt->disk_res = 1;
  disk_dev->read_disk(&disk_fetch);
  elt->disk_res = 0;
  if(global_cache_buf == 0){
    global_cache_buf = elt;
    global_cache_buf->next_c = NULL;
  }else{
    elt->next_c = global_cache_buf;
    global_cache_buf = elt;
  }
  return elt;
}

int sync_elt(c_elt* cache_elt){
  if(cache_elt == 0){
    return -1;
  }
  disk_op disk_wr;
  disk_wr.blockNumber = cache_elt->blockNumber;
  disk_wr.type = WRITE;  
  disk_wr.data = (unsigned char*) cache_elt->data;
  // Perform disk read operation on the block taht contains the mbr
  cache_elt->disk_res = 1;
  if (disk_dev->write_disk(&disk_wr)<0){
    return -1;
  }
  cache_elt->disk_res = 0;
  cache_elt->dirty = 0;
  return -1;
}

int sync(){
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
  }
  return res;
}