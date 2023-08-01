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
#define NUM_ELT_TEST24 400000

int main(void *arg) {
    (void)arg;
    const char* filename = "test_file2.txt";
    uint32_t buffer[100]; 
    uint32_t num;

    // Open the file for writing and create it if it doesn't exist
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0);
    disk_info info;
    memset(&info, 0, sizeof(disk_info));
    fs_info(&info);
    uint32_t block_usage_start = info.free_blocks;
    if (fd == -1) {
        printf("Error opening the file\n");
        return 1;
    }
    ssize_t total_written = 0;
    uint32_t step_size = NUM_ELT_TEST24 / 10;
    // Write numbers from 1 to one million into the file
    for (uint32_t i = 1; i <= NUM_ELT_TEST24; i++) {
        num = i;
        if (i%step_size == 0){
            printf("percent = %d\n", i/step_size);
        }
        ssize_t bytes_written = write(fd, &num, sizeof(uint32_t));
        total_written += bytes_written;
        // printf("total_written = %ld\n", total_written);
        if (bytes_written == -1 || bytes_written == 0) {
            printf("Error writing to the file\n");
            close(fd);
            return 1;
        }
    }
    printf("Data has been written\n");
    // Close the file
    close(fd);

    // Open the file for reading
    fd = open(filename, O_RDONLY, 0);
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
            // printf("buffer[%d] = %u\n", elements_read, buffer[i]);
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
    fd = open(filename, O_WRONLY | O_TRUNC, 0);
    if (fd == -1) {
        printf("Error opening the file in truncate mode\n");
        return 1;
    }
    disk_info info2;
    memset(&info2, 0, sizeof(disk_info));
    fs_info(&info2);
    printf("Checking that blk num   are same %d == %d\n", block_usage_start, info2.free_blocks);
    assert(block_usage_start == info2.free_blocks);
    // Close the file
    close(fd);
    if (unlink("test_file2.txt") == -1) {
        printf("Error Deleting file\n");
        return -1;
    }
    printf("Test passed: File operations successful!\n");
    return 0;
}
