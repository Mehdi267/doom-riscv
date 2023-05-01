/*
 * Damien Dejean - Gaëtan Morin
 * Mathieu Barbe
 * Ensimag, Projet Système 2010
 *
 * XUNIL
 * Headers de la bibliothèque d'appels systèmes.
 */
#ifndef ___SYSCALL_H___
#define ___SYSCALL_H___

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
extern int waitpid(int pid, long int *retval);

extern void *shm_create(const char*);
extern void *shm_acquire(const char*);
extern void shm_release(const char*);

extern void power_off(int exit_value);
extern void show_ps_info();
extern void show_programs();
extern void info_queue();
#endif
