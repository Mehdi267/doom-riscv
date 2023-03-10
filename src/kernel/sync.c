#include "sync.h"
#include "process/scheduler.h"
#include "timer.h"
#include <stdint.h>
#include "process/helperfunc.h"
#include "process/process.h"
#include "drivers/clint.h"

void clock_settings(unsigned long *quartz, unsigned long *ticks) {
  *quartz = clint_dev->clk_freq;
  *ticks = clint_dev->clk_freq / 1000 * TIC_PER; // ticks number of oscillations between 2 interrupts
}

uint64_t counter = 0; // counter is incremented in timer.c
uint64_t current_clock() {
  return counter;
}

void wait_clock(uint64_t clock) {
  process * current_process = get_process_struct_of_pid(getpid());
  current_process->sleep_time = - (current_clock() + clock);
  current_process->state = ASLEEP;
  add_process_to_queue_wrapper(current_process, ASLEEP_QUEUE);
  scheduler();
}


void sleep(uint64_t nbr_sec) { // TODO check overflows
  wait_clock(1000 / TIC_PER * nbr_sec);
}
