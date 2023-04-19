/**
 * @file virt_disk.c
 * @author Mehdi Frikha
 * @brief 
 * This work is inspired from the xv6 riscv os project at mit
 * and the ensimag os project
 * https://github.com/mit-pdos/xv6-riscv
 */
#include "stdint.h"
#include "drivers/disk_device.h"
#include "stddef.h"
#include "stdio.h"
#define VIRTIO0 0x10001000 //Disk registers location
#define R(r) ((volatile uint32_t *)(VIRTIO0 + (r)))

#define VIRTIO_MMIO_MAGIC_VALUE		0x000 // 0x74726976; fun fact this is the word virt is ascii
#define VIRTIO_MMIO_VERSION		0x004 // version; should be 2
#define VIRTIO_MMIO_DEVICE_ID		0x008 // device type; 1 is net, 2 is disk
#define VIRTIO_MMIO_VENDOR_ID		0x00c // 0x554d4551

/**
 * @brief This function will be used to configure the disk, this gets done by modifing the registers  
 * that are associated with the block device that are located at the virtio memory space
 * the disk is added to the first virtio disk slot at this address 0x10001000 and its registers 
 * are located within the 0x10001000 - 0x10001100 range 
 */
static void virt_disk_init()
{
  //This is just to verify that the device is a virt device 
  //and that is a disk and that is have the approriate configuration values
  if(*R(VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976 ||
    *R(VIRTIO_MMIO_VERSION) != 2 ||
    *R(VIRTIO_MMIO_DEVICE_ID) != 2 ||
    *R(VIRTIO_MMIO_VENDOR_ID) != 0x554d4551){
    printf("Disk not found !!!!\n");
    return;
  }


  return;
}

static void virt_disk_read()
{
  return;
}


static void virt_disk_write()
{
  return;
}


disk_device_t virt_disk = {
	virt_disk_init,
  virt_disk_read,
  virt_disk_write
};
