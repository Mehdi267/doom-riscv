#include "stdint.h"
#include "fs_api.h"
#include "stdbool.h"
#ifndef MY_CHDEV_H
#define MY_CHDEV_H


//This file is used by character devices
//these two functions will called by the read and 
//open syscalls when we work these files
//This architecture was slightly inspired 
//by xv6, minix and linux operating system,
typedef struct chdevop {
  bool reserved;
  long (*read)(uint64_t buffer_add, int bytes);
  long (*write)(uint64_t buffer_add, int bytes);
} chdevop;

extern chdevop dev_op[];
#define TERMINAL_DEVICE 1

/**
 * @brief Create a special file (character device or block device) with the specified pathname.
 *
 * The `mknod` function is used to create a special file, including character devices and block devices(working on it).
 *
 * @param pathname The path of the special file to be created.
 * @param mode The permission mode for the special file.(nt working atm)
 * @param dev The device number associated with the special file.
 *            For character devices, the `dev` parameter represents the major and minor numbers
 *            that uniquely identify the device. For block devices, `dev` specifies the device number.
 *
 * @return On success, 0 is returned. On failure, -1 is returned
 */
int mknod(const char *pathname, mode_t mode, dev_t dev);

/**
 * @brief Add file system driver details
 * @return int status
 */
int init_fs_drivers();

#endif // MY_CHDEV_H
