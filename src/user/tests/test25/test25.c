/******************************************************************************
 * Ensimag - Projet Systeme 
 * Test 25
 *
 * File Operations and Error Handling Tests
 * This test program demonstrates basic file operations such as opening files
 * (creating if they do not exist), writing data to files, and reading data 
 * from files. It also tests error handling scenarios for file operations.
 *
 * Test Functions:
 * 1. test_read_empty_file() - Reads from a newly created or existing file and 
 *    prints its content. If the file is empty, it prints "File is empty. Nothing to read."
 * 2. test_write_readonly_file() - Tries to write data to a file opened in read-only mode.
 * 3. test_write_to_existing_file() - Opens an existing file or creates a new file,
 *    writes some initial data to it, seeks to a specific position, writes additional 
 *    data, reads the entire content, and verifies the written data.
 * 4. test_error_cases() - Tests various error scenarios during file operations,
 *    including opening a non-existent file, incorrect flags while opening a file,
 *    and writing/reading from a file descriptor with the wrong permissions.
 ******************************************************************************/

#include "sysapi.h"
#include "test25.h"
#define NUM_ELT_TEST24 400000

int test_read_empty_file() {
    // Create or open a file (change "testfile.txt" to your desired filename)
    int fd = open("testfile3.txt", O_CREAT | O_RDWR, 0);
    if (fd == -1) {
        printf("Error opening file\n");
        return -1;
    }

    // Read from the file
    char buffer[100]; // Buffer to hold the read data
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer));

    if (bytes_read == -1) {
        printf("Error reading from file\n");
        close(fd);
        return -1;
    } else if (bytes_read == 0) {
        printf("File is empty. Nothing to read.\n");
    } else {
        printf("Read %zd bytes: %.*s\n", bytes_read, (int)bytes_read, buffer);
    }

    // Close the file
    if (close(fd) == -1) {
        printf("Error closing file\n");
        return -1;
    }
    if (unlink("testfile3.txt") == -1) {
        printf("Error Deleting file\n");
        return -1;
    }
    return 0;
}

int test_write_readonly_file() {
    // Create or open a file in read-only mode (change "readonly.txt" to your desired filename)
    int fd = open("readonly.txt", O_CREAT | O_RDONLY, 0);
    if (fd == -1) {
        printf("Error opening file\n");
        return -1;
    }

    // Try to write to the read-only file
    char data[] = "Hello, this is a test write to a read-only file.";
    ssize_t bytes_written = write(fd, data, sizeof(data) - 1); // Minus 1 to exclude the null terminator

    if (bytes_written > 0) {
        printf("Error writing to file\n");
        printf("Wrote %ld \n", bytes_written);
        return -1;
    }

    // Close the file
    if (close(fd) == -1) {
        printf("Error closing file\n");
        return -1;
    }
    if (unlink("readonly.txt") == -1) {
        printf("Error Deleting file\n");
        return -1;
    }
    return 0;
}

int test_write_to_existing_file() {
    // Open an existing file or create a new file (change "existingfile.txt" to your desired filename)
    char file[] = "existingfile.txt";
    int fd = open(file, O_CREAT | O_TRUNC | O_RDWR, 0666);
    if (fd == -1) {
        printf("Error opening file\n");
        return -1;
    }

    // Write some initial data to the file
    char initial_data[] = "Initial data.";
    ssize_t bytes_written = write(fd, initial_data, sizeof(initial_data) - 1); // Minus 1 to exclude the null terminator
    if (bytes_written == -1) {
        printf("Error writing to file\n");
        close(fd);
        return -1;
    }

    off_t seek_offset = 7; 
    if (lseek(fd, seek_offset, SEEK_SET) == -1) {
        printf("Error seeking to position\n");
        close(fd);
        return -1;
    }

    // Write additional data to the file
    char additional_data[] = " Additional data.";
    bytes_written = write(fd, additional_data, sizeof(additional_data) - 1); // Minus 1 to exclude the null terminator
    if (bytes_written == -1) {
        printf("Error writing additional data to file\n");
        close(fd);
        return -1;
    }

    // Seek back to the beginning of the file
    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf("Error seeking back to the beginning of the file\n");
        close(fd);
        return -1;
    }

    // Read the entire content of the file and verify initial and additional data
    char buffer[100]; // Buffer to hold the read data
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1); // Minus 1 to leave room for null terminator
    if (bytes_read == -1) {
        printf("Error reading from file\n");
        close(fd);
        return -1;
    }

    buffer[bytes_read] = '\0'; // Null-terminate the buffer

    printf("Read %zd bytes from file: %s\n", bytes_read, buffer);
    char expected_data[] = "Initial Additional data.";
    if (strcmp(expected_data, buffer) != 0){
        return -1;
    }
    // Close the file
    if (close(fd) == -1) {
        printf("Error closing file\n");
        return -1;
    }
    if (unlink(file) == -1) {
        printf("Error Deleting file\n");
        return -1;
    }
    return 0;
}

int test_error_cases() {
    // Test opening a non-existent file
    int fd = open("nonexistent25.txt", O_RDONLY, 0);
    if (fd == -1) {
        printf("Error caught opening non-existent file\n");
    }

    // Test flags while opening a file
    const char path[] =  "newfile25.txt";
    int test_flags_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, 0);
    if (test_flags_fd == -1) {
        printf("Error opening file with flags\n");
        return -1;
    }

    // Test writing to a file descriptor that is not open for writing
    int read_only_fd = open(path, O_RDONLY, 0);
    char data[] = "This write should fail.";
    ssize_t bytes_written = write(read_only_fd, data, sizeof(data) - 1);
    if (bytes_written == -1) {
        printf("Error caught writing to read-only file descriptor\n");
    }

    // Test reading from a file descriptor that is not open for reading
    int write_only_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0);
    char buffer[100];
    ssize_t bytes_read = read(write_only_fd, buffer, sizeof(buffer));
    if (bytes_read == -1) {
        printf("Error caught reading from write-only file descriptor\n");
    }

    // Close file descriptors
    if (close(test_flags_fd) == -1 || close(read_only_fd) == -1 || close(write_only_fd) == -1) {
        printf("Error caught closing file descriptors\n");
    }
    if (unlink(path) == -1) {
        printf("Error Deleting file\n");
        return -1;
    }
    return 0;
}


int main() {
 assert(test_read_empty_file() == 0);
 assert(test_write_readonly_file() == 0);
 assert(test_write_to_existing_file() == 0);
 assert(test_error_cases() == 0);
 return 0;
}




