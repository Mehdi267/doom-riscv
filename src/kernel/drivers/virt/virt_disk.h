#ifndef VIRT_DISK 
#define VIRT_DISK

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
/**
 * @brief I am using virtio 1.1 
 * because this is an old version qemu 
 * and does not support virtio 1.2 
 * 
 * Virtio spec :  "https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf"
*/

// Disk registers location
#define VIRTIO0                     0x10001000
//There are multiple locations for working with the virtio devices 
//they extend from 0x10001000-0x1000a000
//each device is will have 1000 bytes of space in which it registers 
//will be saved. The disk is located in this spot because of the configuration 
//steps made when creating this qemu image
//QEMU_OPT_DISK = -drive file=xinul_disk.img,if=none,format=raw,id=x0

#define R(r) ((volatile uint32_t *)(VIRTIO0 + (r)))

#define NUMQ 24 //Number of queues that we will define

//######################################################
//##  Configuration constants  ##
//######################################################

// Device Status Field(p15)
// The device status field provides a simple low-level indication of the completed steps of the initialization sequence.
#define STATUS_ACKNOWLEDGE          0x01    // Indicates that the guest OS has found the device and recognized it as a valid virtio device.
#define STATUS_DRIVER               0x02    // Indicates that the guest OS knows how to drive the device.
#define STATUS_FAILED               0x80    // Indicates that something went wrong in the guest, and it has given up on the device.
#define STATUS_FEATURES_OK          0x08    // Indicates that the driver has acknowledged all the features it understands, and feature negotiation is complete.
#define STATUS_DRIVER_OK            0x04    // Indicates that the driver is set up and ready to drive the device.
#define STATUS_DEVICE_NEEDS_RESET   0x40    // Indicates that the device has experienced an error from which it can't recover.

//MMIO REGISTERS AND ADDRESSES(p54)
//All of these adresses are located in the mmio part of the spec page 54
#define MMIO_MAGIC_VALUE             0x000 // Magic value; should be 0x74726976 ("virt" in ASCII)
#define MMIO_VERSION                 0x004 // Version; should be 2
#define MMIO_DEVICE_ID               0x008 // Device type; 2 for disk
#define MMIO_VENDOR_ID               0x00C // Vendor ID; 0x554d4551
#define MMIO_DEVICE_FEATURES         0x010 // Device Features
#define MMIO_DEVICE_FEATURES_SEL     0x014 // Device (host) features word selection
#define MMIO_DRIVER_FEATURES         0x020 // Driver Features
#define MMIO_DRIVER_FEATURES_SEL     0x024 // Activated (guest) features word selection
#define MMIO_QUEUE_SEL               0x030 // Virtual queue index
#define MMIO_QUEUE_NUM_MAX           0x034 // Maximum virtual queue size
#define MMIO_QUEUE_NUM               0x038 // Virtual queue size
#define MMIO_QUEUE_READY             0x044 // Virtual queue ready bit
#define MMIO_QUEUE_NOTIFY            0x050 // Queue notifier
#define MMIO_INTERRUPT_STATUS        0x060 // Interrupt status
#define MMIO_INTERRUPT_ACK           0x064 // Interrupt acknowledge
#define MMIO_STATUS                  0x070 // Device status
#define MMIO_QUEUE_DESC_LOW          0x080 // Virtual queue's Descriptor Area 64-bit long physical address (lower 32 bits)
#define MMIO_QUEUE_DESC_HIGH         0x084 // Virtual queue's Descriptor Area 64-bit long physical address (higher 32 bits)
#define MMIO_QUEUE_DRIVER_LOW        0x090 // Virtual queue's Driver Area 64-bit long physical address (lower 32 bits)
#define MMIO_QUEUE_DRIVER_HIGH       0x094 // Virtual queue's Driver Area 64-bit long physical address (higher 32 bits)
#define MMIO_QUEUE_DEVICE_LOW        0x0A0 // Virtual queue's Device Area 64-bit long physical address (lower 32 bits)
#define MMIO_QUEUE_DEVICE_HIGH       0x0A4 // Virtual queue's Device Area 64-bit long physical address (higher 32 bits)
#define MMIO_CONFIG_GENERATION       0x0FC // Configuration atomicity value
#define MMIO_CONFIG                  0x100 // Configuration space

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
//##   Virt queue ##
//######################################################



/**
 * @struct virtq_desc
 * @brief Descriptor structure for a VirtIO device.
 *
 * This structure represents a descriptor used in the VirtIO device. It contains
 * information about the buffer address, length, and flags.
 */
struct virtq_desc {
  #define VIRTQ_DESC_F_NEXT 1
  #define VIRTQ_DESC_F_WRITE 2
  #define VIRTQ_DESC_F_INDIRECT 4
  uint64_t addr;      /**< Address (guest-physical) */
  uint32_t len;       /**< Length */
  uint16_t flags;     /**< Flags indicating descriptor properties */
  uint16_t next;      /**< Next descriptor index if flags & NEXT */
};

/**
 * @struct virtq_avail
 * @brief Available ring structure for a VirtIO device.
 *
 * This structure represents the available ring used in the VirtIO device. It
 * includes flags, an index, and an array of ring entries indicating available
 * descriptors.
 */
struct virtq_avail {
  #define VIRTQ_AVAIL_F_NO_INTERRUPT 1
  uint16_t flags;               /**< Flags for available ring */
  uint16_t idx;                 /**< Index */
  uint16_t ring[NUMQ];          /**< Array of available ring entries */
};

/**
 * @struct virtq_used_elem
 * @brief Used element structure for a VirtIO device.
 *
 * This structure represents an element used in the used ring of the VirtIO
 * device. It contains the index of the start of the used descriptor chain and
 * the total length of the descriptor chain that was used (written to).
 */
struct virtq_used_elem {
  #define VIRTQ_USED_F_NO_NOTIFY 1
  uint32_t id;                  /**< Index of start of used descriptor chain */
  uint32_t len;                 /**< Total length of used descriptor chain */
};

/**
 * @struct virtq_used
 * @brief Used ring structure for a VirtIO device.
 *
 * This structure represents the used ring used in the VirtIO device. It includes
 * flags, an index, an array of used elements, and an available event (only if
 * VIRTIO_F_EVENT_IDX).
 */
struct virtq_used {
    uint16_t flags;               /**< Flags for used ring */
    uint16_t idx;                 /**< Index */
    struct virtq_used_elem ring[NUMQ];   /**< Array of used elements */
};

/**
 * @struct virt_queue
 * @brief VirtIO queue structure for a device.
 *
 * This structure represents a VirtIO queue used by a specific device. It contains
 * the descriptor array, an available ring, and a used ring.
 */
static struct virt_queue {
  struct virtq_desc desc[NUMQ];  /**< The actual descriptors */
  struct virtq_avail available;      /**< A ring of available descriptor heads with free-running index */
  struct virtq_used used;        /**< A ring of used descriptor heads with free-running index */
} block_q;

//######################################################
//##   disk management and operations ##
//######################################################

static struct disk_management {
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
 * @brief Prints the current queue configuration
 * @param features the current virtio disk features
 */
void print_queue_configuration(uint32_t features);

/**
 * @brief Get the disk capacity
 * @return uint32_t the disk capacity
 */
uint32_t get_disk_capacity();


#endif