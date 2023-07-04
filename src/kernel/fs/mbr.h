#include "stdint.h"
#include <stdint.h>
#include "stdbool.h"
#ifndef MBR_H 
#define MBR_H

/*------ MBR ------
The MBR is utilized here to determine the layout of disk(and entering the kernel!).
Although this implementation does not mirror the exact behavior of a real MBR,
we will use it to designate the location of the file system.

A reserved partition of 32 sectors will be allocated for testing purposes,
and three additional partitions can be utilized to host different file systems.
By default, we will mount the second partition, but we may introduce functionality
to detect and utilize other partitions.

------ MBR ------*/

#define MBR_SIGNATURE  0x55AA //Used to determine is the first block is an mbr
#define MBR_BLOCK 1 //Location of the mbr
#define NON_ACTIVE_PARTIION 0x00
#define ACTIVE_PARTITION 0x80
#define NON_ACTIVE_PARTITION 0x00
#define NB_PARTITIONS 4


#define TEST_PARTITION 0x01 
#define EXT2_PARTITION 0x83
#define MIN_EXT2_SIZE 100 // the limit is added in order
                          // to make sure that the ex1t file system 
                          // can have a basic number of blocks

#pragma pack(1)  // Ensures that the struct is packed without any padding


typedef struct MBR {
    uint8_t bootstrapCode[446];  // Bootstrap code or boot loader code
    struct PartitionEntry {
        uint8_t status;            // Status of the partition (active or inactive)
        uint8_t startCHS[3];       // Start of the partition in disk geometry(not used)
        uint8_t type;              // Type of the partition(0x01 for test 
                                   // partition and 0x83 for ext2)
        uint8_t endCHS[3];         // End of the partition in disk geometry(not used)
        uint32_t startLBA;         // Starting logical block address (LBA) of the partition
        uint32_t sizeLBA;          // Size of the partition in logical blocks
    } partitionTable[4];           // Partition table entries (4 entries for MBR)
    uint16_t signature;            // MBR signature (0x55AA)
} mbr_t;

#pragma pack()  // Resets the packing to default

/**
 * @brief Global MBR instance used in the program.
 */
extern mbr_t *global_mdr;

/**
 * @brief Find and read the MBR from the disk.
 * @return 0 if MBR is found and read successfully, -1 otherwise.
 */
int find_mbr();

/**
 * @brief Save the global MBR to the disk.
 * @return 0 if the MBR is saved successfully, -1 if the global MBR is not initialized.
 */
int save_global_mbr();

/**
 * @brief Set up the MBR by allocating memory for the global MBR struct and setting the signature.
 */
int set_up_mbr();

/**
 * @brief Set up a test partition in the global MBR.
 * @return 0 if the test partition is set up successfully, -1 if the global MBR is not initialized.
 */
int setup_test_partition();

/**
 * @brief Prints mbr details
 */
void print_mbr_details();

/**
 * @brief Print occupied space
 * @param occu_places 
 */
void print_occ_places(uint32_t* occu_places);

/**
 * @brief print free space
 * 
 * @param free_space the free space table
 * @param num_free_segments the number of segments inside the tables
 */
void print_free_spaces(uint32_t* free_space,int num_free_segments);


/**
 * @brief Finds the free space on the disk by analyzing the occupied space segments.
 *
 * @param[in] occupiedSpace An array containing the start and end points of occupied space segments.
 * @param[in] numSegments The number of occupied space segments.
 * @param[in] diskSize The total size of the disk.
 * @param[out] freeSpace A pointer to the array that will store the start and end points of free space segments.
 * @param[out] numFreeSegments A pointer to the variable that will store the number of free space segments.
 */
void find_free_space(uint32_t* occupiedSpace, int numSegments, uint32_t diskSize, uint32_t** freeSpace, int* numFreeSegments);

/**
 * @brief Checks if a segment is within the free space.
 *
 * @param[in] start The starting point of the segment.
 * @param[in] size The size of the segment.
 * @param[in] freeSpace An array containing the start and end points of free space segments.
 * @param[in] numFreeSegments The number of free space segments.
 * @return True if the segment is within the free space, false otherwise.
 */
bool is_segment_in_free_space(uint32_t start, uint32_t size, uint32_t* freeSpace, int numFreeSegments);


/**
 * @brief Finds the occupied space on the disk.
 *
 * @return A pointer to an array containing the start and end points of occupied space segments.
 */
uint32_t* find_occupied_space();

/**
 * @brief Checks if the data between start and 
 * start + size is free within the disk
 * @param start start of the the data that we will look up 
 * @param size the size of the data
 * @return true the dusk space is free 
 * @return false the disk space is not free 
 */
bool free_space(uint32_t start, uint32_t size);



#endif  