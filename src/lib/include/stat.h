#include "types.h"

typedef int            blksize_t;  // Block size for filesystem I/O
typedef long           blkcnt_t;   // Number of 512B blocks allocated
typedef unsigned short nlink_t;    // Number of hard links


#define	__S_IFMT	0170000	/* These bits determine file type.  */

/* Stat File types.  */
#define	__S_IFDIR	0040000	/* Directory.  */
#define	__S_IFCHR	0020000	/* Character device.  */
#define	__S_IFBLK	0060000	/* Block device.  */
#define	__S_IFREG	0100000	/* Regular file.  */
#define	__S_IFIFO	0010000	/* FIFO.  */
#define	__S_IFLNK	0120000	/* Symbolic link.  */
#define	__S_IFSOCK	0140000	/* Socket.  */

#define	__S_ISTYPE(mode, mask)	(((mode) & __S_IFMT) == (mask))

#define	S_ISDIR(mode)	 __S_ISTYPE((mode), __S_IFDIR)
#define	S_ISCHR(mode)	 __S_ISTYPE((mode), __S_IFCHR)
#define	S_ISBLK(mode)	 __S_ISTYPE((mode), __S_IFBLK)
#define	S_ISREG(mode)	 __S_ISTYPE((mode), __S_IFREG)
#ifdef __S_IFIFO
# define S_ISFIFO(mode)	 __S_ISTYPE((mode), __S_IFIFO)
#endif
#ifdef __S_IFLNK
# define S_ISLNK(mode)	 __S_ISTYPE((mode), __S_IFLNK)
#endif


struct stat {
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