/*******************************************************************************
 * Ensimag - Projet Systeme 
 * Test 24
 *
 * Does basic file operations, open's a file 
 * (creating if it does not exist) and then writes numbers from 1 to one million into it
 * then it closes the file and finally it reopens and reads the numbers 
 * and makes sure that they are saved in the file.
 ******************************************************************************/

#include "sysapi.h"
#include "test24.h"
#define NUM_ELT_TEST24 100000

int main(void *arg) {
    (void)arg;
    const char* filename = "test_file2.txt";
    uint32_t buffer[100]; 
    uint32_t num;

    // Open the file for writing and create it if it doesn't exist
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC);
    if (fd == -1) {
        printf("Error opening the file\n");
        return 1;
    }
    ssize_t total_written = 0;
    // Write numbers from 1 to one million into the file
    for (uint32_t i = 1; i <= NUM_ELT_TEST24; i++) {
        num = i;
        ssize_t bytes_written = write(fd, &num, sizeof(uint32_t));
        total_written += bytes_written;
        // printf("total_written = %ld\n", total_written);
        if (bytes_written == -1) {
            printf("Error writing to the file\n");
            close(fd);
            return 1;
        }
    }
    printf("Data has been written\n");
    // Close the file
    close(fd);

    // Open the file for reading
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printf("Error opening the file for reading\n");
        return 1;
    }


    ssize_t bytes_read;
    uint32_t elements_read = 0;

    // Read data from the file
    while (elements_read < NUM_ELT_TEST24) {
        bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read == -1) {
            printf("Error reading from the file");
            close(fd);
            return -1;
        }
        // Check if the numbers read match the numbers written
        for (ssize_t i = 0; i < bytes_read / 4; i++) {
            printf("buffer[%d] = %u\n", elements_read, buffer[i]);
            if (buffer[i] != elements_read + 1) {
                printf("Numbers read do not match the numbers written stopped at %u!\n", elements_read + 1);
                close(fd);
                return -1;
            }
            elements_read++;
        }
    }
    // Close the file
    close(fd);

    printf("Data has been read\n");
    

    // Open the file in truncate mode to make it empty
    fd = open(filename, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        printf("Error opening the file in truncate mode\n");
        return 1;
    }

    // Close the file
    close(fd);

    printf("Test passed: File operations successful!\n");
    return 0;
}
