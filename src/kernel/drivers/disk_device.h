// See LICENSE for license details.
#include <stdlib.h>
#include <stdint.h>
#pragma once

typedef struct disk_device {
	void (*init)();
  int (*read_disk)(disk_op*);
  int (*write_disk)(disk_op*);
  uint32_t (*get_disk_size)();
} disk_device_t;

/*
 * disk_dev global variable
 * This variable is useful to init the disk on the machine.
 */
extern disk_device_t *disk_dev;

/*
 * disk drivers
 */
extern disk_device_t virt_disk;

void register_disk(disk_device_t *dev);
