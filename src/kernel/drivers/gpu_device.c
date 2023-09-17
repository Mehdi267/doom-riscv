// See LICENSE for license details.

#include "stdlib.h"
#include "gpu_device.h"
#include "stdio.h"
// See LICENSE for license details.

gpu_device_t gpu_none = {
  .init = NULL,
  .frame = NULL,
  .inval_display = NULL,
  .refresh_all = NULL,
  .update_data = NULL,
  .get_display_info = NULL,
};

gpu_device_t *gpu_dev = &gpu_none;

void register_gpu(gpu_device_t *dev)
{
	gpu_dev = dev;
	if(dev->init){
		if (dev->init()<0){
	    gpu_dev = &gpu_none;
      PRINT_RED("[GPU]Gpu failed\n");
    }else{
      PRINT_GREEN("[GPU]Gpu configured\n");
    }
	}
}

