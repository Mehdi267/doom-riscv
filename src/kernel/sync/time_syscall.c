#include "time_syscall.h"
#include "stddef.h"
#include "timer.h"
#include "stdio.h"

// Placeholder value for the current time (in seconds since epoch)
static time_t current_time_s = 1632268800; 
static time_t current_time_usec = 1632268800000000; 

//Place holder functions
time_t time(time_t *tloc) {
  time_t return_time_s = current_time_s+time_counter*TIC_PER/1000;
  if (tloc != NULL) {
    *tloc = return_time_s;
  }
  return return_time_s;
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
  time_t return_time_s = current_time_s+time_counter*TIC_PER/1000;
  time_t return_time_us = current_time_usec+time_counter*TIC_PER*1000;
  if (tv != NULL) {
    tv->tv_sec = return_time_s;
    tv->tv_usec = return_time_us; // Placeholder microseconds
  }
  if (tz != NULL) {
    tz->tz_minuteswest = 0;
    tz->tz_dsttime = 0;
  }
  return 0; // Return success
}

int settimeofday(const struct timeval *tv, const struct timezone *tz) {
  if (tv != NULL) {
    current_time_s = tv->tv_sec;
    current_time_usec = tv->tv_usec;
    time_counter = 0;
    return 0; // Return success
  }

  return -1; // Return an error if 'tv' is NULL
}
