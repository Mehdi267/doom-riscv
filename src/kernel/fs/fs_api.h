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
int open(const char *file_name, int flags, mode_t mode);

int access(const char *file_name, int mode);
int chdir(const char *new_directory);
int chmod(const char *file_name, mode_t new_mode);
int chroot(const char *new_root_directory);
int close(int file_descriptor);
int create(const char *file_name, mode_t mode);
int dup(int file_descriptor);
int dup2(int file_descriptor, int new_file_descriptor);
int fcntl(int file_descriptor, int function_code, int arg);
int fstat(const char *file_name, struct stat *buffer);
int ioctl(int file_descriptor, int function_code, int arg);
off_t lseek(int file_descriptor, off_t offset, int whence);
int mkdir(const char *dir_name, mode_t mode);
//will try to implement but the the current design choices make this very hard to implement
int mount(const char *special_file, const char *mount_point, int ro_flag);
int pipe(int file_descriptors[2]);
ssize_t read(int file_descriptor, void *buffer, size_t count);
int rename(const char *old_name, const char *new_name);
int rmdir(const char *dir_name);
int stat(const char *file_name, struct stat *status_buffer);
mode_t umask(mode_t mask);
int umount(const char *special_file);
int unlink(const char *file_name);
int utime(const char *file_name, const struct utimbuf *times);
ssize_t write(int file_descriptor, const void *buffer, size_t count);

// Messages from PM
int exec(pid_t pid);
void exit(pid_t pid);
pid_t fork(pid_t parent_pid);
pid_t setsid(pid_t pid);

#endif /* FILE_SYS_CALLS_H */
