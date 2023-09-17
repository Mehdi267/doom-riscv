// See LICENSE for license details.
#include <stdlib.h>
#include <stdint.h>
#pragma once

//######################################################
//##   Disk operations functions ##
//######################################################

#define BLOCK_SIZE 512 

typedef enum operation_type {
  READ = 0,
  WRITE = 1,
} op_type;

typedef struct disk_operation{
  uint32_t blockNumber; //the disk sector that we will read from/write to
  op_type type; //Type of the disk operation(read or write)
  //The location in which we read data into
  //or the location of the data that will be 
  //written to the disk.
  char *data;
} disk_op;

typedef struct disk_device {
	int (*init)();
  int (*read_disk)(disk_op*);
  int (*write_disk)(disk_op*);
  uint32_t (*get_disk_size)();
  void (*print_disk_info)();
} disk_device_t;

/*
 * disk_dev global variable
 * This variable is useful to init the disk on the machine.
 */
extern disk_device_t *disk_dev;

/*
 * disk drivers
 */
extern disk_device_t virt_disk;

void register_disk(disk_device_t *dev);
