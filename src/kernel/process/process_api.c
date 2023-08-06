//Basic import
#include "assert.h" //for assert func
#include "riscv.h" //for riscv related inst and special values
#include "stdlib.h" // for uint
#include "string.h" // for strcpy strlen
#include "stdbool.h" // for the bool type
#include "stdio.h" // for print and related files
#include "hash.h" //hash table

//
#include "helperfunc.h"
#include "process.h"
#include "scheduler.h"

//Memory related
#include "../memory/frame_dist.h"
#include "../memory/pages.h"
#include "../memory/virtual_memory.h"
#include "memory_api.h"
#include "process_memory.h"

//Other imports
#include "bios/info.h"
#include "drivers/splash.h"
#include "encoding.h"
#include "traps/trap.h"
#include "../sync/semaphore_api.h"
#include "timer.h"

//Fs imports
#include "../fs/fs_api.h" //for open and dup2 syscall

#define FLOAT_TO_INT(x) (int)((x) + 0.5)


struct process_management_global proc_mang_g;

/*
** HELPER FUNCTIONS DECLARATIONS
*/
static int free_child_zombie_process(process *process_to_free);
static int free_process_arg_and_fix_tree_link(process *process_to_free,
                                              process *before_process);
static int make_children_orphans_and_kill_zombies(process *parent_process);
static int turn_current_process_into_a_zombie_or_kill_it(bool current_or_custom,
                                                         int pid);
static int process_name_copy(process *p, const char *name);

/*
** MAIN FUNCTIONS
*/

int getpid(void) { return proc_mang_g.current_running_process_pid; }

int getprio(int pid) {
  process *process_pid = ((process *)hash_get(get_process_hash_table(),
                                              cast_int_to_pointer(pid), NULL));
  if (process_pid == NULL) {
    return -1;
  }
  if (validate_action_process_valid(process_pid) < 0) {
    return -1;
  }
  return process_pid->prio;
}

int setpid(int new_pid) {
  // We start by checking that the process exists
  process *process_pid = ((process *)hash_get(
      get_process_hash_table(), cast_int_to_pointer(new_pid), NULL));
  if (process_pid == NULL) {
    return -1;
  }
  proc_mang_g.current_running_process_pid = new_pid;
  return new_pid;
}

int chprio(int pid, int newprio) {
  process *p = ((process *)hash_get(get_process_hash_table(),
                                    cast_int_to_pointer(pid), NULL));
  if (p == NULL) {
    return -1;
  }
  if (newprio > MAXPRIO || newprio < MINPRIO) {
    return -1;
  }
  if (validate_action_process_valid(p) < 0) {
    return -1;
  }
  uint16_t old_prio = p->prio;
  p->prio = newprio;
  switch (p->state) {
  case ACTIVATABLE:
    // no break will call the next switch case
    queue_del(p, next_prev);
    queue_add(p, &activatable_process_queue, process, next_prev, prio);
  case ACTIF:
    scheduler();
    break;
  case BLOCKEDQUEUE:
    queue_del(p, next_prev);
    queue_add(p, p->message.blocked_head, process, next_prev, prio);
    break;
  case BLOCKEDIO:
    queue_del(p, next_prev);
    queue_add(p, &blocked_io_process_queue, process, next_prev, prio);
  default:
    /* BLOCKEDSEMAPHORE, */
    /* BLOCKEDWAITCHILD, */
    /* ASLEEP, */
    /* ZOMBIE, */
    /* KILLED */
    break;
  }
  return old_prio;
}

int leave_queue_process_if_needed(process *leaving_process) {
  if (leaving_process == NULL) {
    return -1;
  }

  debug_print_exit_m(
      "\nTrying to remove %s from a queue id %d process state  = %d\n",
      leaving_process->process_name, leaving_process->pid,
      leaving_process->state);
  if (leaving_process->state == BLOCKEDSEMAPHORE) {
    if (proc_kill_diag(leaving_process->semaphore_id, KILL_CALL,
                       leaving_process->pid) < 0)
      return -1;
  } else {
    queue_del(leaving_process, next_prev);
  }
  return 0;
}

int check_if_new_prio_is_higher_and_call_scheduler(int newprio,
                                                   bool current_or_queue,
                                                   int prio_to_compare_queue) {
  if (!(newprio <= MAXPRIO && newprio >= MINPRIO)) {
    return -1;
  }
  if (current_or_queue == true) {
    int currently_running_prio = getprio(getpid());
    if (currently_running_prio < 0) {
      return -1;
    }
    if (currently_running_prio < newprio) {
      scheduler();
    }
  } else {
    if (prio_to_compare_queue < 0) {
      return -1;
    }
    if (prio_to_compare_queue > newprio) {
      scheduler();
    }
  }
  return 0;
}

void exit_process(int retval) {
  debug_print_exit_m("I am in exit method with argument/ temp pid = %d \n",
                     retval);
  if (validate_action_process_valid(get_process_struct_of_pid(getpid())) < 0) {
    // Something went terribly wrong if we are in here
    exit(-1);
  }
  if (turn_current_process_into_a_zombie_or_kill_it(true, 0) < 0) {
    // Something went terribly wrong if we are in here
    exit(-1);
  }
  get_process_struct_of_pid(getpid())->return_value = retval;
  scheduler();
}
int start(int (*pt_func)(void *), unsigned long ssize, int prio,
          const char *name, void *arg) {
  if (name == NULL) {
    return -1;
  }
  debug_print_no_arg("----------------Start process---------------\n");
  debug_print("Trying to create a new process with the name  = %s \n", name);
  if (++proc_mang_g.nb_proc_running > MAX_NB_PROCESS) {
    proc_mang_g.nb_proc_running--;
    return -1;
  }
  //-------------------------------Input check--------------

  // We verify that the process that made this call is a valid process ie not a
  // zombie
  if (!(getpid() == -1) &&
      validate_action_process_valid(get_process_struct_of_pid(getpid())) < 0) {
    return -1;
  }
  // We check that the function arguments are valid
  if (!(prio <= MAXPRIO && prio >= MINPRIO))
    return -1;

  // Naif check, we can do a thorough check of memory at this level
  // or we can do that using memory api methods
  if (!(ssize > 0))
    return -1;

  //----------Process generation-----------

  process *new_process;
  secmalloc(new_process, sizeof(process));
  // somehow those pointers are not null sometimes
  new_process->next_prev.next = NULL;
  new_process->next_prev.prev = NULL;

  //---------Create a new pid and and new process to hash
  // table----------------

  new_process->pid = increment_pid_and_get_new_pid();
  hash_set(get_process_hash_table(), cast_int_to_pointer(new_process->pid),
           new_process);

  new_process->prio = prio;                     // Priority config
  if (process_name_copy(new_process, name) < 0) // this function fails if size
    return -1;

  // We add PROCESS_SETUP_SIZE because we need space to call the function
  // and in order to place the exit method in the stack
  new_process->ssize = ssize + PROCESS_SETUP_SIZE;
  new_process->page_table_level_2 = NULL;
  new_process->page_tables_lvl_1_list = NULL;
  new_process->released_pages_list = NULL;

  if (process_memory_allocator(new_process, new_process->ssize) < 0) {
    print_memory_no_arg("Memory is full");
    return -1;
  }
  //--------------------Process function config-----------
  new_process->func = pt_func;

  //----------------Context setup-------------------------
  new_process->context_process = (context_t *)malloc(sizeof(context_t));
  if (new_process->context_process == NULL) {
    return -1;
  }

  // new_process->context_process->sp = (uint64_t) frame_pointer;
  new_process->context_process->sp =
      (uint64_t)0x40000000 + FRAME_SIZE * new_process->stack_shift;
  // During the context_switch we will call the process_call_wrapper that has
  // to call the method given as function argument that we placed in s1 also
  // the call has to be made the right argument that is in s2 and it also has
  // to call the exit_process method at the end as this will be important in
  // the case the user uses a return call
  new_process->context_process->ra = (uint64_t)process_call_wrapper_kernel;
  new_process->context_process->s[1] = (uint64_t)pt_func;
  // debug_print("[start -> %d] function adress funciton adress = %ld\n",
  // new_process->pid, (long) pt_func);
  new_process->context_process->s[2] = (uint64_t)arg;
  new_process->context_process->sepc = (uint64_t)process_call_wrapper_kernel;
  new_process->context_process->satp =
      0x8000000000000000 |
      ((long unsigned int)new_process->page_table_level_2 >> 12) |
      ((long unsigned int)new_process->pid << 44);
  // We must created a stack that has the size of a frame and place it in the
  // kernel memory space that will be used to handle interrupts for this
  // process

  void *interrupt_frame_pointer = get_frame();
  debug_print_memory("sscratch frame %p \n", interrupt_frame_pointer);
  if (interrupt_frame_pointer == NULL) {
    return -1;
  }
  new_process->sscratch_frame = interrupt_frame_pointer;
  new_process->context_process->sscratch =
      (uint64_t)interrupt_frame_pointer + FRAME_SIZE;

  //--------------Tree management----------------
  // The parent of the process is the process that called the start method
  new_process->parent = (process *)hash_get(
      get_process_hash_table(), cast_int_to_pointer(getpid()), NULL);

  // if the parent process is null that means we created the head of the tree
  // thus the new process is not attached to a parent
  if (new_process->parent != NULL) {
    // We add the new process as a child to the parent process
    if (new_process->parent->children_tail != NULL) {
      new_process->parent->children_tail->next_sibling = new_process;
      new_process->parent->children_tail = new_process;
    } else {
      new_process->parent->children_head = new_process;
      new_process->parent->children_tail = new_process;
    }
  }
  new_process->children_head = NULL;
  new_process->children_tail = NULL;
  new_process->next_sibling = NULL;

  //--------------Return value----------------
  new_process->return_value = NULL;

  //-------------Shared pages-------------------
  new_process->shared_pages = NULL;
  new_process->released_pages_list = NULL;
  new_process->proc_shared_hash_table = NULL;
  //--------------Semaphore signal-----------
  new_process->sem_signal = 0;
  //------------Add process to the activatable queue
  new_process->state = ACTIVATABLE;
  queue_add(new_process, &activatable_process_queue, process, next_prev, prio);

  debug_print("[%s] created process with pid = %d \n",
              new_process->process_name, new_process->pid);

  //------------We activate this new process if it has a higher
  // priority-----------
  // This function must be called a the very end
  check_if_new_prio_is_higher_and_call_scheduler(new_process->prio, true, 0);
  return new_process->pid;
}

int start_virtual(const char *name, unsigned long ssize, int prio, void *arg) {
  if (name == NULL) {
    return -1;
  }
  debug_print_no_arg("----------------Start process---------------\n");
  debug_print("Trying to create a new process with the name  = %s \n", name);
  if (++proc_mang_g.nb_proc_running > MAX_NB_PROCESS) {
    proc_mang_g.nb_proc_running--;
    return -1;
  }
  //-------------------------------Input check--------------

  // We verify that the process that made this call is a valid process is not a
  // zombie
  if (!(getpid() == -1) &&
      validate_action_process_valid(get_process_struct_of_pid(getpid())) < 0) {
    return -1;
  }
  // We check that the function arguments are valid
  if (!(prio <= MAXPRIO && prio >= MINPRIO))
    return -1;

  // Naif check, we can do a thorough check of memory at this level
  // or we can do that using memory api methods
  if (!(ssize > 0))
    return -1;

  if (!find_app(name)) {
    return -1;
  }
  //----------Process generation-----------

  process *new_process;
  secmalloc(new_process, sizeof(process));
  // somehow those pointers are not null sometimes
  new_process->next_prev.next = NULL;
  new_process->next_prev.prev = NULL;

  //---------Create a new pid and and new process to hashtable----------------
  new_process->pid = increment_pid_and_get_new_pid();
  hash_set(get_process_hash_table(), cast_int_to_pointer(new_process->pid),
           new_process);

  new_process->prio = prio;                     // Priority config
  if (process_name_copy(new_process, name) < 0) // this function fails if size
    return -1;

  // We add PROCESS_SETUP_SIZE because we need space to call the function
  // and in order to place the exit method in the stack
  new_process->ssize = ssize + PROCESS_SETUP_SIZE;
  new_process->page_table_level_2 = NULL;
  new_process->page_tables_lvl_1_list = NULL;
  new_process->released_pages_list = NULL;
  // We need to find the application related to the process, it is at that
  // address where the code will be stored
  new_process->app_pointer = find_app(name);
  if (new_process->app_pointer == NULL) {
    // Cannot locate app code
    return -1;
  }
  if (process_memory_allocator(new_process, new_process->ssize) < 0) {
    print_memory_no_arg("Memory is full");
    return -1;
  }
  //----------------Context setup-------------------------
  new_process->context_process = (context_t *)malloc(sizeof(context_t));
  if (new_process->context_process == NULL) {
    return -1;
  }

  new_process->context_process->sp =
      (uint64_t)0x40000000 + FRAME_SIZE * new_process->stack_shift;
  // During the context_switch we will call the process_call_wrapper that has
  // to call the method given as function argument that we placed in s1 also
  // the call has to be made the right argument that is in s2 and it also has
  // to call the exit_process method at the end as this will be important in
  // the case the user uses a return call
  new_process->context_process->ra = (uint64_t)process_call_wrapper_user;
  new_process->context_process->sepc = 0x40000000;
  // new_process->context_process->sepc = (uint64_t)idle;
  new_process->context_process->s[2] = (uint64_t)arg;
  new_process->context_process->satp =
      0x8000000000000000 |
      ((long unsigned int)new_process->page_table_level_2 >> 12) |
      ((long unsigned int)new_process->pid << 44);
  // We must created a stack that has the size of a frame and place it in the
  // kernel memory space that will be used to handle interrupts for this
  // process

  void *interrupt_frame_pointer = get_frame();
  debug_print_memory("sscratch frame %p \n", interrupt_frame_pointer);
  if (interrupt_frame_pointer == NULL) {
    return -1;
  }
  new_process->sscratch_frame = interrupt_frame_pointer;
  new_process->context_process->sscratch =
      (uint64_t)interrupt_frame_pointer + FRAME_SIZE;

  //--------------Tree management----------------
  // The parent of the process is the process that called the start method
  new_process->parent = (process *)hash_get(
      get_process_hash_table(), cast_int_to_pointer(getpid()), NULL);

  // if the parent process is null that means we created the head of the tree
  // thus the new process is not attached to a parent
  if (new_process->parent != NULL) {
    // We add the new process as a child to the parent process
    if (new_process->parent->children_tail != NULL) {
      new_process->parent->children_tail->next_sibling = new_process;
      new_process->parent->children_tail = new_process;
    } else {
      new_process->parent->children_head = new_process;
      new_process->parent->children_tail = new_process;
    }
  }
  new_process->children_head = NULL;
  new_process->children_tail = NULL;
  new_process->next_sibling = NULL;

  //--------------Return value----------------
  new_process->return_value = NULL;

  //-------------Shared pages-------------------
  new_process->shared_pages = NULL;
  new_process->released_pages_list = NULL;
  new_process->proc_shared_hash_table = NULL;
  //--------------Semaphore signal-----------
  new_process->sem_signal = 0;
  //------------Add process to the activatable queue-------
  new_process->state = ACTIVATABLE;
  //------------File system related configuration----------
  #ifdef VIRTMACHINE
    //Bad code improve later
    char root_char[1] = "/";
    //The reason as to why we use 2 is for the null terminator
    new_process->root_dir.inode = get_inode(EXT2_GOOD_OLD_FIRST_INO);
    new_process->root_dir.dir_name = (char*) malloc(2);
    new_process->root_dir.name_size = 2;
    strcpy(new_process->root_dir.dir_name, root_char);
    new_process->root_dir.dir_name[1] = '\0';
    new_process->cur_dir.inode = get_inode(EXT2_GOOD_OLD_FIRST_INO);
    new_process->cur_dir.dir_name = (char*) malloc(2);
    new_process->cur_dir.name_size = 2;
    strcpy(new_process->cur_dir.dir_name, root_char);
    new_process->cur_dir.dir_name[1] = '\0';
    new_process->open_files_table = NULL;
    //The file descriptors from zero to 3 are reserved  
    memset(new_process->fd_bitmap, 0, SIZE_BIT_MAP);
    *new_process->fd_bitmap = 0x00;
  #endif
  //---Add process to activatable queue
  queue_add(new_process, &activatable_process_queue, process, next_prev, prio);

  debug_print("[%s] created process with pid = %d \n",
              new_process->process_name, new_process->pid);

  //------------We activate this new process if it has a higher
  //priority-----------
  // This function must be called a the very end
  check_if_new_prio_is_higher_and_call_scheduler(new_process->prio, true, 0);
  return new_process->pid;
}

int waitpid(int pid, int *retvalp) {
  debug_print_exit_m("[waitpid] Inside waitpid with pid  = %d\n", pid);
  if (get_process_struct_of_pid(getpid())->children_head == NULL &&
      get_process_struct_of_pid(getpid())->children_tail == NULL) {
    return -1;
  }
  process *temp_process = NULL;
  process *temp_process_before = NULL;
  int pid_to_return;
  // negative pid, we find the first zombie and we take its return value and
  // free it
  if (pid < 0) {
    // temp_process = get_process_struct_of_pid(getpid())->children_head;
    // temp_process_before = temp_process;
    while (true) {
      temp_process = get_process_struct_of_pid(getpid())->children_head;
      temp_process_before = temp_process;
      while (temp_process != NULL) {
        if (temp_process->state == ZOMBIE) {
          break;
        }
        temp_process_before = temp_process;
        temp_process = temp_process->next_sibling;
      }
      if (temp_process == NULL) {
        get_process_struct_of_pid(getpid())->state = BLOCKEDWAITCHILD;
        scheduler();
      } else {
        break;
      }
    }
  }
  // positive pid, we verify pid is a child and then we take its return value
  // and we kill it
  else {
    temp_process = get_process_struct_of_pid(getpid())->children_head;
    temp_process_before = temp_process;
    // We check that the pid is a child to the process that called this method
    while (temp_process != NULL) {
      if (temp_process->pid == pid) {
        break;
      }
      temp_process_before = temp_process;
      temp_process = temp_process->next_sibling;
    }
    if (temp_process == NULL) {
      // pid of the process given as function argument in not a child
      // to current process
      return -1;
    }
    while (true) {
      // We wait until the state becomes a zombie
      if (temp_process->state == ZOMBIE) {
        break;
      }
      get_process_struct_of_pid(getpid())->state = BLOCKEDWAITCHILD;
      scheduler();
    }
  }
  // We take the return value of the process and then we kill it
  pid_to_return = temp_process->pid;
  if (retvalp != NULL) {
#ifdef USER_PROCESSES_ON
    // The user mode function in 64 bits the problem is that when we have a int
    // * pointer and we modify its value only, the value of the upper of upper
    // 32 bits will not be affected thus the returned value will be different
    // from the returned value
    *((unsigned long *)retvalp) = 0;
#endif
    *retvalp = temp_process->return_value;
    debug_print_exit_m("\nretvalp address : %p \nwait pid value : %d \n",
                       retvalp, temp_process->return_value);
    debug_print_exit_m("Value written in *retvalp %x\n", *retvalp);
  }
  if (free_process_arg_and_fix_tree_link(temp_process, temp_process_before) <
      0) {
    return -1;
  }
  return pid_to_return;
}

int kill(int pid) {
  if (pid == idleId || pid == kernelId) {
    // Idle process cannot be killed
    return -1;
  }
  process *process_pid = get_process_struct_of_pid(pid);
  if (process_pid == NULL) {
    return -1;
  }
  debug_print_exit_m(
      "Kill method called current pid/name = %d/%s kill pid = %d ", getpid(),
      getname(), pid);
  if (validate_action_process_valid(process_pid) < 0) {
    debug_print_exit_m("State of the process ot kill = %d \n",
                       process_pid->state);
    return -1;
  }
  if (process_pid->pid == getpid()) {
    // if we caall the kill method on the current process
    // than it is similar to calling the exit method
    exit_process(0);
  }
  if (leave_queue_process_if_needed(process_pid) < 0) {
    return -1;
  }
  if (turn_current_process_into_a_zombie_or_kill_it(false, pid) < 0) {
    return -1;
  }
  process_pid->return_value = 0;
  return 0;
}

void show_ps_info() {
  int pid_iterator = get_pid_iterator();
  process *proc_iter = NULL;
  for (int proc_n_iter = 0; proc_n_iter <= pid_iterator; proc_n_iter++) {
    proc_iter = ((process *)hash_get(get_process_hash_table(),
                                     cast_int_to_pointer(proc_n_iter), NULL));
    if (proc_iter) {
      printf("pid = %d, name = %s, etat = ", proc_iter->pid,
             proc_iter->process_name);
      print_process_state(proc_iter->state);
      printf("\n");
    }
  }
}


void show_programs() {
  printf("\n");
  int app = 0;
  while (symbols_table[app].name != NULL) {
    printf("%s \n", symbols_table[app].name);
    app++;
  }
}

/*
** HELPER FUNCTIONS DEFINITIONS
*/

/**
 * @brief this function deletes the process from the hash table
 * and frees the data structure of the process
 * @param process_to_free the process that we will free that must be a zombie
 * @returns the value 0 if the the operation was a success and a negative value
 * otherwise
 */
static int free_child_zombie_process(process *process_to_free) {
  if (process_to_free == NULL) {
    return -1;
  }
  if (process_to_free->state != ZOMBIE) {
    return -1;
  }
  debug_print_exit_m("Trying to free with pid = %d and name = %s \n",
                     process_to_free->pid, process_to_free->process_name);
  if (process_to_free->pid == getpid()) {
    // If we exit the process from the process it self meaning exit process was
    // called or we applied the kill method on the currently running pid
    // this process memory cannot be deleted instantly
    // it will be removed by the scheduler
    process_to_free->state = KILLED;
  } else {
    debug_print_exit_m("Freeing the process with pid = %d and name = %s \n",
                       process_to_free->pid, process_to_free->process_name);
    // If we killed the process using the kill method then we can removea it
    // directly
    return free_process_memory(process_to_free);
  }
  return 0;
}

/**
 * @brief Free a process and removes it from the parent's children
 * @note This function must be called on a process that has a parent
 * @param process_to_free the process to free
 * @param before_process the siblings that comes before the process, might have
 * the same value
 * @returns the value 0 if the the operation was a success and a negative value
 * otherwise
 */
static int free_process_arg_and_fix_tree_link(process *process_to_free,
                                              process *before_process) {
  if (process_to_free->parent == NULL) {
    return -1;
  }
  if (process_to_free == NULL || before_process == NULL) {
    return -1;
  }
  if (process_to_free->parent->children_head == NULL ||
      process_to_free->parent->children_tail == NULL) {
    return -1;
  }
  if (process_to_free == NULL || before_process == NULL) {
    return -1;
  }
  // Linking ...
  if (process_to_free == process_to_free->parent->children_head &&
      process_to_free == process_to_free->parent->children_tail) {
    process_to_free->parent->children_tail = NULL;
    process_to_free->parent->children_head = NULL;
  } else if (process_to_free == process_to_free->parent->children_head) {
    process_to_free->parent->children_head =
        process_to_free->parent->children_head->next_sibling;
  } else if (process_to_free == process_to_free->parent->children_tail) {
    before_process->next_sibling = NULL;
    process_to_free->parent->children_tail = before_process;
  } else {
    before_process->next_sibling = process_to_free->next_sibling;
  }
  return free_child_zombie_process(process_to_free);
}

/**
 * @brief this method is called when we exit a process, it will make the
 * children of the process that are still alive orphans and it will free all the
 * zombie children
 * @param parent_process the process that will do apply the action on to
 * @returns the value 0 if the the operation was a success and a negative value
 * otherwise
 * @note this method will return 0 if the process does not have any children
 */
static int make_children_orphans_and_kill_zombies(process *parent_process) {
  if (parent_process == NULL) {
    return -1;
  }
  if (parent_process->children_head == NULL &&
      parent_process->children_tail == NULL) {
    return 0;
  } else {
    process *temp_process = parent_process->children_head;
    while (temp_process != NULL) {
      // We free the process in this casse
      if (temp_process->state == ZOMBIE) {
        //  We don't need to the fix the links of the elements because their
        //  relationship is not relevant after this call
        make_children_orphans_and_kill_zombies(temp_process);
        process *process_to_free = temp_process;
        temp_process = temp_process->next_sibling;
        if (free_child_zombie_process(process_to_free) < 0) {
          return -1;
        }
        continue;
      }
      temp_process = temp_process->next_sibling;
    }
  }
  return 0;
}

/**
 * @brief called when we exit a process, it will transform the currently running
 * process or a custom process into a zombie if the parent is still alive or it
 * will kill the process if the parent is dead
 * @param  current_or_custom indicates if we want to apply the function to the
 * current process or a custom process. True for current, false for custom
 * @param pid the id of the process that we will apply the action on if we
 * choose to work with a custom process
 * @returns the value 0 if the the operation was a success and a negative value
 * otherwise
 */
static int turn_current_process_into_a_zombie_or_kill_it(bool current_or_custom,
                                                         int pid) {
  process *current_process = NULL;
  if (current_or_custom == true) {
    // We apply the kill process to the currently running process
    current_process = ((process *)hash_get(
        get_process_hash_table(), cast_int_to_pointer(getpid()), NULL));
  } else {
    // We do the action on a custom process specified using the pid given in the
    // function argument
    current_process = ((process *)hash_get(get_process_hash_table(),
                                           cast_int_to_pointer(pid), NULL));
  }
  if (current_process == NULL) {
    return -1;
  }
  if (make_children_orphans_and_kill_zombies(current_process) < 0) {
    return -1;
  }
  current_process->state = ZOMBIE;
  if (current_process->parent == NULL) {
    return free_child_zombie_process(current_process);
  } else {
    // If the parent is waiting for a child we wake it and see
    // if the child that left correspand to that child that the parent
    // was waiting for.
    if (current_process->parent->state == ZOMBIE) {
      return free_child_zombie_process(current_process);
    }
    if (current_process->parent->state == BLOCKEDWAITCHILD) {
      current_process->parent->state = ACTIVATABLE;
      queue_add(current_process->parent, &activatable_process_queue, process,
                next_prev, prio);
    }
  }
  return 0;
}

static int process_name_copy(process *p, const char *name) {
  size_t size = strlen(name);
  if (size > MAX_SIZE_NAME)
    return -1;
  secmalloc(p->process_name, size);
  strcpy(p->process_name, name);
  return 0;
}
