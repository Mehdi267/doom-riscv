#ifndef SYNC_H_
#define SYNC_H_
#include "process/process.h"
#include <stdint.h>


extern uint64_t counter;
/**
 * @brief return the frequency of the clock and the number of clock oscillations
 * between affecting the pointers passed in parameter
*/
void clock_settings(unsigned long *quartz, unsigned long *ticks);

/**
 * @return the number of interruption since the start of the system
*/
uint64_t current_clock();


/**
* @brief the current process sleeps until the interruption number clock arrives
*/
void wait_clock(uint64_t clock);

/**
* @brief just a wrapper of wait_clock to simplify use
* @param nb_sec: number of seconds that the current process should wait
*/
void sleep(uint64_t nb_sec);

#endif // SYNC_H_
