/*******************************************************************************
 * Ensimag - Projet Systeme 
 * Test 26
 *
 ******************************************************************************/

// #include "sysapi.h"
// #include "test26.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>

// #define MAX_DEPTH 3    // Maximum depth of nested directories
// #define NUM_DIRS 10    // Number of directories to create in each level

// void create_directory(const char *dir_name) {
//     if (mkdir(dir_name, 0) == -1) {
//         perror("Error creating directory");
//         exit(EXIT_FAILURE);
//     }
// }

// void delete_directory(const char *dir_name) {
//     if (rmdir(dir_name) == -1) {
//         perror("Error deleting directory");
//         exit(EXIT_FAILURE);
//     }
// }

// void create_nested_directories(const char *base_dir, int depth) {
//     if (depth == 0)
//         return;

//     char dir_name[50];
//     for (int i = 0; i < NUM_DIRS; i++) {
//         snprintf(dir_name, sizeof(dir_name), "%s/dir%d", base_dir, i);
//         create_directory(dir_name);
//         create_nested_directories(dir_name, depth - 1);
//     }
// }

// void delete_nested_directories(const char *base_dir, int depth) {
//     if (depth == 0)
//         return;

//     char dir_name[50];
//     for (int i = 0; i < NUM_DIRS; i++) {
//         snprintf(dir_name, sizeof(dir_name), "%s/dir%d", base_dir, i);
//         delete_nested_directories(dir_name, depth - 1);
//         delete_directory(dir_name);
//     }
// }

// int dir_test() {
//     const char base_dir[] = "test_dir"; // Change this to your desired base directory name

//     // Create the initial base directory
//     create_directory(base_dir);

//     // Create nested directories up to the maximum depth
//     create_nested_directories(base_dir, MAX_DEPTH);

//     // Delete all the nested directories in reverse order
//     delete_nested_directories(base_dir, MAX_DEPTH);

//     // Delete the initial base directory
//     delete_directory(base_dir);

//     printf("Test completed successfully.\n");
//     return 0;
// }



int main() {
//  assert(dir_test() == 0);
 return 0;
}

