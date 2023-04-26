/**
 * Projet PCSEA RISC-V
 * Mehdi Frikha
 * See license for license details.
 */

#include "process.h"
#include "memory_api.h"
#include "../tests/tests.h"
#include "../timer.h"
#include "hash.h"
#include "helperfunc.h"
#include "mem.h"
#include "scheduler.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "semaphore_api.h"
#include "riscv.h"

int initialize_process_hash_table() {
  pid_process_hash_table = (hash_t *)malloc(sizeof(hash_t));
  if (pid_process_hash_table == NULL) {
    return -1;
  }
  return 0;
}

void activate_and_launch_scheduler(void){
    #ifdef VIRTMACHINE
      csr_set(sstatus, MSTATUS_SIE);
      set_machine_timer_interrupt(100);
    #else
      set_supervisor_timer_interrupt(100); 
      csr_set(sstatus, MSTATUS_SIE);
    #endif
    while(1){ wfi();}
    return;
}



/**
 * @brief setup_main_context is used to allocate space for a scheduler_struct
 * in which we will store the main exection context when the scheduler is first
 * called
 * @return 0 if the declaration is everything goes well and -1 in case of an
 * error
 */
static int setup_main_context() {
  scheduler_main = (scheduler_struct *)malloc(sizeof(scheduler_struct));
  if (scheduler_main == NULL) {
    return -1;
  }
  scheduler_main->main_context = (context_t *)malloc(sizeof(context_t));
  if (scheduler_main->main_context == NULL) {
    return -1;
  }
  return 0;
}

static int create_idle_process() {
  #if defined USER_PROCESSES_ON
    int pid_idle;
    pid_idle = start_virtual("idle", 4000, 1,cast_int_to_pointer(300));
    return pid_idle;
  #elif defined KERNEL_PROCESSES_ON
    int pid_idle;
    pid_idle = start(idle, 4000, 1, "idle",cast_int_to_pointer(300));
    return pid_idle;
  #else
    # error either "USER_PROCESSES_ON" or "KERNEL_PROCESSES_ON" must be defined
    return 0;
  #endif
}

#ifdef TESTING
static int create_testing_process() {
    #ifdef USER_PROCESSES_ON
      int pid_test;
      pid_test = start_virtual("autotest", 4000, 1,cast_int_to_pointer(300));
      return pid_test;
    #endif
    #ifdef KERNEL_PROCESSES_ON
      int pid_test;
      pid_test = start(kernel_tests, 4000, 2, "kernel_tests",cast_int_to_pointer(300));
      return pid_test;
    #else
      return 0;
    #endif
  return 0;
}
#endif

int create_testing_process() {
    int pid_test;
    pid_test = start_virtual("shell", 4000, 1,cast_int_to_pointer(300));
    return pid_test;
}

int activate_and_launch_custom_process(process *process_to_activate) {
  // This mehtod is mostly used for debugging, use with care
  if (process_to_activate == NULL) {
    return -1;
  }
  if (getpid() > 0) {
    // An actif process is already running,
    // this method is used  only to launch the first process
    return -1;
  }
  if (setpid(process_to_activate->pid) < 0) {
    return -1;
  }
  if (process_to_activate->state != ACTIVATABLE) {
    return -1;
  }
  delete_process_from_queue_wrapper(process_to_activate, ACTIVATABLE_QUEUE);
  process_to_activate->state = ACTIF;

  set_supervisor_timer_interrupt(100);
  while (1) {
  }
  return 0;
}

void validation_process() {
  uint8_t variable = 42;
  uint8_t *ptr1 = &variable;
  uint8_t *ptr2 = ptr1 + 0x100000000;

  printf("ptr1 @%p = %u\n", ptr1, *ptr1);
  printf("ptr2 @%p = %u\n", ptr2, *ptr2);
  if (*ptr1 == *ptr2)
    puts("Test mémoire virtuelle OK");
  else
    puts("Test mémoire virtuelle FAIL");
}

int idle(void *arg) {
  debug_print_no_arg("hello world");
  debug_print_process(
      "[Current process = %s] pid = %d; argument given = %ld \n",
      get_pid_name(getpid()), getpid(), cast_pointer_into_a_long(arg));
  // we check for potential pending interrupts
  while(true) {
    uint64_t tmp_status;
    tmp_status = csr_read(sstatus);
    csr_set(sstatus, MSTATUS_SIE);
    __asm__ __volatile__("nop");
    csr_write(sstatus, tmp_status);
  };
}


int process_1(void *arg) {
  int i = 0;
  for (;;) {
    i++;
    debug_print_process(
        "[Current process = %s] pid = %d; argument given = %ld \n",
        get_pid_name(getpid()), getpid(), cast_pointer_into_a_long(arg));
    // scheduler();
    if (i == 6) {
      return 1;
    }
  }
}

int process_2(void *arg) {
  int i = 0;
  for (;;) {
    i++;
    debug_print_process(
        "[Current process = %s] pid = %d; argument given = %ld \n",
        get_pid_name(getpid()), getpid(), cast_pointer_into_a_long(arg));
    // scheduler();
  }
}

/**
 * @brief Defines random processes that are used for debugging purposes
 */
static int declares_debug_processes() {
#ifdef DEBUG_SCHEDULER
  int p1 = start(process_1, 1000, 1, "proc1", cast_int_to_pointer(100));
  int p2 = start(process_2, 1000, 1, "proc2", cast_int_to_pointer(200));
  if (p1 < 0 || p2 < 0) {
    return -1;
  }
#endif
  return 0;
}


int initialize_process_workflow(){
    init_scheduling_process_queue();
    if (initialize_process_hash_table()<0){
        return -1;
    }
    if (hash_init_direct(pid_process_hash_table)<0){
        return -1;
    }
    if (initialize_share_pages_table()<0){
        return -1;
    }
    if (hash_init_string(shared_memory_hash_table)){
        return -1;
    }
    if (init_semaphore_table()<0){
        return -1;
    }
    if (hash_init_direct(semaphore_table)<0){
        return -1;
    }
    if (setup_main_context() <0){;
        return -1;
    }
    if (create_idle_process()<0){
        return -1;
    }
    #ifdef TESTING
      if (create_testing_process()<0){
        return -1;
      }
    #else
      if (create_shell_program()<0){
        return -1;
      }
    #endif
    #ifdef USER_PROCESSES_ON
      csr_clear(sstatus, MSTATUS_SPP);
    #endif 
    //Will only launch the process if the debug mode is set
    if (declares_debug_processes()<0){
        return -1;
    }
    return 0;
}
