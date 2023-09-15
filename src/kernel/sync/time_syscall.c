#include "time_syscall.h"
#include "stddef.h"

// Placeholder value for the current time (in seconds since epoch)
static time_t current_time = 1632268800; 

//Place holder functions
time_t time(time_t *tloc) {
  if (tloc != NULL) {
    *tloc = current_time;
  }
  return current_time;
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
  current_time++;
  if (tv != NULL) {
    tv->tv_sec = current_time;
    tv->tv_usec = 0; // Placeholder microseconds
  }

  if (tz != NULL) {
    tz->tz_minuteswest = 0;
    tz->tz_dsttime = 0;
  }

  return 0; // Return success
}

int settimeofday(const struct timeval *tv, const struct timezone *tz) {
  if (tv != NULL) {
    current_time = tv->tv_sec;
    return 0; // Return success
  }

  return -1; // Return an error if 'tv' is NULL
}
