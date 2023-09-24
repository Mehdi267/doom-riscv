/*
 * Ensimag - Projet système
 * Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
 *
 * Unique header of the standalone test standard library.
 */

#ifndef _SYSAPI_H_
#define _SYSAPI_H_
#include "stat.h"

#define NULL ((void*)0)
typedef __SIZE_TYPE__ size_t;

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

/*******************************************************************************
 * stdint : Standard ints
 ******************************************************************************/
typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;

typedef __INT_FAST8_TYPE__ int_fast8_t;
typedef __INT_FAST16_TYPE__ int_fast16_t;
typedef __INT_FAST32_TYPE__ int_fast32_t;
typedef __INT_FAST64_TYPE__ int_fast64_t;

typedef __INT_LEAST16_TYPE__ int_least16_t;
typedef __INT_LEAST32_TYPE__ int_least32_t;
typedef __INT_LEAST64_TYPE__ int_least64_t;
typedef __INT_LEAST8_TYPE__ int_least8_t;

typedef __INTMAX_TYPE__ intmax_t;
typedef __INTPTR_TYPE__ intptr_t;

typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;

typedef __UINT_FAST8_TYPE__ uint_fast8_t;
typedef __UINT_FAST16_TYPE__ uint_fast16_t;
typedef __UINT_FAST32_TYPE__ uint_fast32_t;
typedef __UINT_FAST64_TYPE__ uint_fast64_t;

typedef __UINT_LEAST8_TYPE__ uint_least8_t;
typedef __UINT_LEAST16_TYPE__ uint_least16_t;
typedef __UINT_LEAST32_TYPE__ uint_least32_t;
typedef __UINT_LEAST64_TYPE__ uint_least64_t;

typedef __UINTMAX_TYPE__ uintmax_t;
typedef __UINTPTR_TYPE__ uintptr_t;

#define INT8_MAX __INT8_MAX__
#define INT16_MAX __INT16_MAX__
#define INT32_MAX __INT32_MAX__
#define INT64_MAX __INT64_MAX__

#define UINT8_MAX  __UINT8_MAX__
#define UINT16_MAX __UINT16_MAX__
#define UINT32_MAX __UINT32_MAX__
#define UINT64_MAX __UINT64_MAX__

#define INTPTR_MAX __INTPTR_MAX__
#define UINTPTR_MAX __UINTPTR_MAX__
#define PTRDIFF_MAX __PTRDIFF_MAX__
#define SIZE_MAX __SIZE_MAX__

/*******************************************************************************
 * Assert : check a condition or fail
 ******************************************************************************/
#define __STRING(x) #x
#define die(str, ...) ({ \
  printf("%s:%d: " str "\n", __FILE__, __LINE__, ##__VA_ARGS__); exit(-1); })

#define assert(x) ({ if (!(x)) die("assertion failed: %s", #x); })

#define DUMMY_VAL 78

#define TSC_SHIFT 8

#define FREQ_PREC 50

#define NBSEMS 10000

#define TRUE 1
#define FALSE 0

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define NR_PHILO 5
// Prototype des appels systeme de la spec
int chprio(int pid, int newprio);
int cons_write(const char *str, unsigned long size);
unsigned long cons_read(char *string, unsigned long length);
void cons_echo(int on);
void exit(int retval);
int getpid(void);
int getprio(int pid);
int kill(int pid);
void power_off(int exit_value);
void show_ps_info();

/* #define WITH_SEM */
// #define WITH_MSG

#if defined WITH_SEM
int scount(int sem);
int screate(short count);
int sdelete(int sem);
int signal(int sem);
int signaln(int sem, short count);
int sreset(int sem, short count);
int try_wait(int sem);
int wait(int sem);
#elif defined WITH_MSG
int pcount(int fid, int *count);
int pcreate(int count);
int pdelete(int fid);
int preceive(int fid,int *message);
int preset(int fid);
int psend(int fid, int message);
#else
# error "WITH_SEM" ou "WITH_MSG" doit être définie.
#endif

/* Wrapper sur les verrous basés sur les sémaphores ou les files de messages */
union sem {
    int fid;
    int sem;
};
void xwait(union sem *s);
void xsignal(union sem *s);
void xsdelete(union sem *s);
void xscreate(union sem *s);

#ifndef TELECOM_TST
void clock_settings(unsigned long *quartz, unsigned long *ticks);
unsigned long current_clock(void);
void wait_clock(unsigned long wakeup);
#endif
int start(const char *process_name, unsigned long ssize, int prio, void *arg);
int waitpid_old(int pid, long int *retval);
int waitpid(int pid, long int *retval, int);

#if defined WITH_SEM
/*
 * Pour la soutenance, devrait afficher la liste des processus actifs, des
 * semaphores utilises et toute autre info utile sur le noyau.
 */
#elif defined WITH_MSG
/*
 * Pour la soutenance, devrait afficher la liste des processus actifs, des
 * files de messages utilisees et toute autre info utile sur le noyau.
 */
#endif
void sys_info(void);

/* Available from our standard library */
#ifndef __SIZE_TYPE__
#error __SIZE_TYPE__ not defined
#endif

typedef __SIZE_TYPE__ size_t;

int strcmp(const char *str1, const char *str2);
size_t strlen(const char *s);
char *strncpy(char *, const char *, size_t);
void *memset(void *dst, int c, size_t n);
int memcmp(const void *, const void *, size_t);

/* printf.h */
void cons_gets(char *s, unsigned long length);
/* assert.c */
int assert_failed(const char *cond, const char *file, int line);

/* math.h */
short randShort(void);
void setSeed(uint_fast64_t _s);
unsigned long rand();
unsigned long long div64(unsigned long long num, unsigned long long div, unsigned long long *rem);

/* it.c */
void test_it(void);

/* Shared memory */
void *shm_create(const char*);
void *shm_acquire(const char*);
void shm_release(const char*);


//Disk related syscalls
extern void display_partions();
extern int create_partition(uint32_t start, uint32_t size, uint8_t partition_type);
extern int delete_partition(uint8_t partition_number);
extern int reset_disk();

//Disk Cache related syscals
extern int sync();
extern int clear_disk_cache();
extern void print_fs_details();


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

enum SEEK_OPERATION {
  SEEK_SET = 0,
  SEEK_CUR = 1,
  SEEK_END = 2,
};

#ifndef DT_UNKNOWN
#define  DT_UNKNOWN     0
#define  DT_FIFO        1
#define  DT_CHR         2
#define  DT_DIR         4
#define  DT_BLK         6
#define  DT_REG         8
#define  DT_LNK         10
#define  DT_SOCK        12
#define  DT_WHT         14
#endif


struct dirent {
  uint64_t         d_ino;
  int64_t          d_off;
  unsigned short   d_reclen;
  unsigned char    d_type;
  char             d_name[256];
}  __attribute__((packed)) ;


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
int lstat(const char *pathname, struct stat *buf);
int stat(const char *pathname, struct stat *buf);
int fstat(unsigned int fd, struct stat *buf);
int pipe(int file_descriptors[2]);
pid_t fork(void);
int execve(const char *filename, char *const argv[], char *const envp[]);
void *sbrk(unsigned long increment);
int rename(const char *old_name, const char *new_name);

//Dir api
char *getcwd(char *buf, size_t size);
int mkdir(const char *dir_name, mode_t mode);
int chdir(const char *new_directory);
int rmdir(const char *path);
int getdents(int fd, struct dirent *dirp,
            unsigned int count);

//Custom api
void print_dir_elements(const char*);
typedef struct disk_info{
 uint32_t total_blocks;
 uint32_t free_blocks;
 uint32_t total_inodes;
 uint32_t free_inodes;
} disk_info;    
void fs_info(disk_info* info);
void void_call();
int access(const char *f, int flag);

//Display syscalls
struct display_info{
  int width;
  int height;
};
int upd_data_display(void* data, int x, int y, int width, int height);
void get_display_info(struct display_info*);

/*
 * RISC-V asm
 */
#define csr_read(csr)														\
({																			\
	register unsigned long __v;													\
	__asm__ __volatile__ ("csrr %0, " #csr : "=r" (__v) : : "memory");		\
	__v;																	\
})

#endif /* _SYSAPI_H_ */
