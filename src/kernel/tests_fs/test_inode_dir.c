/****************************************
Tests inode allocation,
inode recuperation
inode save
inode management(hash table, 
    list operations,
    free list)
Directory creation
Adding inodes to directory
Checking that inodes exist
Removing inodes and then Adding other inodes
Stress tests for all of the above 
*****************************************/
#include "../fs/mbr.h"
#include "../fs/ext2.h"
#include "../fs/disk_buffer.h"
#include "../fs/inode.h"
#include "../fs/fs.h"
#include "assert.h"
#include <stdint.h>
#include <stdio.h>

int basic_test(){
  inode_t* file1 = alloc_inode();
  if (file1 == 0){
    printf("alloc file1 failed\n");
    return -1;
  }
  inode_t* file2 = alloc_inode();
  if (file2 == 0){
    printf("alloc file2 failed\n");
    return -1;
  }
  inode_t* file3 = alloc_inode();
  if (file3 == 0){
    printf("alloc file3 failed\n");
    return -1;
  }
  inode_t* file4 = alloc_inode();
  if (file4 == 0){
    printf("alloc file4 failed\n");
    return -1;
  }
  // uint32_t inodel_number_1 = get_inode_number(file1);             
  // uint32_t inodel_number_2 = get_inode_number(file2);             
  // uint32_t inodel_number_3 = get_inode_number(file3);
  // uint32_t inodel_number_4 = get_inode_number(file4);             
  file1->i_mode = EXT2_S_IFREG;             
  file2->i_mode = EXT2_S_IFREG;             
  file3->i_mode = EXT2_S_IFREG;
  file4->i_mode = EXT2_S_IFREG;             
  super_block* super = (super_block*) get_super_block();
  //uint32_t free_data_block_count = super->s_free_inodes_count;
  if (super == 0){
    return -1;
  }
  
  if (put_inode(file1, 
    get_inode_number(file1), 
        SAVE_INODE)<0){
          return -1;
  }
  if (put_inode(file2, 
    get_inode_number(file2), 
        SAVE_INODE)<0){
          return -1;
  }
  if (put_inode(file3, 
    get_inode_number(file3), 
        SAVE_INODE)<0){
          return -1;
  }
  if (put_inode(file4, 
    get_inode_number(file4), 
        SAVE_INODE)<0){
          return -1;
  }
  
  if (add_inode_directory(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
    get_inode_number(file1),
    EXT2_FT_REG_FILE,
    "file1",
    5)<0){
      return -1;
  }
  if (add_inode_directory(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
    get_inode_number(file2),
    EXT2_FT_REG_FILE,
    "file-2",
    6)<0){
      return -1;
  }
  if (add_inode_directory(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
    get_inode_number(file3),
    EXT2_FT_REG_FILE,
    "file--3",
    7)<0){
      return -1;
  }
  if (add_inode_directory(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
    get_inode_number(file4),
    EXT2_FT_REG_FILE,
    "file--4",
    7)<0){
      return -1;
  }
  
  // print_dir_list(get_inode(EXT2_GOOD_OLD_FIRST_INO));
  // uint32_t inode_number4 = look_for_inode_dir(
  //   get_inode(EXT2_GOOD_OLD_FIRST_INO),
  //   "file--4",
  //   7);
  // printf("looking for file 4, inode number found : %d\n",inode_number4);
  // assert(inode_number4 == inodel_number_4);
  // uint32_t inode_number3 = look_for_inode_dir(
  //   get_inode(EXT2_GOOD_OLD_FIRST_INO),
  //   "file--3",
  //   7);
  // printf("looking for file 3, inode number found : %d\n",inode_number3);
  // assert(inode_number3 == inodel_number_3);
  // uint32_t inode_number1 = look_for_inode_dir(
  //   get_inode(EXT2_GOOD_OLD_FIRST_INO),
  //   "file1",
  //   5);
  // printf("looking for file 1, inode number found : %d\n",inode_number1);
  // assert(inode_number1 == inodel_number_1);
  // uint32_t inode_number2 = look_for_inode_dir(
  //   get_inode(EXT2_GOOD_OLD_FIRST_INO),
  //   "file-2",
  //   6);
  // printf("looking for file 2, inode number found : %d\n",inode_number2);
  // assert(inode_number2 == inodel_number_2);
  // uint32_t inode_number_no = look_for_inode_dir(
  //   get_inode(EXT2_GOOD_OLD_FIRST_INO),
  //   "file155",
  //   7);
  // printf("looking file that does not exist, inode number found : %d\n",inode_number1);
  // assert(inode_number_no == 0);
  
  // if (remove_inode_dir(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
  //   "file1",
  //   5)<0){
  //     printf("remove 1 failed \n");
  //     return -1;
  // } else{
  //   printf("remove 1 success \n");
  // }
  // if (remove_inode_dir(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
  //   "file-2",
  //   6)<0){
  //     printf("remove e-2 failed \n");
  //     return -1;
  // } else{
  //   printf("remove e-2 success \n");
  // }
  // if (remove_inode_dir(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
  //   "file--3",
  //   7)<0){
  //     printf("remove e--3 failed \n");
  //     return -1;
  // } else{
  //   printf("remove e--3 success \n");
  // }
  // if (remove_inode_dir(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
  //   "file--4",
  //   7)<0){
  //     printf("remove e--4 failed \n");
  //     return -1;
  // } else{
  //   printf("remove e--4 success \n");
  // }
  // print_dir_list(get_inode(EXT2_GOOD_OLD_FIRST_INO));
  // assert(look_for_inode_dir(
  //   get_inode(EXT2_GOOD_OLD_FIRST_INO),
  //   "file1",
  //   5)==0);
  // assert(free_data_block_count == 
  //       super->s_free_inodes_count);
  return 0;
}


void test_ext2_fs(){
  if (basic_test()<0){
    PRINT_RED("Basic test failed\n");
  }
  PRINT_GREEN("Test inode passed\n");
}
