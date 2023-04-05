/**
* Projet PCSEA RISC-V
* Mehdi Frikha
* See license for license details.
*/

#ifndef _MEMORY_API_H_
#define _MEMORY_API_H_

#include "process.h"
#include "process_memory.h"
#include "hash.h"
#include "stddef.h"
#include "string.h"
#include "mem.h"
#include "stdlib.h"
#include "helperfunc.h"
#include "stdio.h"
#include "process.h"
#include <stdint.h>

/**
 * @brief Global shared pages parameters
 * @param page_id_counter Counter of shared pages
 * @param shared_memory_hash_table the hash table that will be used for the hsared pages
 * @param custom_release_process This will be used when we need to release the shared page from 
 * a custom process instead of the current process, the reason as to why this parameter was introduced is because we
 * can not add any parameters to system call thus we have to add this global parameter
 * when we are in the shm_release if this value is null we can use the current process and when it not null we use the 
 * this global process. 
 */
extern uint64_t page_id_counter; 
extern hash_t* shared_memory_hash_table;
extern process* custom_release_process;

typedef struct shared_page{
    char *page_key;
    void* page_address;
    int usage;
    uint64_t page_id;
} shared_page_t;



/**
 * @brief Initialises the shared pages table
 * 
 * @return int a positive value if the allocation was sucessful and negetive value otherwise
 */
extern int initialize_share_pages_table();

/**
 * @brief Create a frame that is associated to the key given as function argument,
 * the frame that has been created is a shared page that other processes can access and potentially modify
 * @note If the page exists(key has been mapped already) we return NULL, and if the key is not valid or 
 * there is free memory left we return NULL
 * @param key the key associated to the share page
 * @return void* The adress of the page is operation was succeful and a null pointer otherwise
 */
extern void *shm_create(const char *key);

/**
 * @brief This functions maps the page that is related to function argument to the process 
 * that called this method
 * 
 * @param key the char related to the page that will be shared from this function
 * @return void* returns the adress of the page if it is found 
 * and in the case it does not find the page it returns NULL
 */
void *shm_acquire(const char *key);

/**
 * @brief Releases the frame from the current process the function checks that the current process has acquired 
 * the shared page, if it did then the page unmapped for the current process. 
 * @note If the shared page usage go down to zero then the shared page is released
 * @param key the char associated with shared process
 */
void shm_release(const char *key);


#endif