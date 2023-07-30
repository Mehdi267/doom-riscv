#ifndef MY_DIRENT_H
#define MY_DIRENT_H

#include "fs_api.h"

/**
 * @brief Structure representing a directory entry.
 */
struct dirent {
    ino_t  d_ino;          /**< File serial number (inode number) of the entry. */
    char   d_name[];       /**< Name of the entry (variable-sized array). */
};

/**
 * @brief Type representing a directory stream.
 */
typedef struct {
    int            fd;     /**< File descriptor for the directory stream. */
    struct dirent  cur_ent; /**< Current directory entry. */
} DIR;

/**
 * @brief Close a directory stream.
 *
 * @param dirp A pointer to the directory stream to close.
 * @return 0 on success, -1 on failure.
 */
int closedir(DIR *dirp);

/**
 * @brief Open a directory stream for reading the contents of a directory.
 *
 * @param name The path to the directory to open.
 * @return A pointer to the directory stream on success, NULL on failure.
 */
DIR *opendir(const char *name);

/**
 * @brief Read the next directory entry (filename) from the directory stream.
 *
 * @param dirp A pointer to the directory stream.
 * @return A pointer to the next directory entry on success, NULL if the end of the directory is reached or an error occurs.
 */
struct dirent *readdir(DIR *dirp);

/**
 * @brief Read the next directory entry (filename) from the directory stream in a thread-safe manner.
 *
 * @param dirp A pointer to the directory stream.
 * @param entry A pointer to the buffer where the directory entry will be stored.
 * @param result A pointer to a pointer that will be set to NULL if the end of the directory is reached or an error occurs.
 *               If a valid directory entry is read, result will be set to entry.
 * @return 0 on success, a positive value if the end of the directory is reached, or a negative value if an error occurs.
 */
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);

/**
 * @brief Reset the directory stream position to the beginning, allowing you to read the directory entries again from the start.
 *
 * @param dirp A pointer to the directory stream.
 */
void rewinddir(DIR *dirp);

/**
 * @brief Set the directory stream position to a specific location, allowing you to read the directory entries from that position.
 *
 * @param dirp A pointer to the directory stream.
 * @param loc The position in the directory stream where you want to set the position. This position should be previously obtained using telldir.
 */
void seekdir(DIR *dirp, long int loc);

/**
 * @brief Get the current position of the directory stream.
 *
 * @param dirp A pointer to the directory stream.
 * @return The current position in the directory stream.
 */
long int telldir(DIR *dirp);

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


#endif // MY_DIRENT_H
