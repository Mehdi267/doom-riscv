#include "timer_api.h"
#include "../timer.h" // for TIC_PER cte
#include <stdint.h>
#include "assert.h"
#include "drivers/clint.h" // for clint_dev
#include "../process/helperfunc.h" // get_current_process
#include "../process/scheduler.h" //for scheduler
#include "../process/process.h"

//This variable is global and used to indicate the time evolution since the machine started
uint64_t time_counter = 0; // time_counter is incremented in timer.c
void clock_settings(unsigned long *quartz, unsigned long *ticks) {
  *quartz = clint_dev->clk_freq;
  // ticks number of oscillations between 2 interrupts
  *ticks = clint_dev->clk_freq / 1000 * TIC_PER;
}

uint64_t current_clock() {
  return time_counter;
}

void wait_clock(uint64_t clock) {
  process * current_process = get_current_process();
  current_process->wake_time = - (current_clock() + clock);
  current_process->state = ASLEEP;
  queue_add(current_process, &asleep_process_queue, process, next_prev, wake_time);
  scheduler();
}


void sleep(uint64_t nbr_ms) { // TODO check overflows
  /* printf("1: %lu\n", 1000 / TIC_PER * nbr_sec); */
  /* printf("2: %lu\n", TIC_PER * nbr_sec); */
  /* wait_clock(1000 / TIC_PER * nbr_sec); */
  printf("Sleep called\n");
  wait_clock(nbr_ms / TIC_PER);
}
