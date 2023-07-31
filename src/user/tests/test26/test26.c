/*******************************************************************************
 * Ensimag - Projet Systeme 
 * Test 26
 * Directory Creation and Deletion Tests
 * This test program demonstrates basic directory creation and deletion operations
 * using C standard library functions. It creates nested directories up to a
 * maximum depth, and then recursively deletes them in reverse order.
 *
 * Test Functions:
 * 1. create_directory() - Creates a directory with the specified name.
 * 2. delete_directory() - Deletes a directory with the specified name.
 * 3. create_nested_directories() - Creates nested directories up to the given depth.
 * 4. delete_nested_directories() - Deletes nested directories up to the given depth.
 * 5. dir_test() - Performs the directory creation and deletion test.
 * 6. main() - Entry point of the program, calls dir_test() and checks the result.
 ******************************************************************************/

#include "sysapi.h"
#include "test26.h"

#define MAX_DEPTH 3   // Maximum depth of nested directories
#define NUM_DIRS 5   // Number of directories to create in each level

int create_directory(const char *dir_name) {
  if (mkdir(dir_name, 0) == -1) {
  printf("Error creating directory");
  return -1;
  }  
  return 0;
}

int delete_directory(const char *dir_name) {
  if (rmdir(dir_name) == -1) {
  printf("Error deleting directory");
  return -1;
  }
  return 0;
}

int create_nested_directories(const char *base_dir, int depth) {
  if (depth <= 0)
    return 0;

  char dir_name[50];
  // Create the directories at the current depth
  for (int i = 0; i < NUM_DIRS; i++) {
    memset(dir_name, 0, 50);
    snprintf(dir_name, sizeof(dir_name), "%s/dir%d", base_dir, i);
    assert(create_directory(dir_name) == 0);
  }
  // Create nested directories with decreased depth
  for (int i = 0; i < NUM_DIRS; i++) {
    snprintf(dir_name, sizeof(dir_name), "%s/dir%d", base_dir, i);
    create_nested_directories(dir_name, depth - 1);
  }

  return 0;
}

int delete_nested_directories(const char *base_dir, int depth) {
    if (depth <= 0)
        return -1;

    char dir_name[50];

    // Delete the directories at the current depth
    for (int i = NUM_DIRS - 1; i >= 0; i--) {
        snprintf(dir_name, sizeof(dir_name), "%s/dir%d", base_dir, i);
        delete_nested_directories(dir_name, depth - 1);
    }

    // Delete nested directories with decreased depth
    for (int i = 0; i < NUM_DIRS; i++) {
        snprintf(dir_name, sizeof(dir_name), "%s/dir%d", base_dir, i);
        assert(delete_directory(dir_name) == 0);
    }

    return 0;
}


int dir_test() {
  const char base_dir[] = "test_dir"; // Change this to your desired base directory name
  // Create the initial base directory
  assert(0 == create_directory(base_dir));
  // Create nested directories up to the maximum depth
  assert(0 == create_nested_directories(base_dir, MAX_DEPTH));
  // Delete all the nested directories in reverse order
  assert(0 == delete_nested_directories(base_dir, MAX_DEPTH));
  // Delete the initial base directory
  assert(0 == delete_directory(base_dir));
  printf("Test completed successfully.\n");
  return 0;
}

int main() {
 assert(dir_test() == 0);
 return 0;
}

