#include "timer_api.h"
#include "scheduler.h" //for scheduler
#include "../timer.h" // for TIC_PER cte
#include <stdint.h>
#include "assert.h"
#include "helperfunc.h" // get_current_process
#include "drivers/clint.h" // for clint_dev
#include "process.h"

void clock_settings(unsigned long *quartz, unsigned long *ticks) {
  *quartz = clint_dev->clk_freq;
  // ticks number of oscillations between 2 interrupts
  *ticks = clint_dev->clk_freq / 1000 * TIC_PER;
}

uint64_t counter = 0; // counter is incremented in timer.c
uint64_t current_clock() {
  return counter;
}

void wait_clock(uint64_t clock) {
  process * current_process = get_current_process();
  current_process->wake_time = - (current_clock() + clock);
  current_process->state = ASLEEP;
  queue_add(current_process, &asleep_process_queue, process, next_prev, wake_time);
  scheduler();
}


void sleep(uint64_t nbr_sec) { // TODO check overflows
  wait_clock(1000 / TIC_PER * nbr_sec);
}
