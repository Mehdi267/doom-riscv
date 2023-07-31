/*******************************************************************************
 * Ensimag - Projet Systeme 
 * Test 28
 *
 * Test 28 - Folder Creation, File I/O, Hard Link Test, and Error Handling
 *
 * This program demonstrates folder creation, file I/O, hard link creation, and error handling
 * using various system calls. It creates a folder named "test28" and enters it. Inside "test28",
 * it creates a file named "testfile.txt" and writes data into it. It then creates a hard link with
 * the same data. The program then reads data from both the original file and the hard link and
 * verifies that the data matches. After that, it unlinks the hard link and deletes the directory
 * "test28".
 *
 * Test Functions:
 * 1. createAndWriteToFile() - Creates a file "testfile.txt" and writes data into it.
 * 2. createHardLinkAndVerify() - Creates a hard link with the same data as the original file
 *   and verifies that the data in the hard link matches the original data.
 * 3. test28() - Performs the folder creation, file I/O, hard link creation, and verification.
 * 4. main() - Entry point of the program, calls test28() and checks for errors in system calls.
 ******************************************************************************/

#include "sysapi.h"
#include "test28.h"

/**
 * Creates a file named "testfile.txt" inside the current working directory and writes data into it.
 *
 * @return 0 on success, -1 on failure.
 */
int createAndWriteToFile() {
  int fd = open("testfile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0);
  if (fd < 0) {
    printf("Error creating testfile.txt");
    return -1;
  }

  const char* data = "This is the test data.\n";
  ssize_t bytes_written = write(fd, data, strlen(data));
  if (bytes_written < 0) {
    printf("Error writing to testfile.txt");
    close(fd);
    return -1;
  }
  close(fd);
  printf("testfile.txt was created\n");
  return 0;
}

/**
 * Creates a hard link "testfile_link.txt" with the same data as the original "testfile.txt"
 * and verifies that the data in the hard link matches the original data.
 *
 * @return 0 on success, -1 on failure.
 */
int createHardLinkAndVerify() {
  if (link("testfile.txt", "testfile_link.txt") < 0) {
    printf("Error creating hard link");
    return -1;
  }

  char original_data[100];
  char link_data[100];

  int original_fd = open("testfile.txt", O_RDONLY, 0);
  if (original_fd < 0) {
    printf("Error opening testfile.txt");
    return -1;
  }

  int link_fd = open("testfile_link.txt", O_RDONLY, 0);
  if (link_fd < 0) {
    printf("Error opening testfile_link.txt");
    close(original_fd);
    return -1;
  }

  ssize_t original_bytes_read = read(original_fd, original_data, sizeof(original_data));
  if (original_bytes_read < 0) {
    printf("Error reading from testfile.txt");
    close(original_fd);
    close(link_fd);
    return -1;
  }

  ssize_t link_bytes_read = read(link_fd, link_data, sizeof(link_data));
  if (link_bytes_read < 0) {
    printf("Error reading from testfile_link.txt");
    close(original_fd);
    close(link_fd);
    return -1;
  }

  if (original_bytes_read != link_bytes_read ||
     memcmp(original_data, link_data, 
      (long unsigned int) original_bytes_read) != 0) {
    printf("Data mismatch between original and hard link");
    close(original_fd);
    close(link_fd);
    return -1;
  }

  close(original_fd);
  close(link_fd);
  return 0;
}

int check_hard_link_data(){
  char link_data[100];
  int link_fd = open("testfile_link.txt", O_RDONLY, 0);
  if (link_fd < 0) {
    printf("Error opening testfile_link.txt");
    return -1;
  }

  ssize_t link_bytes_read = read(link_fd, link_data, sizeof(link_data));
  if (link_bytes_read < 0) {
    printf("Error reading from testfile_link.txt");
    close(link_fd);
    return -1;
  }

  const char* data = "This is the test data.\n";
  if (memcmp(data, link_data, strlen(data)) != 0) {
    printf("Data mismatch between original and hard link");
    close(link_fd);
    return -1;
  }

  return close(link_fd);
}

/**
 * The test function that performs the folder creation, file I/O, hard link creation, and verification.
 *
 * @return 0 on success, -1 on failure.
 */
int test28() {
  // Create test28 folder
  if (mkdir("test28", 0) < 0) {
    printf("Error creating test28 directory");
    return -1;
  }

  // Move to test28 folder
  if (chdir("test28") < 0) {
    printf("Error changing directory to test28");
    return -1;
  }

  // Create and write data to testfile.txt
  if (createAndWriteToFile() < 0) {
    return -1;
  }

  // Create a hard link with the same data and verify
  if (createHardLinkAndVerify() < 0) {
    return -1;
  }

  // Unlink the man file
  if (unlink("testfile.txt") < 0) {
      printf("Error unlinking testfile_link.txt");
      return -1;
  }

  if (check_hard_link_data()<0){
    printf("Error reading testfile_link.txt");
    return -1;  
  }

  if (unlink("testfile_link.txt") < 0) {
    printf("Error unlinking testfile_link.txt");
    return -1;
  }

  // Move back to the initial directory and delete test28 folder
  if (chdir("..") < 0) {
    printf("Error changing directory back");
    return -1;
  }

  if (rmdir("test28") < 0) {
    printf("Error deleting test28 directory");
    return -1;
  }

  return 0;
}

/**
 * The entry point of the program.
 *
 * This function calls the test28() function and checks for errors in system calls.
 * If any system call returns a negative value, it returns -1 to indicate an error.
 *
 * @return 0 on success, -1 on failure.
 */
int main() {
  assert(test28() == 0);
  return 0;
}
