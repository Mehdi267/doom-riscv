#include "scheduler.h"
#include "../memory/frame_dist.h"
#include "assert.h"
#include "hash.h"
#include "helperfunc.h"
#include "mem.h"
#include "memory_api.h"
#include "process.h"
#include "queue.h"
#include "riscv.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "timer_api.h"
#include <stdint.h>

LIST_HEAD(blocked_io_process_queue);
LIST_HEAD(activatable_process_queue);
LIST_HEAD(asleep_process_queue);
LIST_HEAD(dead_process_queue);

// initially user process are not on, this will become true when we first visit
// the scheduler
bool started_user_process = false;
scheduler_struct *scheduler_main = NULL;

/*
** HELPER FUNCTIONS DECLARATIONS
*/
uint64_t get_next_wake_time();
void awake_sleeping_process();
void free_dead_process();

void scheduler() {
#ifdef USER_PROCESSES_ON
  // We need to go back to user mode when the scheduler is called
  csr_clear(sstatus, MSTATUS_SPP);
#endif
  debug_print_scheduler_no_arg(
      "\n-----------------Scheduler--------------------\n");
  debug_print_scheduler(
      "[scheduler -> %d] Inside the scheduler with pid equal to %d \n",
      getpid(), getpid());

  // awake process and insert the in activable queue;
  awake_sleeping_process();

  // free the memory of the processes that were killed and they are orphans
  free_dead_process();
  // Scheduler has been called before any execution has started
  // the scheduler_main is configured when we start the kernel if its value is
  // null then we found an error
  //
  if (scheduler_main == NULL) {
    return;
  }
  // Scheduler has been called directly for the first time
  if (getpid() == -1) { // pid has not been set yet
    process *top_process =
        queue_top(&activatable_process_queue, process, next_prev);
    if (top_process == NULL) {
      return;
    }
    queue_out(&activatable_process_queue, process, next_prev);
    if (setpid(top_process->pid) < 0) {
      return;
    }
    /* set_supervisor_interrupts(true); */
    debug_print_scheduler(
        "[scheduler -> %d] Inside the scheduler with no process running, "
        "default launch of the peek process with id = %d\n",
        getpid(), getpid());
    debug_print_scheduler("[scheduler -> %d] running process name = %s\n",
                          getpid(), getname());
    top_process->state = ACTIF;
    started_user_process = true;
    context_switch(
        scheduler_main->main_context,
        top_process->context_process); // pid is set by the activate method
  }
  // Custom process launch enable // the user must set a valid pid
  if (started_user_process == false) {
    // In this case no process is running and we have called the scheduler for
    // the first time
    //  yet in this case, we want to start with a custom process/ that has been
    //  set by the user in the pid field and we also assume that user has also
    //  eliminated the process from the queue
    process *top_process = get_process_struct_of_pid(getpid());
    if (top_process == NULL) {
      return;
    }
    started_user_process = true;
    debug_print_scheduler(
        "[scheduler -> %d] Inside the scheduler with no process running, "
        "custom launch of the process with id = %d\n",
        getpid(), getpid());
    debug_print_scheduler("[scheduler -> %d] running process name = %s\n",
                          getpid(), getname());
    debug_print_scheduler(
        "[scheduler -> %d] function adress of the process = %ld\n", getpid(),
        (long)get_process_struct_of_pid(getpid())->context_process->s[1]);
    debug_print_scheduler("[scheduler -> %d] idle adress = %ld\n", getpid(),
                          (long)idle);
    context_switch(scheduler_main->main_context,
                   get_process_struct_of_pid(getpid())
                       ->context_process); // pid is set by the activate method
  } else {
    // In here we treat the normal case
    // We take the current process struct:
    process *current_process = get_current_process();
    process *top_process =
        queue_top(&activatable_process_queue, process, next_prev);
    if (top_process == NULL || current_process == NULL) {
      return;
    }
    debug_print_scheduler(
        "[scheduler -> %d] current process pid = %d, peek pid = %d\n", getpid(),
        current_process->pid, top_process->pid);
    debug_print_scheduler("[scheduler -> %d] [current process name --> |%s|], "
                          "[peek name --> |%s|]\n",
                          getpid(), current_process->process_name,
                          top_process->process_name);
    debug_print_scheduler(
        "[scheduler -> %d] current process priority = %d, peek priority = %d\n",
        getpid(), current_process->prio, top_process->prio);

    // If the process was actif when this treatement was called, then
    // we swap the process if the peek prio is higher then the current prio
    // and we place the old process if the activatable queue
    if (current_process->state == ACTIF) {
      if (top_process->prio >= current_process->prio) {
        // In this case we switch the process and
        debug_print_scheduler("[scheduler -> %d] Trying to swap from process "
                              "current pid = %d ->>>>>>>> peek pid = %d\n",
                              getpid(), current_process->pid, top_process->pid);
        queue_out(&activatable_process_queue, process, next_prev);
        if (setpid(top_process->pid) < 0) {
          return;
        }
        current_process->state = ACTIVATABLE;
        /* set_supervisor_interrupts(true); */
        top_process->state = ACTIF;
        queue_add(current_process, &activatable_process_queue, process,
                  next_prev, prio);
        debug_print_scheduler("[scheduler -> %d] Swapping processes;  current "
                              "pid = %d ->>>>>>>> peek pid = %d\n",
                              top_process->pid, current_process->pid,
                              top_process->pid);
        context_switch(current_process->context_process,
                       top_process->context_process);
      } else {
        debug_print_scheduler(
            "[scheduler -> %d] Sticking with the same process id = %d\n",
            getpid(), current_process->pid);
      }
    }
    // If the process was killed we free its data
    // and jump directly into an other context
    else if (current_process->state == KILLED) {
      debug_print_scheduler("[scheduler -> %d] I am in a killed process id = "
                            "%d, moving to process = %d\n",
                            getpid(), current_process->pid, top_process->pid);
      queue_add(current_process, &dead_process_queue, process, next_prev, prio);
      queue_out(&activatable_process_queue, process, next_prev);
      if (setpid(top_process->pid) < 0) {
        return;
      }
      /* set_supervisor_interrupts(true); */
      top_process->state = ACTIF;

      direct_context_swap(top_process->context_process);
    }
    // if the process was placed in an other state when this was called
    // then we change the context directly and we don't place the process in the
    // activatable queue
    else {
      debug_print_scheduler("[scheduler -> %d] Current process is not in a "
                            "actif state= %d swaping to %d\n",
                            getpid(), current_process->pid, top_process->pid);
      queue_out(&activatable_process_queue, process, next_prev);
      if (setpid(top_process->pid) < 0) {
        return;
      }
      /* set_supervisor_interrupts(true); */
      top_process->state = ACTIF;
      context_switch(current_process->context_process,
                     top_process->context_process);
    }
  }
  debug_print_scheduler(
      "[scheduler -> %d] I managed to return to the scheduler %d\n", getpid(),
      getpid());
}

/*
** HELPER FUNCTIONS DEFINITIONS
*/
/**
 *@brief returns the smallest wake time of the top process from the
 * asleep_process_queue
 */
uint64_t get_next_wake_time() {
  process *p = queue_top(&asleep_process_queue, process, next_prev);
  if (p) {
    assert(-p->wake_time >= 0);
    return -p->wake_time;
  }
  return 0;
}

/**
 *@brief awakes sleeping process if the current time is > of their wake time
 */
void awake_sleeping_process() {
  uint64_t wake_time = get_next_wake_time();
  while (wake_time != 0 &&
         current_clock() > wake_time) { // several process can be awaken
    process *awake = queue_out(&asleep_process_queue, process, next_prev);
    queue_add(awake, &activatable_process_queue, process, next_prev, prio);
    wake_time = get_next_wake_time();
  }
}

/**
 * @brief free the dead process queue
 */
void free_dead_process() {
  while (!queue_empty(&dead_process_queue)) {
    process *top = queue_out(&dead_process_queue, process, next_prev);
    free_process_memory(top);
  }
}
