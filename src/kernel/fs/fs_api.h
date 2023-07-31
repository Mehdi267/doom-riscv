#include <stdint.h>
#include <stddef.h>
#ifndef FILE_SYS_CALLS_H
#define FILE_SYS_CALLS_H

typedef enum {
    PERMISSION_NONE = 0,
    PERMISSION_READ = 1 << 0,
    PERMISSION_WRITE = 1 << 1,
    PERMISSION_EXECUTE = 1 << 2
} Permission;

typedef long off_t;
typedef unsigned int mode_t;
typedef int pid_t;
typedef long ssize_t;
typedef unsigned long int ino_t;

// Messages from users
enum FileOpenFlags {
    O_RDONLY = 0x0000,      // Read-only
    O_WRONLY = 0x0001,      // Write-only
    O_RDWR = 0x0002,        // Read-write
    O_CREAT = 0x0010,       // Create the file if it doesn't exist
    O_EXCL = 0x0020,        // Fail if the file exists and O_CREAT is used
    O_TRUNC = 0x0040,       // Truncate the file to zero length upon opening
    O_APPEND = 0x0080,      // Set the file offset to the end before each write
    O_SYNC = 0x0200         // Write operations are synchronized on storage
};
/**
 * Opens or creates a file.
 *
 * @param file_name The name of the file to be opened or created.
 * @param flags The file open flags that specify the file access mode and other options.
 * @param mode The file mode to be used if O_CREAT flag is specified.
 * @return On success, returns the file descriptor of the opened or created file. On failure, returns -1 and sets 'errno' to indicate the specific error.
 */
int open(const char *file_name, int flags, mode_t mode);

/**
 * Closes the file associated with the given file descriptor.
 *
 * @param file_descriptor The file descriptor of the file to be closed.
 * @return On success, returns 0. On failure, returns -1 and sets 'errno' to indicate the specific error.
 */
int close(int file_descriptor);

/**
 * Reads data from a file into a buffer.
 *
 * @param file_descriptor The file descriptor of the file to read from.
 * @param buffer The buffer to store the read data.
 * @param count The maximum number of bytes to read.
 * @return On success, returns the number of bytes read. On failure, returns -1 and sets 'errno' to indicate the specific error.
 */
ssize_t read(int file_descriptor, void *buffer, size_t count);

/**
 * Writes data from a buffer to a file.
 *
 * @param file_descriptor The file descriptor of the file to write to.
 * @param buffer The buffer containing the data to be written.
 * @param count The number of bytes to write.
 * @return On success, returns the number of bytes written. On failure, returns -1 and sets 'errno' to indicate the specific error.
 */
ssize_t write(int file_descriptor, const void *buffer, size_t count);

// Other function definitions and comments omitted for brevity...

/**
 * Creates a hard link between an existing file and a new filename.
 *
 * @param oldpath The pathname of the existing file (source) to be linked.
 * @param newpath The pathname of the new link (destination) to be created.
 * @return On success, returns 0. On failure, returns -1 and sets 'errno' to indicate the specific error.
 */
int sys_link(const char *oldpath, const char *newpath);

enum SEEK_OPERATION {
  SEEK_SET = 0,
  SEEK_CUR = 1,
  SEEK_END = 2,
};
/**
 * Changes the file offset of an open file.
 *
 * @param file_descriptor The file descriptor of the open file.
 * @param offset The new file offset.
 * @param whence The reference point for the offset (SEEK_SET, SEEK_CUR, SEEK_END).
 * @return On success, returns the new file offset. On failure, returns -1 and sets 'errno' to indicate the specific error.
 */
off_t lseek(int file_descriptor, off_t offset, int whence);


int dup(int file_descriptor);
int dup2(int file_descriptor, int new_file_descriptor);
int fcntl(int file_descriptor, int function_code, int arg);
// int fstat(const char *file_name, struct stat *buffer);
int ioctl(int file_descriptor, int function_code, int arg);
//will try to implement but the the current design choices make this very hard to implement
int mount(const char *special_file, const char *mount_point, int ro_flag);
// int pipe(int file_descriptors[2]);
int rename(const char *old_name, const char *new_name);
int rmdir(const char *dir_name);
// int stat(const char *file_name, struct stat *status_buffer);
mode_t umask(mode_t mask);
int umount(const char *special_file);
int unlink(const char *file_name);
//int utime(const char *file_name, const struct utimbuf *times);

// Messages from PM
int exec(pid_t pid);
void exit(pid_t pid);
pid_t fork(pid_t parent_pid);
pid_t setsid(pid_t pid);

void print_dir_elements(const char* path);
typedef struct disk_info{
 uint32_t total_blocks;
 uint32_t free_blocks;
 uint32_t total_inodes;
 uint32_t free_inodes;
} disk_info;
void fs_info(disk_info* info);
extern int sync_all(); // define elsewere

#endif /* FILE_SYS_CALLS_H */

