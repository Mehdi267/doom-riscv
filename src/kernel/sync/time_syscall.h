#ifndef ___SYSCALL_TIME_
#define ___SYSCALL_TIME_

// Data structures
typedef signed long time_t;	
typedef signed long suseconds_t;	
struct timeval{
  time_t tv_sec;		/* Seconds.  */
  suseconds_t tv_usec;	/* Microseconds.  */
};

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

time_t time(time_t *tloc);
int gettimeofday(struct timeval *tv, struct timezone *tz);
int settimeofday(const struct timeval *tv, const struct timezone *tz);

#endif