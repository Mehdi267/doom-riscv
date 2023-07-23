#include "drivers/disk_device.h" //to do disk operations
#include "mbr.h" 
#include "fs.h"
#include "../memory/frame_dist.h" // used to allocate the block
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "stdio.h"
#include "string.h"
#include "../logger.h"
#include "disk_buffer.h"

//This value represents the global mdr that 
//will be used the in the program
mbr_t *global_mbr = 0;

int find_mbr(){
  //Read the fist disk 
  //block which contains the mbr
  disk_op disk_operation_mbr;
  disk_operation_mbr.blockNumber = MBR_BLOCK;
  disk_operation_mbr.type = READ;  
  uint16_t data[256];
  disk_operation_mbr.data = (char*) data;
  // Perform disk read operation on the block taht contains the mbr
  disk_dev->read_disk(&disk_operation_mbr);
  if (data[255] == MBR_SIGNATURE) {
    //Mbr was found we can now access partitions
    global_mbr = (mbr_t*) malloc(sizeof(mbr_t));
    memcpy(global_mbr,(void*)data,512);
    return 0;
  }
  else{
    return -1;
  }
}

int save_global_mbr(){
  print_fs_no_arg("[df]save_global_mbr method was called\n");
  if (global_mbr == 0){
    print_fs_no_arg("could not save mbr\n");
    return -1;
  }
  disk_op disk_operation_mbr;
  disk_operation_mbr.blockNumber = MBR_BLOCK;
  disk_operation_mbr.type = WRITE;
  disk_operation_mbr.data = (char*) global_mbr;
  // Perform disk read operation on the block that contains the mbr
  if (disk_dev->write_disk(&disk_operation_mbr)<0){
    print_fs_no_arg("mbr was not saved succefully\n");
    return -1;
  }
  PRINT_GREEN("mbr saved succefully\n");
  print_fs_no_arg("mbr saved succefully\n");
  return 0;
}

int set_up_mbr(){ 
  print_fs_no_arg("[df]set_up_mbr method was called\n");
  if (global_mbr != 0){
    release_frame(global_mbr);
  }
  //We allocate data for the global mbr struct
  global_mbr = (mbr_t*) malloc(sizeof(mbr_t));
  if (global_mbr == 0){
    return -1;
  }
  sync();
  free_cache_list();
  memset(global_mbr, 0, 512);
  global_mbr->signature = MBR_SIGNATURE;
  if (setup_test_partition(EXT2_PARTITION) < 0){
    return -1;
  }
  print_fs_no_arg("[df]set_up_mbr method was finished\n");
  return save_global_mbr();
}

int clear_partition_space(uint8_t partition_number){
  if(global_mbr ==0){
    return -1;
  }
  disk_op* disk_wr = (disk_op*)malloc(sizeof(disk_op));
  char data[BLOCK_SIZE];
  memset(data, 0, BLOCK_SIZE);
  for (int blk = global_mbr->partitionTable[0].startLBA; 
        blk<global_mbr->partitionTable[0].sizeLBA +
            global_mbr->partitionTable[0].startLBA; blk++){
    disk_wr->blockNumber = blk;
    disk_wr->type = WRITE;  
    disk_wr->data = data;
    if (disk_dev->write_disk(disk_wr)<0){
      return -1;
    }
  }
  free(disk_wr);
  return 0;
}

int setup_test_partition(uint8_t partition_type){
  if(global_mbr ==0){
    return -1;
  }
  global_mbr->partitionTable[0].status = ACTIVE_PARTITION;
  global_mbr->partitionTable[0].type = partition_type;//test partition
  global_mbr->partitionTable[0].startLBA = 2;
  global_mbr->partitionTable[0].sizeLBA = TEST_EXT2_PARTITION_SIZE;
  clear_partition_space(0);
  if (partition_type == EXT2_PARTITION){
      configure_ext2_file_system(0);
  }
  return 0;
}

void print_occ_places(uint32_t* occu_places){
  if (occu_places==0){
    return;
  }
  printf("\n-----occ----\n");
  printf("Occupied space segments:\n");
  for (int i=0; i<NB_PARTITIONS; i++){
    if (occu_places[2*i] != 0){
      printf("[%d-----%d], ",
                    occu_places[2*i],
                    occu_places[2*i+1]); 
    }
  }
  printf("\n-----occ end-----\n");
}

void print_free_spaces(uint32_t* free_space, int num_free_segments){
  for (int i = 0; i < num_free_segments; i++) {
        printf("Start: %u, End: %u\n", free_space[2*i], free_space[2*i + 1]);
    }
    printf("-----free end ----\n");
}
  

void print_partition_status(){
  if (global_mbr ==0){
    printf("No mbr table is in memory");
  }
  PRINT_GREEN("##########disk############\n");
  print_mbr_details();
  uint32_t* occu_places = find_occupied_space();
  if (occu_places !=0){
    print_occ_places(occu_places);
  } else{
    return;
  }
  int num_free_segments;
  uint32_t* free_space;
  find_free_space(occu_places, 
                  NB_PARTITIONS, 
                  disk_dev->get_disk_size(), 
                  &free_space, 
                  &num_free_segments);
  printf("\n-----free----\n");
  printf("Free space segments:\n");
  if (free_space !=0){
    print_free_spaces(free_space, num_free_segments);
    free(free_space);
  }
  free(occu_places);
  PRINT_GREEN("##########disk############\n");
}

void print_mbr_details() {
  if (!global_mbr){return;}
  for (int i = 0; i < NB_PARTITIONS; i++) {
    printf("Partition %d:\n", i + 1);
    printf("Status: %d\n", global_mbr->partitionTable[i].status);
    if (global_mbr->partitionTable[i].status != 0){
      printf("Start LBA: %u\n", global_mbr->partitionTable[i].startLBA);
      printf("Size (in LBA): %u\n", global_mbr->partitionTable[i].sizeLBA);
      printf("Type: %s\n", global_mbr->partitionTable[i].type == EXT2_PARTITION ? "EXT2" : "TEST");
      printf("-------------------\n");
    }
  }
}

uint32_t* find_occupied_space() {
  if (!global_mbr){return 0;}
  //four segment contaning the used space
  uint32_t* occupied_space = (uint32_t*)malloc(8*sizeof(uint32_t));
  for (int i = 0; i < NB_PARTITIONS; i++) {
    struct PartitionEntry* partition = &global_mbr->partitionTable[i];
    if (partition->status != 0){
      uint32_t startLBA = partition->startLBA;
      uint32_t endLBA = startLBA + partition->sizeLBA;
      occupied_space[i*2] = startLBA; 
      occupied_space[i*2+1] = endLBA; 
    }else{
      occupied_space[i*2] = 0; 
      occupied_space[i*2+1] = 0; 
    }
  }
  return occupied_space;  // No consecutive empty blocks found
}

void find_free_space(uint32_t* occupiedSpace, int numSegments, uint32_t diskSize, uint32_t** freeSpace, int* numFreeSegments) {
  if (occupiedSpace == 0 && numFreeSegments == 0 && freeSpace == 0){
    return;
  }
  // Sort the occupied segments in ascending order based on the start LBA
  for (int i = 0; i < numSegments - 1; i++) {
      for (int j = 0; j < numSegments - i - 1; j++) {
          if (occupiedSpace[j * 2] > occupiedSpace[(j + 1) * 2]) {
              uint32_t tempStart = occupiedSpace[j * 2];
              uint32_t tempEnd = occupiedSpace[j * 2 + 1];
              occupiedSpace[j * 2] = occupiedSpace[(j + 1) * 2];
              occupiedSpace[j * 2 + 1] = occupiedSpace[(j + 1) * 2 + 1];
              occupiedSpace[(j + 1) * 2] = tempStart;
              occupiedSpace[(j + 1) * 2 + 1] = tempEnd;
          }
      }
  }
  //print_occ_places(occupiedSpace);

  // Find the free segments by iterating over the occupied segments
  *numFreeSegments = 0;
  *freeSpace = (uint32_t*)malloc(sizeof(uint32_t) * numSegments * 2);
  uint32_t startLBA = 0;

  for (int i = 0; i < numSegments; i++) {
      if (startLBA < occupiedSpace[i * 2]) {
          (*freeSpace)[*numFreeSegments * 2] = startLBA;
          (*freeSpace)[*numFreeSegments * 2 + 1] = occupiedSpace[i * 2] - 1;
          // printf("numFreeSegments %d\n",*numFreeSegments);
          (*numFreeSegments)++;
      }
      startLBA = occupiedSpace[i * 2 + 1] + 1;
  }

  // Check for free space at the end of the disk
  if (startLBA < diskSize) {
      (*freeSpace)[*numFreeSegments * 2] = startLBA;
      (*freeSpace)[*numFreeSegments * 2 + 1] = diskSize - 1;
      (*numFreeSegments)++;
  }
  //print_free_spaces(*freeSpace,* numFreeSegments);
}


bool is_segment_in_free_space(uint32_t start, uint32_t size, uint32_t* free_space, int numFreeSegments) {
  if (free_space<0 || numFreeSegments ==0){
    return -1;
  }
  //printf("########is_segment_in_free_space#####\n");
  //print_free_spaces(free_space, numFreeSegments);
  for (int i = 0; i < numFreeSegments; i++) {
      if (start >= free_space[i * 2] && (start + size - 1) <= free_space[i * 2 + 1]) {
          debug_print_v_fs("Matching segments for start: %u, size: %u\n", start, size);
          debug_print_v_fs("Matching segment: Start: %u, End: %u\n", free_space[i * 2], free_space[i * 2 + 1]);
          return true;
      }
  }
  return false;
}

bool free_space(uint32_t start, uint32_t size){
  if (start + size > disk_dev->get_disk_size()){
    PRINT_RED("Space is too large\n");
    return false;
  }
  uint32_t* occu_places = find_occupied_space();
  if (occu_places == 0){
    printf("find_occupied_space failed");
    return false;
  }
  int num_free_segments;
  uint32_t* free_space;
  find_free_space(occu_places, 
                  NB_PARTITIONS, 
                  disk_dev->get_disk_size(), 
                  &free_space, 
                  &num_free_segments);
  if (free_space == 0){
    printf("find_free_space failed");
    free(occu_places);
    return false;
  }
  bool res = is_segment_in_free_space(start, size, free_space, num_free_segments);
  free(occu_places);
  free(free_space);
  return res;
} 

int create_partition(uint32_t start, uint32_t size, uint8_t partition_type){
  //We start by checking that an empty partition exists
  if (partition_type != EXT2_PARTITION 
      && partition_type != TEST_PARTITION){
        PRINT_RED("Partition type is not valid");
        return -1;
  }
  if (!global_mbr){return -1;}
  int empty_partition = -1;
  for (int i = 0; i < NB_PARTITIONS; i++) {
    if (global_mbr->partitionTable[i].status == 0x0){
      empty_partition = i;
      break;
    }
  }
  if (empty_partition == -1){
    PRINT_RED("There aren't any free partitions");
    return -1;
  } 
  if (free_space(start, size) == false){
    PRINT_RED("The specified space is not avaliable");
    return -1;
  }
  //The desired space is avaliable and 
  //we have found an empty partition that we can use
  if (partition_type == EXT2_PARTITION){
    if (size < MIN_EXT2_SIZE){
      printf("Size is too small for the ext 2 file system");
      return -1;
    }
  } 
  if (empty_partition != -1){
    print_fs_no_arg("Creating a new partition \n");
    debug_print_v_fs("[mbr]empty_partition = %d\n",empty_partition);
    debug_print_v_fs("[mbr]start %d\n",start); 
    debug_print_v_fs("[mbr]size %d\n",size); 
    debug_print_v_fs("[mbr]partition_type %d\n",partition_type); 
    global_mbr->partitionTable[empty_partition].status = ACTIVE_PARTITION;   
    global_mbr->partitionTable[empty_partition].type = partition_type;//test partition
    global_mbr->partitionTable[empty_partition].startLBA = start;
    global_mbr->partitionTable[empty_partition].sizeLBA = size;
    save_global_mbr();
    debug_print_v_fs("partition %d was created\n",empty_partition);
    printf("\033[0;32mPartition was created succefully\033[0m\n"); 
    if (partition_type == EXT2_PARTITION){
      configure_ext2_file_system(empty_partition);
    }
    return 0;
  }
  return -1;
}

int get_partition_size(uint8_t partiton_number){
  if (!(0 <= partiton_number && partiton_number < NB_PARTITIONS)){
    PRINT_RED("partition number must be between 1 and 4\n");
    return -1;
  }
  if (global_mbr == 0 ){
    return 0;
  }
  return global_mbr->partitionTable[partiton_number].sizeLBA;
}

int delete_partition(uint8_t partiton_number){
  if (!(1 < partiton_number && partiton_number <= NB_PARTITIONS)){
    PRINT_RED("partition number must be between 1 and 4\n");
    return -1;
  }
  if (global_mbr == 0){
    printf("mbr was not found");
    return -1;
  }
  global_mbr->partitionTable[partiton_number-1].status = NON_ACTIVE_PARTITION;  
  printf("\033[0;32mPartition was deleted succefully\033[0m\n"); 
  return 0;
}

