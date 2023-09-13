#ifndef TIME_H
#define TIME_H

#include "stdint.h"

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


// Constants for clock IDs
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3

// Constants for timer creation
#define TIMER_ABSTIME 1

// Constants for nanosleep
#define TIMER_ABSTIME 1

// Data structures
struct timeval64 {
    uint64_t tv_sec;     // Seconds
    uint64_t tv_usec;    // Microseconds
};

struct timespec64 {
    uint64_t tv_sec;     // Seconds
    uint64_t tv_nsec;    // Nanoseconds
};

typedef int clockid_t; 
// System calls
time_t time(time_t *tloc);
int gettimeofday(struct timeval *tv, struct timezone *tz);
int settimeofday(const struct timeval *tv, const struct timezone *tz);
// int clock_gettime(clockid_t clk_id, struct timespec *tp);

#endif // TIME_CONSTANTS_H

