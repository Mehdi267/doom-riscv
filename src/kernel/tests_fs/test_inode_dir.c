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
  super_block* super = (super_block*) get_super_block();
  uint32_t free_data_block_count = super->s_free_blocks_count;
  uint32_t free_inode_count = super->s_free_inodes_count;
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
  uint32_t inodel_number_1 = get_inode_number(file1);             
  uint32_t inodel_number_2 = get_inode_number(file2);             
  uint32_t inodel_number_3 = get_inode_number(file3);
  uint32_t inodel_number_4 = get_inode_number(file4);             
  file1->i_mode = EXT2_S_IFREG;             
  file2->i_mode = EXT2_S_IFREG;             
  file3->i_mode = EXT2_S_IFREG;
  file4->i_mode = EXT2_S_IFREG;             
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
  
  print_dir_list(get_inode(EXT2_GOOD_OLD_FIRST_INO), 1);
  uint32_t inode_number4 = look_for_inode_dir(
    get_inode(EXT2_GOOD_OLD_FIRST_INO),
    "file--4",
    7);
  printf("looking for file 4, inode number found : %d\n",inode_number4);
  assert(inode_number4 == inodel_number_4);
  uint32_t inode_number3 = look_for_inode_dir(
    get_inode(EXT2_GOOD_OLD_FIRST_INO),
    "file--3",
    7);
  printf("looking for file 3, inode number found : %d\n",inode_number3);
  assert(inode_number3 == inodel_number_3);
  uint32_t inode_number1 = look_for_inode_dir(
    get_inode(EXT2_GOOD_OLD_FIRST_INO),
    "file1",
    5);
  printf("looking for file 1, inode number found : %d\n",inode_number1);
  assert(inode_number1 == inodel_number_1);
  uint32_t inode_number2 = look_for_inode_dir(
    get_inode(EXT2_GOOD_OLD_FIRST_INO),
    "file-2",
    6);
  printf("looking for file 2, inode number found : %d\n",inode_number2);
  assert(inode_number2 == inodel_number_2);
  uint32_t inode_number_no = look_for_inode_dir(
    get_inode(EXT2_GOOD_OLD_FIRST_INO),
    "file155",
    7);
  printf("looking file that does not exist, inode number found : %d\n",inode_number_no);
  assert(inode_number_no == 0);
  if (remove_inode_dir(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
    "file1",
    5)<0){
      printf("remove 1 failed \n");
      return -1;
  } else{
    printf("remove 1 success \n");
  }
  if (remove_inode_dir(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
    "file-2",
    6)<0){
      printf("remove e-2 failed \n");
      return -1;
  } else{
    printf("remove e-2 success \n");
  }
  if (remove_inode_dir(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
    "file--3",
    7)<0){
      printf("remove e--3 failed \n");
      return -1;
  } else{
    printf("remove e--3 success \n");
  }
  if (remove_inode_dir(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
    "file--4",
    7)<0){
      printf("remove e--4 failed \n");
      return -1;
  } else{
    printf("remove e--4 success \n");
  }
  if (free_inode(file1,inodel_number_1)<0){
    return -1;
  }
  if (free_inode(file2,inodel_number_2)<0){
    return -1;
  }
  if (free_inode(file3,inodel_number_3)<0){
    return -1;
  }
  if (free_inode(file4,inodel_number_4)<0){
    return -1;
  }
  assert(free_inode_count == 
        super->s_free_inodes_count);
  print_dir_list(get_inode(EXT2_GOOD_OLD_FIRST_INO), 1);
  assert(look_for_inode_dir(
    get_inode(EXT2_GOOD_OLD_FIRST_INO),
    "file1",
    5)==0);
  assert(free_data_block_count == 
        super->s_free_blocks_count);
  return 0;
}

int gdb_variable = 0;

int stress_test(){
  super_block* super = (super_block*) get_super_block();
  uint32_t free_data_block_count = super->s_free_blocks_count;
  uint32_t free_inode_count = super->s_free_inodes_count;
  uint32_t number_of_free_files = super->s_free_inodes_count < 1000 
        ? super->s_free_inodes_count : 1000;
  #define FILE_NAME_SIZE 32
  char filename[FILE_NAME_SIZE];
  uint32_t file_ids[number_of_free_files+1];
  for(int file_iter = 0; file_iter <number_of_free_files; file_iter++){
    gdb_variable++;
    printf("i = %d, max = %d\n", file_iter, number_of_free_files);
    inode_t* file = alloc_inode();
    file_ids[file_iter] = get_inode_number(file);
    if (put_inode(file, 
        get_inode_number(file),
        SAVE_INODE)<0){
          return -1;
    }
    // print_cache_details(root_file_system->inode_list);
  }
  PRINT_GREEN("Created files and i am now adding them to the directory\n");
  // print_cache_details(root_file_system->inode_list);
  for(int file_iter = 0; file_iter <number_of_free_files; file_iter++){
    sprintf(filename, "file%d", file_iter);
    uint32_t name_size = file_iter == 0 ? 5 : 4;
    uint32_t num = file_iter;
    while (num != 0){
      num = num / 10;
      name_size += 1;
    }
    if (add_inode_directory(get_inode(EXT2_GOOD_OLD_FIRST_INO), 
        file_ids[file_iter],
        EXT2_FT_REG_FILE,filename,
        name_size)<0){
          return -1;
      }
  }
  PRINT_GREEN("Created files and added them\n");
  for(int file_iter = 0; file_iter <number_of_free_files; file_iter++){
    uint32_t name_size = file_iter == 0 ? 5 : 4;
    uint32_t num = file_iter;
    while (num != 0){
      num = num / 10;
      name_size += 1;
    }
    sprintf(filename, "file%d", file_iter);
    assert(file_ids[file_iter] ==  look_for_inode_dir(
      get_inode(EXT2_GOOD_OLD_FIRST_INO),
      filename,
      name_size));
  }
  PRINT_GREEN("All files were located, deleting directories\n");
  for(int file_iter = 0; file_iter <number_of_free_files; file_iter++){
    uint32_t name_size = file_iter == 0 ? 5 : 4;
    uint32_t num = file_iter;
    while (num != 0){
      num = num / 10;
      name_size += 1;
    }
    sprintf(filename, "file%d", file_iter);
      if (remove_inode_dir(
        get_inode(EXT2_GOOD_OLD_FIRST_INO), 
      filename,
      name_size)<0){
      printf("remove file failed \n");
      return -1;
    }
  }
  PRINT_GREEN("All files were removed from the directories, freeing files\n");
  for(int file_iter = number_of_free_files - 1;
        file_iter >= 0; 
        file_iter--){
    // print_cache_details(root_file_system->inode_list);
    if (free_inode(NULL,file_ids[file_iter])<0){
      return -1;
    }
  }
  return 0;
  assert(free_inode_count == 
        super->s_free_inodes_count);
  assert(free_data_block_count == 
        super->s_free_blocks_count);
  return 0;
}


void test_ext2_fs(){
  if (basic_test()<0){
    PRINT_RED("Basic test failed\n");
  }else{
    PRINT_GREEN("Basic test passed\n");
  }
  print_cache_details(root_file_system->inode_list);
  PRINT_GREEN("###############################\n");
  if (stress_test()<0){
    PRINT_RED("Stress test failed\n");
  }
  PRINT_GREEN("Test inode exit\n");
}
