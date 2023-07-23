/**
* Projet PCSEA RISC-V
* Mehdi Frikha
* See license for license details.
*/

#include "helperfunc.h"
#include "process.h"
#include "memory_api.h"
#include "stddef.h"

#include "stdio.h"
#include "assert.h"
#include "stddef.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "riscv.h"


void* cast_int_to_pointer(int int_to_cast){
    return (void*)((long) int_to_cast);
}


void* cast_char_star_into_pointer(char* char_star){
    return (void*) char_star;
}


long cast_pointer_into_a_long(void * pointer){
    return (long) pointer;
}


int cast_pointer_into_a_int(void * pointer){
    return (int)((long) pointer);
}


void set_supervisor_interrupts(bool val){
    if (val){
        csr_set(sstatus, MSTATUS_SIE);
    }
    else{
        csr_clear(sstatus, MSTATUS_SIE);
    }
}


process* get_process_struct_of_pid(int pid){
    process* process_pid = ((process*) hash_get(get_process_hash_table(), cast_int_to_pointer(pid), NULL));
    return process_pid;
}

process* get_current_process() {
  return ((process*) hash_get(get_process_hash_table(), cast_int_to_pointer(getpid()), NULL));
}

semaphore_t* get_semaphore_struct(int sem){
    semaphore_t* sem_val = ((semaphore_t*) hash_get(get_semaphore_table(), cast_int_to_pointer(sem), NULL));
    return sem_val;
}


char* getname(void){
    process* currently_running_process = ((process*) hash_get(get_process_hash_table(), cast_int_to_pointer(getpid()), NULL));
    if (currently_running_process == NULL){
        return NULL;
    }
    return currently_running_process->process_name;
}


char* get_pid_name(int pid){
    process* process_pid = ((process*) hash_get(get_process_hash_table(), cast_int_to_pointer(pid), NULL));
    if (process_pid == NULL){
        return NULL;
    }
    return process_pid->process_name;
}

int save_pid(int pid){
  id_list_t* node = (id_list_t*)malloc(sizeof(id_list_t));
  if (!node){
    return -1;
  }
  node->id = pid;
  if (proc_mang_g.process_id_list){
    node->next_id = proc_mang_g.process_id_list;
  }
  else{
    node->next_id = NULL;
  }
  proc_mang_g.process_id_list = node;
  return 0;
}

int get_pid_iterator(){
  return proc_mang_g.pid_iterator;
} 

int increment_pid_and_get_new_pid(){
  if (proc_mang_g.process_id_list != 0){
    int return_pid = proc_mang_g.process_id_list->id; 
    proc_mang_g.process_id_list = proc_mang_g.process_id_list->next_id;
    return return_pid;
  }
  proc_mang_g.pid_iterator++;
  return proc_mang_g.pid_iterator;
}

int increment_shared_page_counter(){
    page_id_counter++;
    return page_id_counter;
}

int increment_fd_counter(){
  proc_mang_g.fd_counter++;
  return proc_mang_g.fd_counter;
}

int increment_semaphore_id(){
    semaphore_id_counter++;
    return semaphore_id_counter;
} 

hash_t* get_shared_pages_hash_table(void){
    return shared_memory_hash_table;
}

hash_t* get_process_hash_table(void){
    return proc_mang_g.pid_process_hash_table;
}

hash_t* get_semaphore_table(void){
    return semaphore_table;
}

process* get_custom_release_process(void){
    return custom_release_process;
}

void set_custom_release_process(process* proc){
    custom_release_process = proc;
    return;
}


int validate_action_process_valid(process* process_pid){
    if (process_pid == NULL){
        return -1;
    }
    if (   process_pid->state == ZOMBIE
        || process_pid->state == KILLED){
        return -1;
    }
    return 0;
}


void print_pte(page_table_entry *pte)
{
    print_memory_no_arg("----------------Pte values start----------------\n");
    debug_print_memory("valid = %d \n",pte->valid);
    debug_print_memory("read = %d \n",pte->read);
    debug_print_memory("write = %d \n",pte->write);
    debug_print_memory("exec = %d \n",pte->exec);
    debug_print_memory("res = %d \n",pte->resU);
    debug_print_memory("global = %d \n",pte->global);
    debug_print_memory("resA = %d \n",pte->resA);
    debug_print_memory("resD = %d \n",pte->resD);
    debug_print_memory("rsw = %d \n",pte->rsw);
    debug_print_memory("ppn0 = %d \n",pte->ppn0);
    debug_print_memory("ppn1 = %d \n",pte->ppn1);
    debug_print_memory("ppn2 = %d \n",pte->ppn2);
    debug_print_memory("reserved = %d \n",pte->reserved);
    debug_print_memory("pbmt = %d \n",pte->pbmt);
    debug_print_memory("n = %d \n",pte->n);
    print_memory_no_arg("----------------Pte values end----------------\n");
}

/**
 * @brief Prints the details of a shared proc
 * 
 * @param node 
 */
void print_shared_page_node(char* text_print ,shared_pages_proc_t* node){
    if (node==NULL)
        return;
    debug_print_memory_api("---------Printing shared page : [%s] ---------\n",node->key);
    if (text_print!=NULL){
        debug_print_memory_api("---------%s---------\n",text_print);
    }
    debug_print_memory_api("lvl0_index = %d \n",node->lvl0_index);
    debug_print_memory_api("lvl1_index = %d \n",node->lvl1_index);
    debug_print_memory_api("lvl2_index = %d \n",node->lvl2_index);
    debug_print_memory_api("page_table = %p \n",node->page_table);
    if (text_print!=NULL){
        debug_print_memory_api("---------%s---------\n",text_print);
    }
}

void print_process_state(process_state state){
  if (state == ACTIF){
    printf("ACTIF");
  }
  if (state == ACTIVATABLE){
    printf("ACTIVATABLE");
  }
  if (state == BLOCKEDSEMAPHORE){
    printf("BLOCKEDSEMAPHORE");
  }
  if (state == BLOCKEDIO){
    printf("BLOCKEDIO");
  }
  if (state == BLOCKEDQUEUE){
    printf("BLOCKEDQUEUE");
  }
  if (state == BLOCKEDWAITCHILD){
    printf("BLOCKEDWAITCHILD");
  }
  if (state == ASLEEP){
    printf("ASLEEP");
  }
  if (state == ZOMBIE){
    printf("ZOMBIE");
  }
  if (state == KILLED){
    printf("KILLED");
  }
}

bool is_absolute_directory(const char* path) {
    // Check if the path is not NULL and not an empty string.
    if (path && *path != '\0') {
        // An absolute directory starts with a forward slash ('/') or a drive letter (on Windows).
        // For this example, we assume a Unix-like file system with a forward slash.
        if (*path == '/')
            return true;
        else
            return false;
    }
    // If the path is NULL or an empty string, it's not absolute.
    return false;
}

void extractFolders(const char* path) {
    char tempPath[256]; // Assuming the maximum path length is 255 characters
    char* folder;
    const char* delimiters = "\\/"; // Delimiters: both forward and backslashes

    // Make a copy of the input path to avoid modifying the original string
    strncpy(tempPath, path, sizeof(tempPath) - 1);
    tempPath[sizeof(tempPath) - 1] = '\0'; // Ensure null-termination

    printf("Path: %s\n", path);
    printf("Folders:\n");

    folder = strtok(tempPath, delimiters); // Split path by delimiters
    while (folder != NULL) {
        printf("%s\n", folder);
        folder = strtok(NULL, delimiters);
    }
}

path_fs* extract_files(const char* path) {
  char tempPath[256]; // Assuming the maximum path length is 255 characters
  const char* delimiters = "\\/"; // Delimiters: both forward and backslashes

  // Make a copy of the input path to avoid modifying the original string
  strncpy(tempPath, path, sizeof(tempPath) - 1);
  tempPath[sizeof(tempPath) - 1] = '\0'; // Ensure null-termination

  // Count the number of elements
  uint32_t count = 0;
  char* token = strtok(tempPath, delimiters); // Split path by delimiters
  while (token != NULL) {
    count++;
    token = strtok(NULL, delimiters);
  }

  // Allocate memory for the path_fs structure
  path_fs* result = (path_fs*)malloc(sizeof(path_fs));
  if (result == NULL) {
    printf("Memory allocation failed");
    return NULL;
  }

  // Allocate memory for the files array and fill it with folder and file names
  result->files = (char**)malloc(count * sizeof(char*));
  if (result->files == NULL) {
    printf("Memory allocation failed");
    free(result);
    return NULL;
  }

  // Reset tempPath and tokenize it again to fill the files array
  strncpy(tempPath, path, sizeof(tempPath) - 1);
  tempPath[sizeof(tempPath) - 1] = '\0';
  uint32_t i = 0;
  token = strtok(tempPath, delimiters);
  while (token != NULL) {
    result->files[i] = malloc(strlen(token)); // Copy the token into the files array
    memcpy(result->files[i], token, strlen(token));
    token = strtok(NULL, delimiters);
    i++;
  }

  result->nb_files = count;
  return result;
}

void free_path_fs(path_fs* path) {
  if (path == NULL) {
    return;
  }
  for (uint32_t i = 0; i < path->nb_files; i++) {
    free(path->files[i]);
  }
  free(path->files);
  free(path);
}
