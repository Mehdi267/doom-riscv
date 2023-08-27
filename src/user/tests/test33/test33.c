/*******************************************************************************
 * Ensimag - Projet Systeme 
 * Test 32
 *
 * Test 33 - get dir test
 *
 ******************************************************************************/

#include "sysapi.h"
#include "test33.h"


enum {
    BUF_SIZE = 1024,
};

int main() {
    // Create a directory
    if (mkdir("mydir", 0777) == -2) {
      printf("mkdir");
      return 1;
    }

    // Enter the directory
    if (chdir("mydir") != 0) {
      printf("chdir");
      return 1;
    }

    // Create a file
    int fd = open("myfile.txt", O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
      printf("open");
      return 1;
    }

    // Close the file
    close(fd);
    int fddir = open(".", O_DIRECTORY , 0644);
    if (fddir == -1) {
      printf("open dir");
      return 1;
    }
    char buf_c[BUF_SIZE];
    struct dirent * buf = (struct dirent *)buf_c;
    int nread = getdents(fddir, buf, BUF_SIZE);

    if (nread == -1) {
      printf("getdents");
      return 1;
    }

    struct dirent *entry;
    int offset = 0;
    int i  = 0;
    // printf("##nread = %d\n", nread);
    while (offset < nread - 1) { // temp measure
      if(i > 5){break;}
      entry = (struct dirent *)(buf_c + offset);
      // printf("inode nb: %ld\n", entry->d_ino);
      // printf("Entry rec len: %d\n", entry->d_reclen);
      // printf("Entry type: %d\n", entry->d_type);
      // printf("Entry name: %s\n", entry->d_name);
      if (i == 0){
        assert(entry->d_reclen == 32);
        assert(entry->d_type == 2);
      }
      if (i == 1){
        assert(entry->d_reclen == 32);
        assert(entry->d_type == 2);
      }
      if (i == 2){
        assert(entry->d_reclen == 40);
        assert(entry->d_type == 1);
        assert(strcmp("myfile.txt", entry->d_name) == 0);
      }
      offset += entry->d_reclen;
      i++;
    }
    if (close(fddir)<0){
      return 1;
    }
    // Clean up: Delete the file and the directory
    if (unlink("myfile.txt") != 0) {
      printf("unlink");
    }

    if (chdir("..") != 0) {
      printf("chdir");
      return 1;
    }

    if (rmdir("mydir") != 0) {
      printf("rmdir");
      return 1;
    }

    return 0;
}
