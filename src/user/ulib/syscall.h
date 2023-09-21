/*
 * Damien Dejean - Gaëtan Morin
 * Mathieu Barbe
 * Mehdi Frikha
 * Ensimag, Projet Système 2010
 * XUNIL
 * Headers de la bibliothèque d'appels systèmes.
 */
#ifndef ___SYSCALL_H___
#define ___SYSCALL_H___
#include "stdint.h"
#include "dirent.h"
typedef __SIZE_TYPE__ size_t;


/*******************************************************************************
 * Assert : check a condition or fail
 ******************************************************************************/
#define __STRING(x) #x
#define die(str, ...) ({ \
  printf("%s:%d: " str "\n", __FILE__, __LINE__, ##__VA_ARGS__); exit(-1); })

#define assert(x) ({ if (!(x)) die("assertion failed: %s", #x); })

/*******************************************************************************
 * var args
 ******************************************************************************/

typedef __builtin_va_list va_list;
#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)
#define va_copy(d,s)    __builtin_va_copy(d,s)


/*******************************************************************************
 * stdio
 ******************************************************************************/

int getchar(void);
int printf(const char *, ...);
int puts(const char *);
int sprintf(char *, const char *, ...);
int snprintf(char *, size_t, const char *, ...);
int vprintf(const char *, va_list);
int vsprintf(char *, const char *, va_list);
int vsnprintf(char *, size_t, const char *, va_list);
#define fprintf(f, ...) printf(__VA_ARGS__)

/*******************************************************************************
 * Printf macros
 ******************************************************************************/
#define PRINTF_LEFT_JUSTIFY 1
#define PRINTF_SHOW_SIGN 2
#define PRINTF_SPACE_PLUS 4
#define PRINTF_ALTERNATE 8
#define PRINTF_PAD0 16
#define PRINTF_CAPITAL_X 32

#define PRINTF_BUF_LEN 512

int strcmp(const char *str1, const char *str2);
size_t strlen(const char *s);
char *strncpy(char *, const char *, size_t);
void *memset(void *dst, int c, size_t n);


extern int chprio(int pid, int newprio);
extern int cons_write(const char *str, unsigned long size);
extern unsigned long cons_read(char *string, unsigned long length);
extern void cons_echo(int on);
extern void exit(int retval);
extern int getpid(void);
extern int getprio(int pid);
extern int kill(int pid);
 
union sem {
    int fid;
    int sem;
};

void xwait(union sem *s);
void xsignal(union sem *s);
void xsdelete(union sem *s);
void xscreate(union sem *s);

extern int scount(int sem);
extern int screate(short count);
extern int sdelete(int sem);
extern int signal(int sem);
extern int signaln(int sem, short count);
extern int sreset(int sem, short count);
extern int try_wait(int sem);
extern int wait(int sem);

extern int pcount(int fid, int *count);
extern int pcreate(int count);
extern int pdelete(int fid);
extern int preceive(int fid,int *message);
extern int preset(int fid);
extern int psend(int fid, int message);
    
extern void clock_settings(unsigned long *quartz, unsigned long *ticks);
extern unsigned long current_clock(void);
extern void wait_clock(unsigned long wakeup);

extern int start(const char *process_name, unsigned long ssize, int prio, void *arg);
extern int waitpid_old(int pid, long int *retval);
extern int waitpid(int pid, long int *retval, int);

extern void *shm_create(const char*);
extern void *shm_acquire(const char*);
extern void shm_release(const char*);

extern void power_off(int exit_value);
extern void show_ps_info();
extern void show_programs();
extern void info_queue();
extern void sleep(unsigned int nbr_sec);

//Disk related syscalls
extern void display_partions();
extern int create_partition(uint32_t start, uint32_t size, uint8_t partition_type);
extern int delete_partition(uint8_t partition_number);
extern int reset_disk();

//Disk Cache related syscals
extern int sync();
extern int clear_disk_cache();
extern void print_fs_details();

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
typedef unsigned short nlink_t;    // Number of hard links
typedef long           time_t;     // Time type (usually represents POSIX timestamp)


// Messages from users
#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_RDWR 00000002
#ifndef O_CREAT
#define O_CREAT 00000100  
#endif
#ifndef O_EXCL
#define O_EXCL 00000200  
#endif
#ifndef O_NOCTTY
#define O_NOCTTY 00000400  
#endif
#ifndef O_TRUNC
#define O_TRUNC 00001000  
#endif
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
#ifndef O_DIRECTORY
#define O_DIRECTORY 00200000  
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 00400000  
#endif
#ifndef O_NOATIME
#define O_NOATIME 01000000
#endif

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

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

//Fs api 
int open(const char *file_name, int flags, mode_t mode);
int close(int file_descriptor);
ssize_t read(int file_descriptor, void *buffer, size_t count);
ssize_t write(int file_descriptor, const void *buffer, size_t count);
off_t lseek(int file_descriptor, off_t offset, int whence);
int unlink(const char *file_name);
int link(const char *oldpath, const char *newpath);
int dup(int file_descriptor);
int dup2(int file_descriptor, int new_file_descriptor);
int stat(const char *pathname, struct stat *buf);
int fstat(unsigned int fd, struct stat *buf);
int pipe(int file_descriptors[2]);
pid_t fork(void);
int execve(const char *filename, char *const argv[], char *const envp[]);
void *sbrk(unsigned long increment);

//dir api
char *getcwd(char *buf, size_t size);
int mkdir(const char *dir_name, mode_t mode);
int chdir(const char *new_directory);
int rmdir(const char *path);
int getdents(int fd, struct dirent *dirp,
            unsigned int count);

//Custom calls
void ld_progs_into_disk();
int write_file_disk(const char* name);
void print_dir_elements(const char*);
typedef struct disk_info{
 uint32_t total_blocks;
 uint32_t free_blocks;
 uint32_t total_inodes;
 uint32_t free_inodes;
} disk_info;    
void fs_info(disk_info* info);
void void_call();


//Display syscalls
struct display_info{
  int width;
  int height;
};
int upd_data_display(void* data, int x, int y, int width, int height);
void get_display_info(struct display_info*);
int access(const char *, int);

//Time syscalls
// Data structures
typedef signed long time_t;	
typedef signed long suseconds_t;	
struct timeval{
  time_t tv_sec;		/* Seconds.  */
  suseconds_t tv_usec;	/* Microseconds.  */
};

struct timezone {
   int tz_minutewest;
   int tz_dsttime;
};

time_t time(time_t *tloc);
int gettimeofday(struct timeval *tv, struct timezone *tz);
int settimeofday(const struct timeval *tv, const struct timezone *tz);

typedef enum input_type{
  CONSOLE_INPUT,
  RAW_INPUT,
} input_t;

//Used to set input type for the current process 
void set_in_mode(int pid, input_t type);

#endif
