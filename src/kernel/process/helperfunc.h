/**
* Projet PCSEA RISC-V
* Mehdi Frikha
* See license for license details.
*/

#ifndef _HELPFUNCTIONS_H_
#define _HELPFUNCTIONS_H_

#define PATH_SIZE_MAX 512

#include "stdint.h"
#include "process.h"
#include "../memory/pages.h"
#include "../sync/semaphore_api.h"
#include <stdint.h>
#define CHECK_AND_RET_NEG_ONE(var) ({ \
    typeof(var) result = (var); \
    if (result == 0) { \
        return -1; \
    } \
})
#define CHECK_AND_RET_Zero(var) ({ \
    typeof(var) result = (var); \
    if (result == 0) { \
        return 0; \
    } \
})

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * @brief Casts an int to a void * pointer 
*/
extern void* cast_int_to_pointer(int);

/**
 * @brief Casts a char* into a void* 
 * @param char_star the char to cast 
 * @return void* 
 */
void* cast_char_star_into_pointer(char* char_star);

/**
 * @brief Casts a pointer into an long
*/
extern long cast_pointer_into_a_long(void * pointer);

/**
 * @brief Casts a pointer into an int
*/
extern int cast_pointer_into_a_int(void * pointer);


/**
 * @brief returns the struct process that holds the pid given as the function argument pid
 * @param pid id of the process that we want to get its struct
 * @return the process struct that hold pid argument or null if it can't find the struct 
*/
extern process* get_process_struct_of_pid(int pid);

/**
 * @brief Returns the name of the currently running process
 * @return the name of the currently running process
 */
extern char* getname(void);

/**
 * @brief Returns the name of the process with with the pid given
 * as the function parameter
 * @param pid id of the process that we want to print its name 
 * @return the name of the process with the pid given ad the function argument or NULL if it can't find it
 */
extern char* get_pid_name(int pid);

/**
 * @brief print_pte display a page table entry in a human readable format
 * displaying the value associated to every segement 
 * @param pte the page table entry that we canto display
 */
extern void print_pte(page_table_entry *pte);   


/**
 * @brief Increment the pid and returns the new value of 
 * the pid
*/
extern int increment_pid_and_get_new_pid();

/**
 * @brief Increment the shared page table counter and returns the new value of the counter; 
 * @return int 
 */
extern int increment_shared_page_counter();

/**
 * @brief Increments the semaphore id counter, used when creating new semaphores  
 * @return int 
 */
extern int increment_semaphore_id();

/**
 * @brief Returns the hash table tha map id with process structs
*/
extern hash_t* get_process_hash_table(void);

/**
 * @brief Get the shared pages hash table object
 * 
 * @return hash_t*  
 */
extern hash_t* get_shared_pages_hash_table(void);

/**
 * @brief Get the semaphore hash table
 * @return hash_t* the semaphore hash table
 */
extern hash_t* get_semaphore_table(void);

/**
 * @brief Get the semaphore struct object from the hash table 
 * of the semaphore
 * 
 * @param sem the id of the semaphore
 * @return semaphore_t* semaphore information
 */
extern semaphore_t* get_semaphore_struct(int sem);

/**
 * @brief Get the custom release process
 * 
 * @return process* 
 */
extern process* get_custom_release_process(void);

/**
 * @brief Set the custom release process to a value given as 
 * a function argument
 * @param proc the new value tthat will assocaited to the custom_release_process 
 */
extern void set_custom_release_process(process* proc);

/**
 * @brief Validate that the process is an state on which we can call system calls
 * we will mostly check that the process is not a ZOMBIE but the function can be extended to 
 * other states
*/
extern int validate_action_process_valid(process* process_pid);

// mask an adress with a long int
#define MASK_ADDRESS(a, i) ((unsigned long int)(a) & (i));
/**
 * @brief set supervisor interrupts to the value given a fnction argument
*/
extern void set_supervisor_interrupts(bool val);


/**
 * @brief returns the process struct of the current process
*/
process* get_current_process();

/**
 * @brief Outputs the details of a shared page
 * 
 * @param text_print 
 * @param node 
 */
extern void print_shared_page_node(char* text_print ,shared_pages_proc_t* node);

/**
 * @brief Saves process pid 
 * @param pid the pid to save 
 * @return int 0 if sucessful and -1 if not
 */
extern int save_pid(int pid);

/**
 * @brief Get the pid iterator 
 * @return int the pid iterator
 */
extern int get_pid_iterator();

/**
 * @brief Print process state 
 * @param state process state 
 */
extern void print_process_state(process_state state);


/**
 * @brief Checks if the path given is an absolute directory 
 * or a relative one 
 * @param path the path that we could like to verify
 * @return true it is an absolute directory
 * @return false it is not an absolute directory
 */
bool is_absolute_directory(const char* path);


typedef struct path_files{
  char** files;
  uint32_t nb_files;
} path_fs;


/**
 * @brief Extracts folder and file names from the given path.
 *
 * This function takes a path as input and extracts individual folder and file
 * names from it. It tokenizes the path using forward slashes ('/') and
 * backslashes ('\') as delimiters. It dynamically allocates memory for the
 * result and returns a pointer to a `path_fs` structure containing the
 * extracted names and the number of elements.
 *
 * @param path The input path from which to extract folder and file names.
 * @return A pointer to the `path_fs` structure containing the extracted names
 *         and the number of elements. The memory for the structure and its
 *         `files` array is dynamically allocated and must be freed using the
 *         `free_path_fs` function after use. If the memory allocation fails
 *         or the input `path` is NULL, the function returns NULL.
 */
path_fs* extract_files(const char* path);

/**
 * @brief Prints path data of the path_fs struct given as argument 
 * @param path_data the struct that we will read
 */
void print_path_data(path_fs* path_data);

/**
 * @brief Frees the memory allocated for the `path_fs` structure.
 *
 * This function frees the dynamically allocated memory for the `path_fs`
 * structure and its `files` array. It also frees each individual folder and
 * file name stored in the `files` array. If the input `path` is NULL, this
 * function has no effect.
 *
 * @param path A pointer to the `path_fs` structure to be freed.
 */
void free_path_fs(path_fs* path);

/**
 * @brief Create a new string for the start of the string pointer 
 * until we encounter a "\0"
 * @param str the old string
 * @return char* new allocated string 
 */
char* create_substring(const char* str);
#endif
