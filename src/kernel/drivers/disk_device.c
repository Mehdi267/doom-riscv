// See LICENSE for license details.

#include "stdlib.h"
#include "disk_device.h"

// See LICENSE for license details.

disk_device_t disk_none = {
		.init = NULL,
		.read_disk = NULL,
		.write_disk = NULL,
    .get_disk_size = NULL,
    .print_disk_info = NULL
};

disk_device_t *disk_dev = &disk_none;

void register_disk(disk_device_t *dev)
{
	disk_dev = dev;
	if(dev->init)
	{
		dev->init();
	}
}

