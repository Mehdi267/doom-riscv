/**
 * @brief Virtual Disk Device implementation
 * @author Mehdi Frikha 
 * This file contains the implementation of the Virtual Disk Device, which interacts with the disk
 * in the virtual environment.
 * Virtio spec :  "https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf"
 * This spec is mentioned many times in this article as all of the details related to 
 * the implemenetation come from there 
 * Other resources : 
 * https://blogs.oracle.com/linux/post/introduction-to-virtio
 * https://web.eecs.utk.edu/~smarz1/courses/cosc361/notes/virtio/
 * This work is inspired by the xv6 RISC-V OS project at MIT
 * and the ENSIMAG OS project.
 * inspired from : https://github.com/mit-pdos/xv6-riscv
 */

#include "stdint.h"
#include "drivers/disk_device.h"
#include "stddef.h"
#include "stdio.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "logger.h"

#include "timer.h"
#include "string.h"
#include "virt_disk.h"
#include "virtio.h"

//Disk queue
virt_q block_q;

/**
 * @brief Initializes the virtual disk device by configuring 
 * its registers in the virtio memory space.
 * Device setup occures in 8 steps as described in the spec(p39)
 * these steps has been taken directly from the spec 
 * 1. Reset the device.
 * 2. Set the ACKNOWLEDGE status bit: the guest OS has noticed the device.
 * 3. Set the DRIVER status bit: the guest OS knows how to drive the device.
 * 4. Read device feature bits, and write the subset of feature bits understood by the OS and driver to the
 * device. During this step the driver MAY read (but MUST NOT write) the device-specific configuration
 * fields to check that it can support the device before accepting it.
 * 5. Set the FEATURES_OK status bit. The driver MUST NOT accept new feature bits after this step.
 * 6. Re-read device status to ensure the FEATURES_OK bit is still set: otherwise, the device does not
 * support our subset of features and the device is unusable.
 * 7. Perform device-specific setup, including discovery of virtqueues for the device, optional per-bus setup,
 * reading and possibly writing the device’s virtio configuration space, and population of virtqueues.
 * 8. Set the DRIVER_OK status bit. At this point the device is “live”.
 */
void virt_disk_init(){
  // Verify that the device is a virtio device, a disk, and has the appropriate configuration values.
  if (*R(MMIO_MAGIC_VALUE) != 0x74726976 ||
      *R(MMIO_VERSION) != 2 ||
      *R(MMIO_DEVICE_ID) != 2 ||
      *R(MMIO_VENDOR_ID) != 0x554d4551) {
      //If the we are here that means that 
      //the disk was not found, please 
      //check that the image exists and that are no problem
      //when linking it to the qemu image
      printf("Disk not found!\n");
      return;
  }
  
  //Step 1 : we reset the device
  *R(MMIO_STATUS) = 0;
  
  //Step 2: Set the ACKNOWLEDGE status bit: the guest OS has noticed the device.
  *R(MMIO_STATUS) = STATUS_ACKNOWLEDGE;
  
  //Step 3: Set the DRIVER status bit: the guest OS knows how to drive the device.
  *R(MMIO_STATUS) |= STATUS_DRIVER; 
  
  //Step 4 : features configuraton
  uint64_t features = *R(MMIO_DEVICE_FEATURES);
  // printb(&features, 4);
  features &= ~(1 << VIRTIO_BLK_F_RO);
  features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
  features &= ~(1 << VIRTIO_RING_F_EVENT_IDX);
  features &= ~(1 << VIRTIO_RING_F_INDIRECT_DESC);
  *R(MMIO_DRIVER_FEATURES) = features;

  // uint32_t diskFeatures = *R(MMIO_DEVICE_FEATURES);
  // print_queue_configuration(diskFeatures);

  //Step 5: we validate the features by setting the features okay bit to one
  *R(MMIO_STATUS) |= STATUS_FEATURES_OK;
  
  //Step 6: We validate the the Features were accepted
  if (!(*R(MMIO_STATUS) & STATUS_FEATURES_OK)){
    printf("Features are not valid!\n");
    return;
  }
  
  //Step 7: Device specific setup
  // initialize queue 0.
  *R(MMIO_QUEUE_SEL) = 0;
  //printf("max queue = %d \n",*R(MMIO_QUEUE_NUM_MAX));  read_virtio_blk_config();
  // ensure queue 0 is not in use.
  
  if(*R(MMIO_QUEUE_READY))
    printf("virtio disk should not be ready");

  // check maximum queue size.
  uint32_t max = *R(MMIO_QUEUE_NUM_MAX);
  if(max < NUMQ){
    printf("\x1B[1\x1B[31 Queue size is too small!\x1b[0");
    return;
  }
  *R(MMIO_QUEUE_NUM) = NUMQ;

  //We save the location of the queues
  *R(MMIO_QUEUE_DESC_LOW) = (uint64_t)block_q.desc;
  *R(MMIO_QUEUE_DESC_HIGH) = (uint64_t)block_q.desc >> 32;
  *R(MMIO_QUEUE_DRIVER_LOW) = (uint64_t)&block_q.available;
  *R(MMIO_QUEUE_DRIVER_HIGH) = (uint64_t)&block_q.available >> 32;
  *R(MMIO_QUEUE_DEVICE_LOW) = (uint64_t)&block_q.used;
  *R(MMIO_QUEUE_DEVICE_HIGH) = (uint64_t)&block_q.used >> 32;
  
  // queue is ready.
  *R(MMIO_QUEUE_READY) = 0x1;
  //Step 8 : tell the device that everything is ready
  *R(MMIO_STATUS) |= STATUS_DRIVER_OK;
  if (*R(MMIO_STATUS) != 15){
    printf("\x1B[1m\x1B[31m Disk setup failed! STATUS %d\x1b[0m",
                       *R(MMIO_STATUS));
    return;
  }


  for (int i=0; i < NUMQ; i++){
    block_m.desc_avail[i] = 0;
  }
  block_q.available.idx = 0;
  block_q.available.flags = 
          VIRTQ_AVAIL_F_NO_INTERRUPT; // no intterupt but might change 
                                      // later in order to have faster
                                      // read speeds
  block_m.used_iter = 0;
  return;
}

/**
 * Performs a disk operation using a virtual block_q.
 * 
 * Unfortunately, in order to perform this disk operation, 
 * we cannot strictly adhere to the specifications.
 * Instead, we need to approach this problem differently.
 * 
 * The specification suggests using a single descriptor to handle the request,
 * but we will be using three descriptors.
 * This deviation from the spec is necessary due to limitations and issues  
 * encountered with QEMU.
 * 
 * The struct virtio_blk_req defined in the specification is as follows:
 * 
 *     struct virtio_blk_req {
 *         le32 type;
 *         le32 reserved;
 *         le64 sector;
 *         u8 data[];
 *         u8 status;
 *     };
 * 
 * According to the spec, we can use a request of this type, where we fill in
 * the type (read or write),
 * sector number, and data pointer. The device will then write to the status 
 * field to indicate the success
 * or failure of the operation.
 * 
 * However, this approach doesn't work as 
 * expected due to issues caused by QEMU. A workaround for this problem
 * is explained in detail in the following
 * blog post: https://brennan.io/2020/03/22/sos-block-device/.
 * 
 * The solution is to split the descriptor into three separate 
 * descriptors, chaining them together,
 * and modifying their flags to achieve the desired operations.
 * Similar code implementing this approach
 * can be found in the xv6 project.
 */
int virt_disk_op(disk_op* oper) {
  //We start by looking for three 
  //free descriptors, it is possible 
  //that we cannot find one, thus we can to make sure 
  //the process sleeps until some are liberated 
  if (oper == 0){
    print_v_disk_no_arg("disk pointer is null");
  }
  debug_print_v_disk("[disk]Doing a disk operation of type %s\n", oper->type ? "WRITE":"READ" );
  uint32_t* desc_list =  get_three_desc();

  while(desc_list == 0){
    //Must use a mutex or the mutex 
    //or make the process sleep x amout of time 
    //because this is wasted processing
    desc_list =  get_three_desc();
  }
  //Since we found three free descriptors we can start filling them up 
  //and tell the device to do the operation
  struct virtio_blk_req *req = (struct virtio_blk_req *)
                          malloc(sizeof(struct virtio_blk_req));

  if (req == 0){
    return -1;
  }
  if(oper->type)
    req->type = VIRTIO_BLK_T_OUT; // write the disk
  else
    req->type = VIRTIO_BLK_T_IN; // read the disk
  req->reserved = 0; 
  req->sector = oper->blockNumber;
  //The first descriptor, we place the request and we 
  //inform the desc that this is not a full request 
  //by using the next flag
  
  // printf("%d/%d/%d \n", desc_list[0], desc_list[1], desc_list[2]);
  block_q.desc[desc_list[0]].addr = (uint64_t) req;
  block_q.desc[desc_list[0]].len = sizeof(struct virtio_blk_req);
  block_q.desc[desc_list[0]].flags = VIRTQ_DESC_F_NEXT;
  block_q.desc[desc_list[0]].next = desc_list[1];

  //In the second desc we place the location of the data pointer 
  //and we also link to the last desc in which that contains the status
  block_q.desc[desc_list[1]].addr = (uint64_t) oper->data;
  block_q.desc[desc_list[1]].len = BLOCK_SIZE;
  //These are mean for the device if we wish to read
  //then the device will write to b -> read and vice versa
  if(oper->type == READ){
    block_q.desc[desc_list[1]].flags = VIRTQ_DESC_F_WRITE;
    block_q.desc[desc_list[1]].flags |= VIRTQ_DESC_F_NEXT;
  }
  else if (oper->type == WRITE){ 
    block_q.desc[desc_list[1]].flags = 0;
    block_q.desc[desc_list[1]].flags |= VIRTQ_DESC_F_NEXT;
  }

  block_q.desc[desc_list[1]].next = desc_list[2];

  //Status bit desc 
  block_m.desc_statut[desc_list[0]] = 0xff;//We set it one but if the operation was successful 
                                        //it will be set to zero by the device 
  block_q.desc[desc_list[2]].addr = (uint64_t) &block_m.desc_statut[desc_list[0]];
  block_q.desc[desc_list[2]].len = 1;
  block_q.desc[desc_list[2]].flags = VIRTQ_DESC_F_WRITE;
  block_q.desc[desc_list[2]].next = 0;
  
  // tell the device the first index in our chain of descriptors.
  block_q.available.ring[block_q.available.idx % NUMQ] = desc_list[0];

  // tell the device another avail ring entry is available.
  block_q.available.idx += 1; 
  debug_print_v_disk("[disk]newly set statut value = %d \n", block_m.desc_statut[desc_list[0]]);
  debug_print_v_disk("[disk]used ring idx; before operation = %d\n",block_q.used.idx);
  *R(MMIO_QUEUE_NOTIFY) = 0;
  while(block_m.desc_statut[desc_list[0]] == 0xff){
    // debug_print_v_disk("[disk]statut desc loop = %d \n", block_m.desc_statut[desc_list[0]]);
  }
  debug_print_v_disk("[disk]Statut desc loop exit = %d \n", block_m.desc_statut[desc_list[0]]);
  debug_print_v_disk("[disk]Used ring idx = %d" ,
                  block_q.used.idx);
  debug_print_v_disk(" desc id =  %d / len = %d\n",
                  block_q.used.ring[block_q.used.idx-1%NUMQ].id, 
                  block_q.used.ring[block_q.used.idx-1%NUMQ].len);
  // print_block(oper->data, 512);
  free_list(desc_list);
  free(desc_list);
  free(req);
  return 0;
}


/**
 * @brief Reads data from the virtual block_q.
 */
int virt_disk_read(disk_op* op){
  return virt_disk_op(op);
}

/**
 * @brief Writes data to the virtual block_q.
 */
int virt_disk_write(disk_op* op){
  return virt_disk_op(op);
}


void read_virtio_blk_config() {
  d_conf *config = (d_conf*) (char*)(VIRTIO0 + 0x100);
  printf("\n-------Disk configuration ------\n");
  printf("Capacity: %lu\n", config->capacity);
  printf("Size Max: %u\n", config->size_max);
  printf("Seg Max: %u\n", config->seg_max);
  printf("Geometry:\n");
  printf("  Cylinders: %u\n", config->geometry.cylinders);
  printf("  Heads: %u\n", config->geometry.heads);
  printf("  Sectors: %u\n", config->geometry.sectors);
  printf("Block Size: %u\n", config->blk_size);
  printf("Topology:\n");
  printf("  Physical Block Exp: %u\n", config->topology.physical_block_exp);
  printf("  Alignment Offset: %u\n", config->topology.alignment_offset);
  printf("  Min IO Size: %u\n", config->topology.min_io_size);
  printf("  Opt IO Size: %u\n", config->topology.opt_io_size);
  printf("Writeback: %u\n", config->writeback);
  printf("Max Discard Sectors: %u\n", config->max_discard_sectors);
  printf("Max Discard Seg: %u\n", config->max_discard_seg);
  printf("Discard Sector Alignment: %u\n", config->discard_sector_alignment);
  printf("Max Write Zeroes Sectors: %u\n", config->max_write_zeroes_sectors);
  printf("Max Write Zeroes Seg: %u\n", config->max_write_zeroes_seg);
  printf("Write Zeroes May Unmap: %u\n", config->write_zeroes_may_unmap);
  printf("-------Disk configuration end------\n\n");
}

uint32_t get_disk_capacity() {
  d_conf *config = (d_conf*) (char*)(VIRTIO0 + 0x100);
  return (uint32_t)config->capacity;
}


void print_queue_configuration(uint32_t features) {
    struct {
        uint32_t bit;
        const char* description;
    } bitDescriptions[] = {
        {1, "VIRTIO_BLK_F_SIZE_MAX: Maximum size of any single segment is in size_max."},
        {2, "VIRTIO_BLK_F_SEG_MAX: Maximum number of segments in a request is in seg_max."},
        {4, "VIRTIO_BLK_F_GEOMETRY: Disk-style geometry specified in geometry."},
        {5, "VIRTIO_BLK_F_RO: Device is read-only."},
        {6, "VIRTIO_BLK_F_BLK_SIZE: Block size of the disk is in blk_size."},
        {9, "VIRTIO_BLK_F_FLUSH: Cache flush command support."},
        {10, "VIRTIO_BLK_F_TOPOLOGY: Device exports information on optimal I/O alignment."},
        {11, "VIRTIO_BLK_F_CONFIG_WCE: Device can toggle its cache between writeback and writethrough modes."},
        {13, "VIRTIO_BLK_F_DISCARD: Device can support discard command, maximum discard sectors size in max_discard_sectors and maximum discard segment number in max_discard_seg."},
        {14, "VIRTIO_BLK_F_WRITE_ZEROES: Device can support write zeroes command, maximum write zeroes sectors size in max_write_zeroes_sectors and maximum write zeroes segment number in max_write_zeroes_seg."}
    };
    printf("########Features##########\n\n");
    printf("Current configuration of the queue:\n");
    for (int i = 0; i < sizeof(bitDescriptions) / sizeof(bitDescriptions[0]); ++i) {
        if (features & (1 << bitDescriptions[i].bit)) {
            printf("  - (bit - %d) desc : %s\n",
                                bitDescriptions[i].bit,
                                bitDescriptions[i].description);
        }
    }
    printf("\n########Features End ##########\n");
}


uint32_t* get_three_desc(){
  uint32_t* desc_ids = (uint32_t*)malloc(3 * sizeof(uint32_t));;
  uint32_t nb_found = 0;
  uint32_t iter = 0;
  while(1){
    if (iter == NUMQ){
      //We did not find three desc
      //We free those who have been 
      //reserved
      if (nb_found > 0){
        block_m.desc_avail[desc_ids[0]] = 0;
        if (nb_found == 2){
          block_m.desc_avail[desc_ids[1]] = 0;
        }
      }
      free(desc_ids);
      return 0;
    }
    if (block_m.desc_avail[iter] == 0){
        block_m.desc_avail[iter] = 1; //Must be atomic when working with multiple threads
        desc_ids[nb_found] = iter;
        nb_found++;
    }
    if (nb_found == 3){
      return desc_ids;
    }
    iter++;
  }
}

void free_list(uint32_t* list){
  if (list != 0){
    block_m.desc_avail[list[0]]=0;
    block_m.desc_avail[list[1]]=0;
    block_m.desc_avail[list[2]]=0;
  }
}


// Virtual Disk Device structure
disk_device_t virt_disk = {
    virt_disk_init,
    virt_disk_read,
    virt_disk_write,
    get_disk_capacity,
    read_virtio_blk_config
};