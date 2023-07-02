#include "test_drivers.h"
#include <stdio.h>


int disk_driver_test(void *arg) {
  print_v_disk_no_arg("\n--disk read 1 block 2--\n");
  
  // Prepare disk read operation 1
  disk_op op_read_1;
  op_read_1.blockNumber = 2;
  op_read_1.type = READ;
  op_read_1.data = malloc(BLOCK_SIZE);
  memset(op_read_1.data, 2, BLOCK_SIZE);
  
  // Perform disk read operation 1
  disk_dev->read_disk(&op_read_1);
  // print_block(op_read_1.data, BLOCK_SIZE);

  print_v_disk_no_arg("\n--disk write 1 block 2---\n");
  
  // Prepare disk write operation 1
  disk_op op_write_1;
  op_write_1.blockNumber = 2;
  op_write_1.type = WRITE;
  op_write_1.data = (unsigned char*)malloc(BLOCK_SIZE);
  memset(op_write_1.data, 0xFF, BLOCK_SIZE);
  // print_block(op_write_1.data, BLOCK_SIZE);
  
  // Perform disk write operation 1
  disk_dev->write_disk(&op_write_1);

  print_v_disk_no_arg("\n--disk read 2 block 2---\n");
  
  // Prepare disk read operation 2
  disk_op op_read_2;
  op_read_2.blockNumber = 2;
  op_read_2.type = READ;
  op_read_2.data = malloc(BLOCK_SIZE);
  memset(op_read_2.data, 1, BLOCK_SIZE);
  
  // Perform disk read operation 2
  disk_dev->read_disk(&op_read_2);
  
  // Compare data from read operations 1 and 2
  if (!memcmp(op_write_1.data, op_read_2.data, BLOCK_SIZE) == 0){
    return -1;
  };
  // print_block(op_read_2.data, BLOCK_SIZE);

  print_v_disk_no_arg("\n--disk write 2 block 2---\n");
  
  // Prepare disk write operation 2
  disk_op op_write_2;
  op_write_2.blockNumber = 2;
  op_write_2.type = WRITE;
  op_write_2.data = (unsigned char*)malloc(BLOCK_SIZE);
  memcpy(op_write_2.data, op_read_1.data, BLOCK_SIZE);
  // print_block(op_write_2.data, BLOCK_SIZE);
  
  // Perform disk write operation 2
  disk_dev->write_disk(&op_write_2);

  // Free dynamically allocated memory
  free(op_read_1.data);
  free(op_read_2.data);
  free(op_write_1.data);
  free(op_write_2.data);
  return 0;
}
