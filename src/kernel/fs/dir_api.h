#ifndef MY_DIRAPI_H
#define MY_DIRAPI_H

#include "fs_api.h"
#include "dirent.h"
/**
 * @brief Check accessibility of a file.
 *
 * @param file_name The path to the file to be checked.
 * @param mode The mode indicating the type of accessibility to check (e.g., R_OK for read access).
 * @return 0 if the file is accessible with the specified mode, -1 on failure, and errno is set to indicate the error.
 */
int access(const char *file_name, int mode);

/**
 * @brief Get the current working directory.
 *
 * @param buf A pointer to the buffer where the current working directory will be stored.
 * @param size The size of the buffer (buf) in bytes.
 * @return On success, returns buf. On failure, returns NULL, and errno is set to indicate the error.
 */
char *getcwd(char *buf, size_t size);

/**
 * @brief Change the current working directory.
 *
 * @param new_directory The path of the directory to become the new current working directory.
 * @return 0 on success, -1 on failure, and errno is set to indicate the error.
 */
int chdir(const char *new_directory);

/**
 * @brief Change the permissions of a file.
 *
 * @param file_name The path to the file whose permissions will be changed.
 * @param new_mode The new file mode (permissions) to be set for the file.
 * @return 0 on success, -1 on failure, and errno is set to indicate the error.
 */
int chmod(const char *file_name, mode_t new_mode);

/**
 * @brief Change the root directory.
 *
 * @param new_root_directory The path of the directory to become the new root directory.
 * @return 0 on success, -1 on failure, and errno is set to indicate the error.
 */
int chroot(const char *new_root_directory);


/**
 * @brief Create a new directory.
 *
 * The mkdir() function creates a new directory with the specified name and permissions.
 *
 * @param dir_name The path to the new directory to be created.
 *                 It can be an absolute or relative path.
 * @param mode     The permissions to set for the new directory.
 *                 It is specified as an octal number (e.g., 0755) and determines the access rights
 *                 for the owner, group, and others.
 *
 * @return  On success, returns 0.
 *          On failure, returns -1, and the error code is set in the errno variable to indicate
 *          the specific reason for the failure.
 *
 * @note If the directory already exists, the mkdir() function will fail.
 *       The parent directory of the new directory should already exist for mkdir() to succeed.
 *
 * @note The mode parameter is used to set the permissions of the new directory.
 *       The exact permission bits that are set depend on the user's umask setting.
 *       To ensure specific permissions are set, it is recommended to use explicit octal values.
 *  
 * @note The mkdir() function may fail due to various reasons, including permissions, file system
 *       constraints, or other errors. In case of failure, check the errno variable for the
 *       specific error code.
 */
int mkdir(const char *dir_name, mode_t mode);

/**
 * @brief Remove a directory.
 *
 * The rmdir() function deletes a directory specified by the path name.
 * The directory must be empty for the removal to be successful. If the
 * directory is not empty or there is an error, the function will fail.
 *
 * @param path The path to the directory to be removed.
 * @return Upon successful completion, 0 is returned. On error, -1 is returned,
 *         and errno is set to indicate the error.
 * @note The directory must be empty for the operation to succeed.
 * @see rmdir(2)
 */
int rmdir(const char *path);

/**
 * @brief Rename a file or directory.
 *
 * The rename() function renames a file or directory specified by the oldpath
 * to the newpath. If the newpath already exists, it will be overwritten.
 *
 * @param oldpath The current path of the file or directory to be renamed.
 * @param newpath The new path or name for the file or directory.
 * @return Upon successful completion, 0 is returned. On error, -1 is returned,
 *         and errno is set to indicate the error.
 * @see rename(2)
 */
int rename(const char *oldpath, const char *newpath);

/**
 * int getdents(unsigned int fd, struct dirent *dirp, unsigned int count);
 *
 * The getdents() system call reads directory entries from the directory referred
 * to by the open file descriptor 'fd' into the buffer pointed to by 'dirp'. The
 * entries are read into the buffer in a format defined by 'struct dirent'.
 * This function allows you to retrieve a list of directory entries directly from
 * the kernel into the provided buffer.
 *
 * Parameters:
 *   - fd: The file descriptor of the directory to read from.
 *   - dirp: A pointer to a buffer where directory entries will be stored.
 *   - count: The size of the buffer in bytes.
 *
 * Return Value:
 *   - On success, the number of bytes read into the buffer.
 *   - On error, -1 is returned and errno is set to indicate the error.
 *
 * Notes:
 *   - It's important to provide a buffer of sufficient size (specified by 'count')
 *     to accommodate the directory entries. If the buffer is too small, some
 *     entries might not be read completely, and the function may need to be called
 *     multiple times to read all entries.
 *   - The 'count' parameter helps to prevent buffer overflow, ensuring that the
 *     system reads no more data than can fit into the provided buffer.
 */
int getdents(int fd, struct dirent *dirp,
            unsigned int count);

#endif // MY_DIRAPI_H
