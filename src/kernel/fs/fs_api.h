#include <stdint.h>
#include <stddef.h>
#include "../process/fs_bridge.h"
#ifndef FILE_SYS_CALLS_H
#define FILE_SYS_CALLS_H

typedef enum {
    PERMISSION_NONE = 0,
    PERMISSION_READ = 1 << 0,
    PERMISSION_WRITE = 1 << 1,
    PERMISSION_EXECUTE = 1 << 2
} Permission;

typedef long long off_t;
typedef unsigned int mode_t;
typedef int pid_t;
typedef long ssize_t;
typedef unsigned long int ino_t;
typedef unsigned int   uid_t;      // User ID of owner
typedef unsigned int   gid_t;      // Group ID of owner
typedef unsigned int   dev_t;      // Device ID (if file is character or block special)
typedef int            blksize_t;  // Block size for filesystem I/O
typedef long           blkcnt_t;   // Number of 512B blocks allocated
typedef long           time_t;     // Time type (usually represents POSIX timestamp)
typedef unsigned short nlink_t;    // Number of hard links

// Read-only
#define O_RDONLY 00000000
// Write-only
#define O_WRONLY 00000001
// Read-write
#define O_RDWR 00000002
// Create the file if it doesn't exist
#ifndef O_CREAT
#define O_CREAT 00000100  
#endif
// Fail if the file exists and O_CREAT is used
#ifndef O_EXCL
#define O_EXCL 00000200  
#endif
#ifndef O_NOCTTY
#define O_NOCTTY 00000400  
#endif
// Truncate the file to zero length upon opening
#ifndef O_TRUNC
#define O_TRUNC 00001000  
#endif
// Set the file offset to the end before each write
#ifndef O_APPEND
#define O_APPEND 00002000
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK 00004000
#endif
#ifndef O_SYNC
#define O_SYNC 00010000
#endif
#ifndef FASYNC
#define FASYNC 00020000  
#endif
#ifndef O_DIRECT
#define O_DIRECT 00040000  
#endif
#ifndef O_LARGEFILE
#define O_LARGEFILE 00100000
#endif
// Ensure that the file is a directory
#ifndef O_DIRECTORY
#define O_DIRECTORY 00200000  
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 00400000  
#endif
#ifndef O_NOATIME
#define O_NOATIME 01000000
#endif

/**
 * Opens or creates a file.
 *
 * @param file_name The name of the file to be opened or created.
 * @param flags The file open flags that specify the file access mode and other options.
 * @param mode The file mode to be used if O_CREAT flag is specified.
 * @return On success, returns the file descriptor of the opened or created file. On failure, returns -1 and sets 'errno' to indicate the specific error.
 */
int open(process*, const char *file_name, int flags, mode_t mode);

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

struct __attribute__((__packed__)) stat {
    dev_t     st_dev;         // ID of device containing file
    ino_t     st_ino;         // Inode number
    mode_t    st_mode;        // File type and mode
    nlink_t   st_nlink;       // Number of hard links
    uid_t     st_uid;         // User ID of owner
    gid_t     st_gid;         // Group ID of owner
    dev_t     st_rdev;        // Device ID (if file is character or block special)
    off_t     st_size;        // Total size, in bytes
    blksize_t st_blksize;     // Block size for filesystem I/O
    blkcnt_t  st_blocks;      // Number of 512B blocks allocated
    time_t    st_atime;       // Time of last access
    time_t    st_mtime;       // Time of last modification
    time_t    st_ctime;       // Time of last status change
};

/* Stat File types.  */
#define	__S_IFDIR	0040000	/* Directory.  */
#define	__S_IFCHR	0020000	/* Character device.  */
#define	__S_IFBLK	0060000	/* Block device.  */
#define	__S_IFREG	0100000	/* Regular file.  */
#define	__S_IFIFO	0010000	/* FIFO.  */
#define	__S_IFLNK	0120000	/* Symbolic link.  */
#define	__S_IFSOCK	0140000	/* Socket.  */

#define	__S_ISUID	04000	/* Set user ID on execution.  */
#define	__S_ISGID	02000	/* Set group ID on execution.  */
#define	__S_ISVTX	01000	/* Save swapped text after use (sticky).  */
#define	__S_IREAD	0400	/* Read by owner.  */
#define	__S_IWRITE	0200	/* Write by owner.  */
#define	__S_IEXEC	0100	/* Execute by owner.  */

/**
 * @brief Get file information for a specified file.
 *
 * The stat function retrieves file information, such as size, permissions,
 * timestamps, etc., for a file specified by its pathname.
 *
 * @param[in] pathname The path to the file whose information is to be retrieved.
 * @param[out] buf Pointer to a struct stat that will be filled with file information.
 * @return 0 on success, -1 on failure, and the specific error code is set in errno.
 */
int stat(const char *pathname, struct stat *buf);

/**
 * @brief Get file information for an open file descriptor.
 *
 * The fstat function retrieves file information, such as size, permissions,
 * timestamps, etc., for a file associated with the specified file descriptor.
 *
 * @param[in] fd The file descriptor for the open file whose information is to be retrieved.
 * @param[out] buf Pointer to a struct stat that will be filled with file information.
 * @return 0 on success, -1 on failure, and the specific error code is set in errno.
 */
int fstat(unsigned int fd, struct stat *buf);


/**
 * @brief Duplicate a file descriptor.
 *
 * The dup() function duplicates an existing file descriptor, file_descriptor,
 * using the lowest-numbered unused file descriptor for the new descriptor.
 *
 * @param file_descriptor The file descriptor to be duplicated.
 * @return On success, returns the new file descriptor. On failure, -1 is returned,
 * and errno is set to indicate the error.
 * @note The new file descriptor shares the same file offset, file status flags,
 * and file access mode with the original file descriptor.
 */
int dup(process*, int file_descriptor);

/**
 * @brief Duplicate a file descriptor to a specific file descriptor number.
 *
 * The dup2() function duplicates the file descriptor specified by file_descriptor
 * to the file descriptor number specified by new_file_descriptor.
 * If new_file_descriptor is already open, it is closed before duplicating the descriptor.
 *
 * @param file_descriptor The file descriptor to be duplicated.
 * @param new_file_descriptor The new file descriptor number to which the duplication will be performed.
 * @return On success, returns the new file descriptor. On failure, -1 is returned,
 * and errno is set to indicate the error.
 * @note If new_file_descriptor is negative or greater than or equal to the maximum number of file descriptors,
 * the function fails, and no duplication occurs.
 * @note The new file descriptor shares the same file offset, file status flags,
 * and file access mode with the original file descriptor.
 */
int dup2(process*, int file_descriptor, int new_file_descriptor);

/**
 * @brief Create a pipe for inter-process communication.
 *
 * This function creates a pipe and returns two file descriptors in the
 * `file_descriptors` array. The first file descriptor (index 0) is used for
 * reading from the pipe, and the second file descriptor (index 1) is used for
 * writing to the pipe.
 *
 * @param file_descriptors An array of two integers where the file descriptors
 *                         for the read and write ends of the pipe will be stored.
 * @return On success, 0 is returned. On failure, -1 is returned, 
 */
int sys_pipe(int file_descriptors[2]);

/**
 * @brief Delete a name from the file system.
 *
 * This function removes the file named by `file_name` from the file system.
 * If the name refers to a file, the file is deleted. If the name refers to
 * a symbolic link, the symbolic link is removed and not the file it points to.
 *
 * @param file_name The name of the file to be unlinked.
 * @return On success, 0 is returned. On failure, -1 is returned, 
 */
int unlink(const char *file_name);

int rename(const char *old_name, const char *new_name);
int rmdir(const char *dir_name);

#define R_OK  4  /* Read */
#define W_OK  2  /* Write */
#define X_OK  1  /* Execute */
#define F_OK  0  /* Existence */

int access(const char *, int);

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

