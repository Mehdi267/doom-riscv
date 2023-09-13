#ifndef VIRT_IO 
#define VIRT_IO

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
/**
 * @brief Using virtio 1.1 
 * 
 * Virtio spec :  "https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf"
*/

// Disk registers location
#define VIRTIO0                     0x10001000
#define VIRTIO1                     0x10002000
//There are multiple locations for working with the virtio devices 
//they extend from 0x10001000-0x1000a000
//each device is will have 1000 bytes of space in which it registers 
//will be saved. 
//The disk is located in this spot because of the configuration 
//steps made when creating this qemu image
//QEMU_OPT_DISK = -drive file=xinul_disk.img,if=none,format=raw,id=x0

#define R(r) ((volatile uint32_t *)(VIRTIO0 + (r)))
#define G(r) ((volatile uint32_t *)(VIRTIO1 + (r)))

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
typedef struct virt_queue {
  struct virtq_desc desc[NUMQ];  /**< The actual descriptors */
  struct virtq_avail available;      /**< A ring of available descriptor heads with free-running index */
  struct virtq_used used;        /**< A ring of used descriptor heads with free-running index */
} virt_q;

/**
 * @brief Prints the current queue configuration
 * @param features the current virtio disk features
 */
void print_queue_configuration(uint32_t features);


#endif