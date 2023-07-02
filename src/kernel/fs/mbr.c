#include "drivers/disk_device.h" //to do disk operations
#include "mbr.h" 
#include "../memory/frame_dist.h" // used to allocate the block
#include <stdbool.h>
#include <stdint.h>
#include "stdio.h"
#include "string.h"

//This value represents the global mdr that 
//will be used the in the program
mbr_t *global_mdr;

int find_mbr(){
  //Read the fist disk 
  //block which contains the mbr
  disk_op disk_operation_mbr;
  disk_operation_mbr.blockNumber = MBR_BLOCK;
  disk_operation_mbr.type = READ;
  uint16_t data[256];
  disk_operation_mbr.data = (unsigned char*) data;
  // Perform disk read operation on the block taht contains the mbr
  disk_dev->read_disk(&disk_operation_mbr);
  if (data[255] == MBR_SIGNATURE) {
    //Mbr was found we can now access partitions
    global_mdr = get_frame();
    memcpy(global_mdr,(void*)data,512);
    return 0;
  }
  else{
    return -1;
  }
}

int save_global_mbr(){
  if (global_mdr == 0){
    return -1;
  }
  disk_op disk_operation_mbr;
  disk_operation_mbr.blockNumber = MBR_BLOCK;
  disk_operation_mbr.type = WRITE;
  disk_operation_mbr.data = (unsigned char*) global_mdr;
  // Perform disk read operation on the block taht contains the mbr
  return disk_dev->write_disk(&disk_operation_mbr);
}

int set_up_mbr(){ 
  //We allocate data for the global mbr struct
  global_mdr = get_frame();
  memset(global_mdr, 0, 512);
  global_mdr->signature = MBR_SIGNATURE;
  if (setup_test_partition()<-1){
    return -1;
  }
  return save_global_mbr();
}

int setup_test_partition(){
  if(global_mdr ==0){
    return -1;
  }
  global_mdr->partitionTable[0].status = ACTIVE_PARTITION;
  global_mdr->partitionTable[0].type = TEST_PARTITION;//test partition
  global_mdr->partitionTable[0].startLBA = 2;
  global_mdr->partitionTable[0].sizeLBA = 32;
  return 0;
}

void print_partition_status(){
  if (global_mdr ==0){
    printf("No mbr table is in memory");
  }
  print_mbr_details();
  uint32_t* occu_places = find_occupied_space();
  if (occu_places !=0){
    printf("Occupied space segments:\n");
    for (int i=0; i<NB_PARTITIONS; i++){
      if (occu_places[2*i] != 0){
        printf("[%d-----%d], ",
                      occu_places[2*i],
                      occu_places[2*i+1]); 
      }
    }
    printf("\n-----end----\n");
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
  printf("Free space segments:\n");
  if (free_space !=0){
    for (int i = 0; i < num_free_segments; i++) {
        printf("Start: %u, End: %u\n", free_space[2*i], free_space[2*i + 1]);
    }
    printf("---------\n");
    free(free_space);
  }
  free(occu_places);
}

void print_mbr_details() {
  if (!global_mdr){return;}
  for (int i = 0; i < NB_PARTITIONS; i++) {
    printf("Partition %d:\n", i + 1);
    printf("Status: %02X\n", global_mdr->partitionTable[i].status);
    printf("Start LBA: %u\n", global_mdr->partitionTable[i].startLBA);
    printf("Size (in LBA): %u\n", global_mdr->partitionTable[i].sizeLBA);
    printf("Type: %d\n", global_mdr->partitionTable[i].type);
    printf("-------------------\n");
  }
}

uint32_t* find_occupied_space() {
  if (!global_mdr){return 0;}
  //four segment contaning the used space
  uint32_t* occupied_space = (uint32_t*)malloc(8*sizeof(uint32_t));
  for (int i = 0; i < NB_PARTITIONS; i++) {
    struct PartitionEntry* partition = &global_mdr->partitionTable[i];
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
}


bool is_segment_in_free_space(uint32_t start, uint32_t size, const uint32_t* freeSpace, int numFreeSegments) {
    if (freeSpace<0 || numFreeSegments ==0){
      return -1;
    }
    for (int i = 0; i < numFreeSegments; i++) {
        if (start >= freeSpace[i * 2] && (start + size - 1) <= freeSpace[i * 2 + 1]) {
            return true;
        }
    }
    return false;
}

bool free_space(uint32_t start, uint32_t size){
  if (start + size > disk_dev->get_disk_size()){
    printf("Space is too large");
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
  bool res = is_segment_in_free_space(start, size, free_space, NB_PARTITIONS);
  free(occu_places);
  free(free_space);
  return res;
} 

int create_partition(uint32_t start, uint32_t size, uint8_t partition_type){
  //We start by checking that an empty partition exists
  if (!global_mdr){return -1;}
  int empty_partition = -1;
  for (int i = 0; i < NB_PARTITIONS; i++) {
    if (global_mdr->partitionTable[i].status == 0x0){
      empty_partition = i;
      break;
    }
  }
  if (empty_partition == -1){
    printf("There aren't any free partitions");
    return -1;
  } 
  if (free_space(start, size) == false){
    printf("The specified space is not avaliable");
    return -1;
  }
  //The desired space is avaliable and 
  //we have found an empty partition that we can use
  if (partition_type == EXT2_PARTITION){
    if (size < MIN_EXT2_SIZE){
      printf("Size is too small for the ext 2 file system");
    }
  } 
  global_mdr->partitionTable[empty_partition].status = ACTIVE_PARTITION;  
  global_mdr->partitionTable[empty_partition].type = partition_type;//test partition
  global_mdr->partitionTable[empty_partition].startLBA = start;
  global_mdr->partitionTable[empty_partition].sizeLBA = size;
  return 0;
}

int delete_partition(uint8_t partiton_number){
  if (1 <= partiton_number && partiton_number < NB_PARTITIONS){
    printf("partition number must be between 1 and 4");
    return -1;
  }
  if (global_mdr == 0){
    printf("mbr was not found");
    return -1;
  }
  global_mdr->partitionTable[partiton_number].status = NON_ACTIVE_PARTITION;  
  return 0;
}

