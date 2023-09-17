#ifndef VIRT_GPU 
#define VIRT_GPU

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "virtio.h"

// Define the bit positions for each feature
#define VIRTIO_GPU_F_VIRGL_BIT   (0) // Bit position for VIRTIO_GPU_F_VIRGL
#define VIRTIO_GPU_F_EDID_BIT    (1) // Bit position for VIRTIO_GPU_F_EDID


#define WIDTH 320
#define HEIGHT 200

typedef struct pixel {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;
} pixel;

enum virtio_gpu_ctrl_type {
  /* 2d commands */
  VIRTIO_GPU_CMD_GET_DISPLAY_INFO = 0x0100,
  VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
  VIRTIO_GPU_CMD_RESOURCE_UNREF,
  VIRTIO_GPU_CMD_SET_SCANOUT,
  VIRTIO_GPU_CMD_RESOURCE_FLUSH,
  VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
  VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
  VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING,
  VIRTIO_GPU_CMD_GET_CAPSET_INFO,
  VIRTIO_GPU_CMD_GET_CAPSET,
  VIRTIO_GPU_CMD_GET_EDID,
  /* cursor commands */
  VIRTIO_GPU_CMD_UPDATE_CURSOR = 0x0300,
  VIRTIO_GPU_CMD_MOVE_CURSOR,
  /* success responses */
  VIRTIO_GPU_RESP_OK_NODATA = 0x1100,
  VIRTIO_GPU_RESP_OK_DISPLAY_INFO,
  VIRTIO_GPU_RESP_OK_CAPSET_INFO,
  VIRTIO_GPU_RESP_OK_CAPSET,
  VIRTIO_GPU_RESP_OK_EDID,
  /* error responses */
  VIRTIO_GPU_RESP_ERR_UNSPEC = 0x1200,
  VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY,
  VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID,
  VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID,
  VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID,
  VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER,
};

#define VIRTIO_GPU_FLAG_FENCE (1 << 0)
struct virtio_gpu_ctrl_hdr {
  uint32_t type;
  uint32_t flags;
  uint64_t fence_id;
  uint32_t ctx_id;
  uint32_t padding;
};

//----------CMD------------
/*
VIRTIO_GPU_CMD_GET_DISPLAY_INFO Retrieve the current output configuration. No request data (just
bare struct virtio_gpu_ctrl_hdr). Response type is VIRTIO_GPU_RESP_OK_DISPLAY_INFO, response
data is struct virtio_gpu_resp_display_info.
*/
#define VIRTIO_GPU_MAX_SCANOUTS 16
#define VIRTIO_GPU_USED_SCANOUT 1
#define VIRTIO_GPU_SCANOUTS 15
struct virtio_gpu_rect {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
};

struct virtio_gpu_resp_display_info {
  struct virtio_gpu_ctrl_hdr hdr;
  struct virtio_gpu_display_one {
    struct virtio_gpu_rect r; 
    uint32_t enabled;
    uint32_t flags;
  } pmodes[VIRTIO_GPU_SCANOUTS];
};

//----------CMD------------
//VIRTIO_GPU_CMD_GET_EDID
/*
  Retrieve the EDID data for a given scanout. Request data is struct virtio_-
  gpu_get_edid). Response type is VIRTIO_GPU_RESP_OK_EDID, response data is struct virtio_gpu_-
  resp_edid. Support is optional and negotiated using the VIRTIO_GPU_F_EDID feature flag.
*/
struct virtio_gpu_get_edid {
  struct virtio_gpu_ctrl_hdr hdr;
  uint32_t scanout;
  uint32_t padding;
};
struct virtio_gpu_resp_edid {
  struct virtio_gpu_ctrl_hdr hdr;
  uint32_t size;
  uint32_t padding;
  uint8_t edid[1024];
};

//----------CMD------------
//VIRTIO_GPU_CMD_RESOURCE_CREATE_2D 
/*
  Create a 2D resource on the host. Request data is struct
  virtio_gpu_resource_create_2d. Response type is VIRTIO_GPU_RESP_OK_NODATA
*/
enum virtio_gpu_formats {
  VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM = 1,
  VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM = 2,
  VIRTIO_GPU_FORMAT_A8R8G8B8_UNORM = 3,
  VIRTIO_GPU_FORMAT_X8R8G8B8_UNORM = 4,
  VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM = 67,
  VIRTIO_GPU_FORMAT_X8B8G8R8_UNORM = 68,
  VIRTIO_GPU_FORMAT_A8B8G8R8_UNORM = 121,
  VIRTIO_GPU_FORMAT_R8G8B8X8_UNORM = 134,
};

struct virtio_gpu_resource_create_2d {
  struct virtio_gpu_ctrl_hdr hdr;
  uint32_t resource_id;
  uint32_t format;
  uint32_t width;
  uint32_t height;
};

//----------CMD------------
//VIRTIO_GPU_CMD_RESOURCE_UNREF
/*
  Destroy a resource. Request data is struct virtio_gpu_resource_-
  unref. Response type is VIRTIO_GPU_RESP_OK_NODATA.
*/
struct virtio_gpu_resource_unref {
  struct virtio_gpu_ctrl_hdr hdr;
  uint32_t resource_id;
  uint32_t padding;
};

//----------CMD------------
//VIRTIO_GPU_CMD_SET_SCANOUT Set the scanout parameters for a single output.
//Request data is :
//struct virtio_gpu_set_scanout. Response type is VIRTIO_GPU_RESP_OK_NODATA.
struct virtio_gpu_set_scanout {
  struct virtio_gpu_ctrl_hdr hdr;
  struct virtio_gpu_rect r;
  uint32_t scanout_id;
  uint32_t resource_id;
};

//----------CMD------------
// VIRTIO_GPU_CMD_RESOURCE_FLUSH
struct virtio_gpu_resource_flush {
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_rect r;
    uint32_t resource_id;
    uint32_t padding;
};
// This flushes a resource to the screen.

//----------CMD------------
// VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D
struct virtio_gpu_transfer_to_host_2d {
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_rect r;
    uint64_t offset;
    uint32_t resource_id;
    uint32_t padding;
};
// This transfers data from guest memory to a host resource.

//----------CMD------------
// VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING
struct virtio_gpu_resource_attach_backing {
    struct virtio_gpu_ctrl_hdr hdr;
    uint32_t resource_id;
    uint32_t nr_entries;
};
struct virtio_gpu_mem_entry {
    uint64_t addr;
    uint32_t length;
    uint32_t padding;
};
// This assigns backing pages to a resource.

//----------CMD------------
// VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING
struct virtio_gpu_resource_detach_backing {
    struct virtio_gpu_ctrl_hdr hdr;
    uint32_t resource_id;
    uint32_t padding;
};
// This detaches backing pages from a resource.


//######################################################
//##   disk management and operations ##
//######################################################

struct gpu_management {
  uint32_t used_iter;
  uint8_t desc_avail[NUMQ];
  uint8_t desc_statut[NUMQ];
} gpu_m;


//######################################################
//##   config struct ##
//######################################################

#define VIRTIO_GPU_EVENT_DISPLAY (1 << 0)
typedef struct virtio_gpu_config {
  uint32_t events_read;
  uint32_t events_clear;
  uint32_t num_scanouts;
  uint32_t reserved;
} gpu_conf;

/**
 * @brief This function display the properties 
 * of the disk
 */
extern void print_virtio_gpu_config();

/**
 * @brief Free a list of descpritors present in a list
 * @param list the list that containt the desciptors
 */
void free_list_desc(uint32_t* list, int size);

/**
 * @brief Reserves num descriptors
 * @return uint32_t* address of the reserved descriptors
 */
uint32_t* get_desc(uint16_t num);

/**
 * @brief Get display information.
 *
 * @return The display information structure.
 */
struct virtio_gpu_resp_display_info get_display_info();

/**
 * @brief Print display information.
 */
void print_display_info();

/**
 * @brief Print details of a response header.
 *
 * @param header The response header to print details of.
 */
void print_header_details(struct virtio_gpu_ctrl_hdr *header);

/**
 * @brief Send a GPU request.
 *
 * @param req       Pointer to the request data.
 * @param req_size  Size of the request data.
 * @param res       Pointer to the response data.
 * @param res_size  Size of the response data.
 */
void request_gpu(void *req, uint64_t req_size, void *res, uint64_t res_size);

/**
 * @brief Initialize the GPU frame and connection.
 */
void init_gpu_frame();

/**
 * @brief Invalidate and flush a memory region.
 *
 * @param x      X-coordinate.
 * @param y      Y-coordinate.
 * @param width  Width of the region.
 * @param height Height of the region.
 *
 * @return 0 if successful, or an error code.
 */
int invalidate_and_flush(int x, int y, int width, int height);

/**
 * @brief Update the entire screen.
 */
void update_all();

/**
 * @brief Update a frame with data.
 *
 * @param data   Pointer to the data to update with.
 * @param x      X-coordinate.
 * @param y      Y-coordinate.
 * @param width  Width of the region.
 * @param height Height of the region.
 *
 * @return 0 if successful, or an error code.
 */
int update_data(void *data, int x, int y, int width, int height);

/**
 * @brief Check if coordinates are valid.
 *
 * @param x      X-coordinate.
 * @param y      Y-coordinate.
 * @param width  Width of the region.
 * @param height Height of the region.
 *
 * @return 0 if valid, negative otherwise.
 */
int check_cord(int x, int y, int width, int height);

/**
 * @brief Get information about the current display.
 *
 * @param info Pointer to a display information structure to fill.
 */
void get_display_info_virt(struct display_info *info);

/**
 * @brief Display some boxes.
 */
void display_boxes();

#endif