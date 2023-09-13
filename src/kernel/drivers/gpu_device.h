// See LICENSE for license details.
#include <stdlib.h>
#include <stdint.h>
#pragma once

struct display_info{
  int width;
  int height;
};

typedef struct gpu_device {
	void (*init)();
  int width;
  int height;
  void* frame;
  int (*inval_display)(int x, int y, int width, int height);
  void (*refresh_all)();
  int (*update_data)(void* data, int x, int y, int width, int height);
  void (*get_display_info)(struct display_info*);
} gpu_device_t;


/*
 * gpu_dev global variable
 * This variable is useful to init the gpu on the machine.
 */
extern gpu_device_t *gpu_dev;

/*
 * gpu drivers
 */
extern gpu_device_t virt_gpu;

void register_gpu(gpu_device_t *dev);
