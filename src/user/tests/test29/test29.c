/*******************************************************************************
 * Ensimag - Projet Systeme 
 * Test 29
 *
 * Test 29 - docs
 *
 ******************************************************************************/

#include "sysapi.h"
#include "test29.h"


int dup_test() {
  int originalFileDescriptor, duplicateFileDescriptor, secondDuplicateFileDescriptor;
  char buffer[1024];
  ssize_t bytesRead, bytesWritten;
  char original_name[] = "Hello World";

  // Test with dup
  originalFileDescriptor = open("example_dup.txt", O_RDWR | O_CREAT | O_TRUNC, 0);
  printf("originalFileDescriptor : %d\n", originalFileDescriptor);
  assert(originalFileDescriptor != -1);

  bytesWritten = write(originalFileDescriptor, "Hello", 5);  // Write "Hello" to the original file
  assert(bytesWritten == 5);

  // Duplicate the file descriptor
  duplicateFileDescriptor = dup(originalFileDescriptor);
  printf("duplicateFileDescriptor (dup): %d\n", duplicateFileDescriptor);
  assert(duplicateFileDescriptor != -1);

  // Write " World" to the duplicate file descriptor
  bytesWritten = write(duplicateFileDescriptor, " World", 6);
  assert(bytesWritten == 6);

  // Read from the original file descriptor
  lseek(originalFileDescriptor, 0, SEEK_SET);  // Move the file offset to the beginning
  bytesRead = read(originalFileDescriptor, buffer, strlen(original_name));
  assert(bytesRead == 11);  // 5 bytes from "Hello" + 6 bytes from " World"

  buffer[bytesRead] = '\0';
  printf("Content of the original file (dup): %s\n", buffer);
  assert(0 == memcmp(original_name, buffer, strlen(original_name)));

  // Close both file descriptors
  close(originalFileDescriptor);
  // Read from the original file descriptor
  lseek(duplicateFileDescriptor, 0, SEEK_SET);  // Move the file offset to the beginning
  bytesRead = read(duplicateFileDescriptor, buffer, strlen(original_name));
  assert(bytesRead == 11);  // 5 bytes from "Hello" + 6 bytes from " World"

  buffer[bytesRead] = '\0';
  printf("Content of the original dup file 1: %s\n", buffer);
  assert(0 == memcmp(original_name, buffer, strlen(original_name)));
  close(duplicateFileDescriptor);

  // Test with dup2
  originalFileDescriptor = open("example_dup2.txt", O_RDWR | O_CREAT | O_TRUNC, 0);
  printf("originalFileDescriptor : %d\n", originalFileDescriptor);
  assert(originalFileDescriptor != -1);

  bytesWritten = write(originalFileDescriptor, "Hello", 5);  // Write "Hello" to the original file
  assert(bytesWritten == 5);

  // Duplicate the file descriptor using dup2 to a specific file descriptor
  secondDuplicateFileDescriptor = dup2(originalFileDescriptor, 10);
  printf("duplicateFileDescriptor (dup): %d\n", secondDuplicateFileDescriptor);
  assert(secondDuplicateFileDescriptor != -1);

  // Write " World" to the duplicate file descriptor
  bytesWritten = write(secondDuplicateFileDescriptor, " World", 6);
  assert(bytesWritten == 6);

  // Read from the original file descriptor
  lseek(originalFileDescriptor, 0, SEEK_SET);  // Move the file offset to the beginning
  bytesRead = read(originalFileDescriptor, buffer, strlen(original_name));
  assert(bytesRead == 11);  // 5 bytes from "Hello" + 6 bytes from " World"

  buffer[bytesRead] = '\0';
  printf("Content of the original file (dup2): %s\n", buffer);
  assert(0 == memcmp(original_name, buffer, strlen(original_name)));

  // Close both file descriptors
  close(originalFileDescriptor);
  // Read from the original file descriptor
  lseek(secondDuplicateFileDescriptor, 0, SEEK_SET);  // Move the file offset to the beginning
  bytesRead = read(secondDuplicateFileDescriptor, buffer, strlen(original_name));
  assert(bytesRead == 11);  // 5 bytes from "Hello" + 6 bytes from " World"

  buffer[bytesRead] = '\0';
  printf("Content of the original dup 2 file (dup2): %s\n", buffer);
  assert(0 == memcmp(original_name, buffer, strlen(original_name)));
  close(secondDuplicateFileDescriptor);

  if (unlink("example_dup.txt") < 0) {
    printf("Error unlinking example_dup.txt");
    return -1;
  }
  if (unlink("example_dup2.txt") < 0) {
    printf("Error unlinking example_dup.txt");
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
  assert(dup_test() == 0);
  return 0;
}
