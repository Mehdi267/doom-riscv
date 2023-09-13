#ifndef VIRT_DISK 
#define VIRT_DISK

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "virtio.h"


//Feature Bits for Virtio Block Device(p84)
#define VIRTIO_BLK_F_SIZE_MAX        1     // Maximum size of any single segment is in size_max.
#define VIRTIO_BLK_F_SEG_MAX         2     // Maximum number of segments in a request is in seg_max.
#define VIRTIO_BLK_F_GEOMETRY        4     // Disk-style geometry specified in geometry.
#define VIRTIO_BLK_F_RO              5     // Device is read-only.
#define VIRTIO_BLK_F_BLK_SIZE        6     // Block size of disk is in blk_size.
#define VIRTIO_BLK_F_FLUSH           9     // Cache flush command support.
#define VIRTIO_BLK_F_TOPOLOGY        10    // Device exports information on optimal I/O alignment.
#define VIRTIO_BLK_F_CONFIG_WCE      11    // Device can toggle its cache between writeback and writethrough modes.
#define VIRTIO_BLK_F_DISCARD         13    // Device can support discard command, maximum discard sectors size in max_discard_sectors and maximum discard segment number in max_discard_seg.
#define VIRTIO_BLK_F_WRITE_ZEROES    14    // Device can support write zeroes command, maximum write zeroes sectors size in max_write_zeroes_sectors and maximum write zeroes segment number in max_write_zeroes_seg.
#define VIRTIO_RING_F_INDIRECT_DESC  28
#define VIRTIO_RING_F_EVENT_IDX      29


//######################################################
//##   disk management and operations ##
//######################################################

struct disk_management {
  //Indicates how many descriptors 
  //have been checked in the used locations, 
  //this value is used for our personal record 
  //and will be compared with the value present 
  //in the device ringto check if the we have verified 
  //of all of the reads have been validated    
  uint32_t used_iter;
  uint8_t desc_avail[NUMQ];
  uint8_t desc_statut[NUMQ];
} block_m;


struct virtio_blk_req {
  #define VIRTIO_BLK_T_IN 0
  #define VIRTIO_BLK_T_OUT 1
  #define VIRTIO_BLK_T_FLUSH 4
  #define VIRTIO_BLK_T_DISCARD 11
  #define VIRTIO_BLK_T_WRITE_ZEROES 13
  uint32_t type;
  uint32_t reserved;
  uint64_t sector;
};

//######################################################
//##   config struct ##
//######################################################

//spec p85 block device config structure
//We can access this configuration data placed in the address 
//of the virtio registers plus 0x100. 
//These  
typedef struct virtio_blk_config {
  uint64_t capacity;
  uint32_t size_max;
  uint32_t seg_max;
  struct virtio_blk_geometry {
    uint16_t cylinders;
    uint8_t heads;
    uint8_t sectors;
  } geometry;
  uint32_t blk_size;
  struct virtio_blk_topology {
    // # of logical blocks per physical block (log2)
    uint8_t physical_block_exp;
    // offset of first aligned logical block
    uint8_t alignment_offset;
    // suggested minimum I/O size in blocks
    uint16_t min_io_size;
    // optimal (suggested maximum) I/O size in blocks
    uint32_t opt_io_size;
  } topology;
  uint8_t writeback;
  uint8_t unused0[3];
  uint32_t max_discard_sectors;
  uint32_t max_discard_seg;
  uint32_t discard_sector_alignment;
  uint32_t max_write_zeroes_sectors;
  uint32_t max_write_zeroes_seg;
  uint8_t write_zeroes_may_unmap;
  uint8_t unused1[3];
} d_conf;

/**
 * @brief This function display the properties 
 * of the disk
 */
extern void read_virtio_blk_config();

/**
 * @brief Retrieves three available descriptors.
 * 
 * @return A pointer to the array of three descriptors,
 * or NULL if not found.
 */
uint32_t* get_three_desc();

/**
 * @brief Free a list of descpritors present in a list
 * @param list the list that containt the desciptors
 */
void free_list(uint32_t* list);

/**
 * @brief Get the disk capacity
 * @return uint32_t the disk capacity
 */
uint32_t get_disk_capacity();


#endif