/*******************************************************************************
 * Ensimag - Projet Systeme
 * Copyright 2023 - Mehdi Frikha 
 * Test 23
 *
 * Does basic file operations, open's a file 
 * (creating if it does not exist) and then writes data into and 
 * then it closes the file and finally it reopens and reads that said data 
 * and makes sure that it still present and saved in the file.
 ******************************************************************************/

#include "sysapi.h"
#include "test23.h"

int main(void *arg){
    (void)arg;
    const char* filename = "test_file.txt";
    const char* data_to_write = "Hello, this is the test data!";
    char buffer[100];

    // Open the file for writing and create it if it doesn't exist
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0);
    if (fd == -1) {
        printf("Error opening the file");
        return 1;
    }

    // Write data into the file
    ssize_t bytes_written = write(fd, data_to_write, strlen(data_to_write));
    if (bytes_written == -1) {
        printf("Error writing to the file");
        close(fd);
        return 1;
    }

    // Close the file
    close(fd);

    // Open the file for reading
    fd = open(filename, O_RDONLY, 0);
    if (fd == -1) {
        printf("Error opening the file for reading");
        return 1;
    }

    // Read data from the file
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        printf("Error reading from the file");
        close(fd);
        return 1;
    }

    // Null-terminate the buffer to use it as a C-style string
    buffer[bytes_read] = '\0';

    // Close the file
    close(fd);

    // Check if the data read matches the data written
    if (strcmp(buffer, data_to_write) == 0) {
        printf("Data read matches the data written: %s\n", buffer);
    } else {
        printf("Data read does not match the data written!\n");
        return -1;
    }
    return 0;
}
