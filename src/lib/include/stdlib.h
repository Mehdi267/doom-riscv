#pragma once

#include <stddef.h>
#include <stdint.h>

__attribute__((noreturn)) void abort(void);
__attribute__((noreturn)) void exit(int status);

//######################################################
//##   Memory Management ##
//######################################################

/*
* @brief this macro is meant to be used in function witch returns -1 on failure
*/
#define secmalloc(p, n)                                                        \
  if (n >= 2147483647){                                                        \
    return -1;                                                                 \
  }                                                                            \
  p = malloc(n);                                                               \
  if (p == NULL)                                                               \
  return -1

void* malloc(size_t size);
void* calloc(size_t n_elements, size_t element_size);
void* memalign(size_t alignment, size_t size);
void free(void* ptr);
long int strtol(const char *nptr, char **endptr, int base);

//######################################################
//##   Utility functions ##
//######################################################

/**
 * @brief Prints the the area pointed at by data 
 * and lite by the size parameter in binary
 * @param data a pointer to the area that we wish to display
 * @param size how many bytes we wish to display
 */
void printb(const void* data, size_t size);

/**
 * @brief Prints the bytes a block(equal to the size param)
 * of data starting at the pointer data. 
 * @param data the location of the block 
 * @param size the size of the block
 */
void print_block(void* data, size_t size);



//######################################################
//##   Spin lock functions ##
//######################################################
/**
 * @brief Acquires a spin lock using inline assembly.
 *
 * This function attempts to acquire a spin lock by atomically swapping the value
 * at the specified lock address with a non-zero value. If the lock is already
 * acquired by another thread, it enters a loop and retries until the lock becomes
 * available.
 *
 * @param lock_address Pointer to the lock address.
 */
void spinlock_lock(int* lock_address);

/**
 * @brief Releases a spin lock using inline assembly.
 *
 * This function releases a spin lock by atomically swapping the value at the specified
 * lock address with zero. This action makes the lock available for other threads to acquire.
 *
 * @param lock_address Pointer to the lock address.
 */
void spinlock_unlock(int* lock_address);




//######################################################
//##   Disk operations functions ##
//######################################################

#define BLOCK_SIZE 512 

typedef enum operation_type {
  READ = 0,
  WRITE = 1,
} op_type;

typedef struct disk_operation{
  uint32_t blockNumber; //the disk sector that we will read from/write to
  op_type type; //Type of the disk operation(read or write)
  //The location in which we read data into
  //or the location of the data that will be 
  //written to the disk.
  char *data;
} disk_op;
