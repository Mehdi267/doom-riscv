/**
 * @file virt_disk.c
 * @brief Virtual Disk Device implementation
* @author Mehdi Frikha 
 * This file contains the implementation of the Virtual Disk Device, which interacts with the disk
 * in the virtual environment.
 * 
 * This work is inspired by the xv6 RISC-V OS project at MIT
 * and the ENSIMAG OS project.
 * refernce: https://github.com/mit-pdos/xv6-riscv
 */

#include "stdint.h"
#include "drivers/disk_device.h"
#include "stddef.h"
#include "stdio.h"

#define VIRTIO0                     0x10001000 // Disk registers location
#define R(r) ((volatile uint32_t *)(VIRTIO0 + (r)))

#define VIRTIO_MMIO_MAGIC_VALUE     0x000 // Magic value; should be 0x74726976 ("virt" in ASCII)
#define VIRTIO_MMIO_VERSION         0x004 // Version; should be 2
#define VIRTIO_MMIO_DEVICE_ID       0x008 // Device type; 2 for disk
#define VIRTIO_MMIO_VENDOR_ID       0x00c // Vendor ID; 0x554d4551

/**
 * @brief Initializes the virtual disk device by configuring its registers in the virtio memory space.
 */
static void virt_disk_init()
{
    // Verify that the device is a virtio device, a disk, and has the appropriate configuration values.
    if (*R(VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976 ||
        *R(VIRTIO_MMIO_VERSION) != 2 ||
        *R(VIRTIO_MMIO_DEVICE_ID) != 2 ||
        *R(VIRTIO_MMIO_VENDOR_ID) != 0x554d4551) {
        printf("Disk not found!!!\n");
        return;
    }

    // Perform additional initialization if required.

    return;
}

/**
 * @brief Reads data from the virtual disk.
 */
static void virt_disk_read()
{
    // Implement the disk read operation.

    return;
}

/**
 * @brief Writes data to the virtual disk.
 */
static void virt_disk_write()
{
    // Implement the disk write operation.

    return;
}

// Virtual Disk Device structure
disk_device_t virt_disk = {
    virt_disk_init,
    virt_disk_read,
    virt_disk_write
};
