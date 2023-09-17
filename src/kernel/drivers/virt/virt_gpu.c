/**
 * @brief gpu implementation
 * @author Mehdi Frikha 
 * Virtio spec :  "https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf"
 * Other resources : 
 * https://blog.stephenmarz.com/2020/11/11/risc-v-os-using-rust-graphics/#virtio
 */

#include "stdint.h"
#include "drivers/gpu_device.h"
#include "stddef.h"
#include "stdio.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "logger.h"

#include "timer.h"
#include "string.h"
#include "virt_gpu.h"
#include "assert.h"

//Gpu queue
virt_q gpu_q;
/**
 * @brief Initializes the virtual gpu device by configuring 
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
int virt_gpu_init(){
  // Verify that the device is a virtio device, a disk, and has the appropriate configuration values.
  if (*G(MMIO_MAGIC_VALUE) != 0x74726976 ||
      *G(MMIO_VERSION) != 2 ||
      *G(MMIO_DEVICE_ID) != 16 ||
      *G(MMIO_VENDOR_ID) != 0x554d4551) {
      //If the we are here that means that 
      //the disk was not found, please 
      //check that the image exists and that are no problem
      //when linking it to the qemu image
      printf("Gpu not found!\n");
      return -1;
  }
  //Step 1 : we reset the device
  *G(MMIO_STATUS) = 0;
  
  //Step 2: Set the ACKNOWLEDGE status bit: the guest OS has noticed the device.
  *G(MMIO_STATUS) = STATUS_ACKNOWLEDGE;
  
  //Step 3: Set the DRIVER status bit: the guest OS knows how to drive the device.
  *G(MMIO_STATUS) |= STATUS_DRIVER; 
  
  //Step 4 : features configuraton
  uint64_t features = *G(MMIO_DEVICE_FEATURES);
  features &= 0;
  *G(MMIO_DRIVER_FEATURES) = features;

  //Step 5: we validate the features by setting the features okay bit to one
  *G(MMIO_STATUS) |= STATUS_FEATURES_OK;
  
  //Step 6: We validate the the Features were accepted
  if (!(*G(MMIO_STATUS) & STATUS_FEATURES_OK)){
    printf("Features are not valid!\n");
    return -1;
  }
  
  //Step 7: Device specific setup
  // initialize queue 0.
  *G(MMIO_QUEUE_SEL) = 0;
  // ensure queue 0 is not in use.
  
  if(*G(MMIO_QUEUE_READY))
    printf("virtio disk should not be ready");

  // check maximum queue size.
  uint32_t max = *G(MMIO_QUEUE_NUM_MAX);
  if(max < NUMQ){
    printf("\x1B[1\x1B[31 Queue size is too small!\x1b[0");
    return -1;
  }
  *G(MMIO_QUEUE_NUM) = NUMQ;

  //We save the location of the queues
  *G(MMIO_QUEUE_DESC_LOW) = (uint64_t)gpu_q.desc;
  *G(MMIO_QUEUE_DESC_HIGH) = (uint64_t)gpu_q.desc >> 32;
  *G(MMIO_QUEUE_DRIVER_LOW) = (uint64_t)&gpu_q.available;
  *G(MMIO_QUEUE_DRIVER_HIGH) = (uint64_t)&gpu_q.available >> 32;
  *G(MMIO_QUEUE_DEVICE_LOW) = (uint64_t)&gpu_q.used;
  *G(MMIO_QUEUE_DEVICE_HIGH) = (uint64_t)&gpu_q.used >> 32;
  
  // queue is ready.
  *G(MMIO_QUEUE_READY) = 0x1;
  //Step 8 : tell the device that everything is ready
  *G(MMIO_STATUS) |= STATUS_DRIVER_OK;
  if (*G(MMIO_STATUS) != 15){
    printf("\x1B[1m\x1B[31m gpu setup failed! STATUS %d\x1b[0m",
                       *G(MMIO_STATUS));
    return -1;
  }

  for (int i=0; i < NUMQ; i++){
    gpu_m.desc_avail[i] = 0;
  }
  gpu_q.available.idx = 0;
  gpu_q.available.flags = 
          VIRTQ_AVAIL_F_NO_INTERRUPT; // no intterupt but might change 
                                      // later in order to have faster
                                      // read speeds
  gpu_m.used_iter = 0;
  // print_display_info();
  virt_gpu.frame =  malloc(HEIGHT*WIDTH*sizeof(pixel)); 
  if (virt_gpu.frame == 0){return -1;}
  init_gpu_frame(virt_gpu.frame);
  display_boxes();
  update_all();
  return 0;
}


void request_gpu(void *req, uint64_t req_size, void* res, uint64_t res_size){
  uint32_t* desc_list =  get_desc(3);  
  while(desc_list == 0){
    desc_list = get_desc(3);
  }  

  gpu_q.desc[desc_list[0]].addr = (uint64_t) req;
  gpu_q.desc[desc_list[0]].len = req_size;
  gpu_q.desc[desc_list[0]].flags = VIRTQ_DESC_F_NEXT;
  gpu_q.desc[desc_list[0]].next = desc_list[1];

  gpu_q.desc[desc_list[1]].addr = (uint64_t) res;
  gpu_q.desc[desc_list[1]].len = res_size;
  gpu_q.desc[desc_list[1]].flags = VIRTQ_DESC_F_WRITE ;
  gpu_q.desc[desc_list[1]].flags |= VIRTQ_DESC_F_NEXT;
  gpu_q.desc[desc_list[1]].next = desc_list[2];

  gpu_m.desc_statut[desc_list[0]] = 0xff;//We set it one but if the operation was successful 
                                        //it will be set to zero by the device 
  gpu_q.desc[desc_list[2]].addr = (uint64_t) &gpu_m.desc_statut[desc_list[0]];
  gpu_q.desc[desc_list[2]].len = 1;
  gpu_q.desc[desc_list[2]].flags = VIRTQ_DESC_F_WRITE;
  gpu_q.desc[desc_list[2]].next = 0;
  
  // tell the device the first index in our chain of descriptors.
  gpu_q.available.ring[gpu_q.available.idx % NUMQ] = desc_list[0];
  // tell the device another avail ring entry is available.
  gpu_q.available.idx += 1; 
  *G(MMIO_QUEUE_NOTIFY) = 0;
  while(gpu_m.desc_statut[desc_list[0]] == 0xff){}
  free_list_desc(desc_list, 3);
  free(desc_list);
  return;
}

struct virtio_gpu_resp_display_info get_display_info(){
  //Request structs
  struct virtio_gpu_ctrl_hdr get_dis_req;
  memset(&get_dis_req, 0, sizeof(struct virtio_gpu_ctrl_hdr)); 
  get_dis_req.type = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;
  
  struct virtio_gpu_resp_display_info get_dis_res;
  memset(&get_dis_res, 0, sizeof(struct virtio_gpu_resp_display_info));    
 
  request_gpu(&get_dis_req, sizeof(struct virtio_gpu_ctrl_hdr), 
      &get_dis_res, sizeof(struct virtio_gpu_resp_display_info));
 
  assert(get_dis_res.hdr.type == VIRTIO_GPU_RESP_OK_DISPLAY_INFO);
  return get_dis_res;
}


void init_gpu_frame(void* frame){
  struct virtio_gpu_resource_create_2d req_create;
  memset(&req_create, 0, sizeof(struct virtio_gpu_resource_create_2d));
  req_create.hdr.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
  req_create.resource_id = 1;
  req_create.format = VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM;
  req_create.width = WIDTH;
  req_create.height = HEIGHT;
  struct virtio_gpu_ctrl_hdr res_create;
 
  request_gpu(&req_create, sizeof(struct virtio_gpu_resource_create_2d), 
      &res_create, sizeof(struct virtio_gpu_ctrl_hdr) - 4);
  assert(res_create.type == VIRTIO_GPU_RESP_OK_NODATA);

  struct virtio_gpu_resource_attach_backing base;
  base.hdr.type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
  base.resource_id = 1;
  base.nr_entries = 1;
  struct virtio_gpu_mem_entry mem_entry;
  mem_entry.addr = (uint64_t)frame;
  mem_entry.length = HEIGHT*WIDTH*sizeof(pixel); 
  mem_entry.padding = 0;

  uint64_t size_attach = sizeof(struct virtio_gpu_resource_attach_backing) +
       sizeof(struct virtio_gpu_mem_entry);
  void* req_attach = malloc(size_attach);
  if (req_attach == 0){return;}
  memset(req_attach, 0, size_attach);
  memcpy(req_attach, &base, sizeof(struct virtio_gpu_resource_attach_backing));
  memcpy((char*)req_attach+sizeof(struct virtio_gpu_resource_attach_backing), 
        &mem_entry, sizeof(struct virtio_gpu_mem_entry));  
  struct virtio_gpu_ctrl_hdr res_attach;
  request_gpu(req_attach, size_attach, 
      &res_attach, sizeof(struct virtio_gpu_ctrl_hdr)- 4);
  free(req_attach);
  assert(res_attach.type == VIRTIO_GPU_RESP_OK_NODATA);

  struct virtio_gpu_set_scanout req_display; 
  memset(&req_display, 0, sizeof(struct virtio_gpu_set_scanout));
  req_display.hdr.type = VIRTIO_GPU_CMD_SET_SCANOUT;
  req_display.r.x = 0;
  req_display.r.y = 0;
  req_display.r.width = WIDTH;
  req_display.r.height = HEIGHT;
  req_display.scanout_id = 0;
  req_display.resource_id = 1;
  struct virtio_gpu_ctrl_hdr res_display;
  request_gpu(&req_display, sizeof(struct virtio_gpu_set_scanout), 
      &res_display, sizeof(struct virtio_gpu_ctrl_hdr)- 4);
  // print_header_details(&res_display);
  assert(res_display.type == VIRTIO_GPU_RESP_OK_NODATA);
}


int invalidate_and_flush(int x, int y, int width, int height){
  if (check_cord(x, y, width, height)<0){
    return -1;
  }
  struct virtio_gpu_transfer_to_host_2d req_invalid;
  memset(&req_invalid, 0, sizeof(struct virtio_gpu_transfer_to_host_2d));
  req_invalid.hdr.type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
  req_invalid.r.x = x;
  req_invalid.r.y = y;
  req_invalid.r.width = width;
  req_invalid.r.height = height;
  req_invalid.resource_id = 1;
  req_invalid.offset = 0;
  struct virtio_gpu_ctrl_hdr res_invalid;
  request_gpu(&req_invalid, sizeof(struct virtio_gpu_transfer_to_host_2d), 
      &res_invalid, sizeof(struct virtio_gpu_ctrl_hdr)- 4);
  // print_header_details(&res_invalid);
  if (res_invalid.type != VIRTIO_GPU_RESP_OK_NODATA){
    return -1;
  }
  // printf("Pass this point\n");
  struct virtio_gpu_resource_flush req_flush;
  memset(&req_flush, 0, sizeof(struct virtio_gpu_resource_flush));
  req_flush.hdr.type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
  req_flush.r.x = x;
  req_flush.r.y = y;
  req_flush.r.width = width;
  req_flush.r.height = height;
  req_flush.resource_id = 1;
  struct virtio_gpu_ctrl_hdr res_flush;
  request_gpu(&req_flush, sizeof(struct virtio_gpu_resource_flush), 
      &res_flush, sizeof(struct virtio_gpu_ctrl_hdr)- 4);
  // print_header_details(&res_flush);
  if (res_flush.type != VIRTIO_GPU_RESP_OK_NODATA){
    return -1;
  }
  // get_display_info();
  return 0;
}

void update_all(){
  assert(invalidate_and_flush(0, 0, WIDTH, HEIGHT) == 0);
}

uint32_t* get_desc(uint16_t num){
  uint32_t* desc_ids = (uint32_t*)malloc(num * sizeof(uint32_t));;
  uint32_t nb_found = 0;
  uint32_t iter = 0;
  while(1){
    if (iter == NUMQ){
      //We did not find three desc
      //We free those who have been 
      //reserved
      if (nb_found > 0){
        int iter_free = 0;
        while(iter_free < nb_found){
          gpu_m.desc_avail[desc_ids[iter_free]] = 0;
          iter_free++;
        }
      }
      free(desc_ids);
      return 0;
    }
    if (gpu_m.desc_avail[iter] == 0){
        gpu_m.desc_avail[iter] = 1; //Must be atomic when working with multiple threads
        desc_ids[nb_found] = iter;
        nb_found++;
    }
    if (nb_found == num){
      return desc_ids;
    }
    iter++;
  }
}

void free_list_desc(uint32_t* list, int size){
  if (list != 0){
    int iter_free = 0;
    while(iter_free < size){
      gpu_m.desc_avail[list[iter_free]]=0;
      iter_free++;
    }
  }
}

int check_cord(int x, int y, int width, int height){
  if (x<0 || y<0 || width<0 || height<0){
    return -1;
  }
  if (x+width>WIDTH || y+height>HEIGHT){
    return -1;
  }
  return 0;
}

int update_data(void* data, int x, int y, int width, int height){
  if (check_cord(x, y, width, height)<0 && 
        HEIGHT*WIDTH*sizeof(pixel) > width*height*sizeof(pixel)){
    return -1;
  }
  memcpy(virt_gpu.frame+x*WIDTH+y, data, width*height*sizeof(pixel));
  return invalidate_and_flush(x, y, width, height);
}

void print_display_info() {
  struct virtio_gpu_resp_display_info dis = get_display_info();
  struct virtio_gpu_resp_display_info* info = &dis; 
  for (int i = 0; i < VIRTIO_GPU_USED_SCANOUT; i++) {
    const struct virtio_gpu_display_one* display = &info->pmodes[i];
    printf("Display %d:\n", i);
    printf("  Rect:\n");
    printf("   X: %u\n", display->r.x);
    printf("   Y: %u\n", display->r.y);
    printf("   Width: %u\n", display->r.width);
    printf("   Height: %u\n", display->r.height);
    printf("  Enabled: %u\n", display->enabled);
    printf("  Flags: 0x%x\n", display->flags);
  }
}


// Function to read and print gpu info
void print_virtio_gpu_config() {
  gpu_conf *config = (gpu_conf*) (char*)(VIRTIO1 + 0x100);
  printf("events_read: %u\n", config->events_read);
  printf("events_clear: %u\n", config->events_clear);
  printf("num_scanouts: %u\n", config->num_scanouts);
  printf("reserved: %u\n", config->reserved);
}

void print_header_details(struct virtio_gpu_ctrl_hdr *header){
  printf("Header:\n");
  printf("  Type: 0x%x\n", header->type);
  printf("  Flags: 0x%x\n", header->flags);
  printf("  Fence ID: 0x%lx\n", header->fence_id);
  printf("  Context ID: 0x%x\n", header->ctx_id);
  printf("  Padding: 0x%x\n", header->padding);
}

void get_display_info_virt(struct display_info* dis_ptr){
  if (dis_ptr == 0){
    return;
  }
  dis_ptr->width = WIDTH;
  dis_ptr->height = HEIGHT;
}

void display_boxes() {
  if (!virt_gpu.frame){return;}
  struct pixel* data = (pixel*)(virt_gpu.frame);
  // Initialize pixel colors
  unsigned char orientation = 6;
  orientation += 40;
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      if (x < WIDTH / 2 && y < HEIGHT / 2) {
        // Top-left corner: Red
        (data + y * WIDTH + x)->red = 255;
        (data + y * WIDTH + x)->green = 0;
        (data + y * WIDTH + x)->blue = orientation;
      } else if (x >= WIDTH / 2 && y < HEIGHT / 2) {
        // Top-right corner: Blue
        (data + y * WIDTH + x)->red = 0;
        (data + y * WIDTH + x)->green = orientation;
        (data + y * WIDTH + x)->blue = 255;
      } else if (x < WIDTH / 2 && y >= HEIGHT / 2) {
        // Bottom-left corner: Green
        (data + y * WIDTH + x)->red = 0;
        (data + y * WIDTH + x)->green = 255;
        (data + y * WIDTH + x)->blue = orientation;
      } else {
        // Remaining corner: Any color (e.g., yellow)
        (data + y * WIDTH + x)->red = 255;
        (data + y * WIDTH + x)->green = 255+orientation;
        (data + y * WIDTH + x)->blue = orientation;
      }
      (data + y * WIDTH + x)->alpha = 255; // Alpha component (transparency)
    }
  }
  // Ensure that orientation doesn't go beyond 255
  gpu_dev->update_data(data, 0, 0, WIDTH, HEIGHT);
} 

// Virtual gpu Device structure
gpu_device_t virt_gpu = {
  virt_gpu_init,
  WIDTH,
  HEIGHT,
  NULL,
  invalidate_and_flush,
  update_all,
  update_data,
  get_display_info_virt
};

