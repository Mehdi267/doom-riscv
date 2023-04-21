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
#endif
