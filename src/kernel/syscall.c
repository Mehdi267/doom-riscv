/*
 * Projet PCSEA RISC-V
 */
#include "assert.h"
#include "riscv.h"
#include "syscall_num.h"
#include "traps/trap.h"
#include "process/process.h"
#include "sync/timer_api.h"
#include "sync/semaphore_api.h"
#include "sync/time_syscall.h"
#include "process/memory_api.h"
#include "process/scheduler.h"
#include "sync/msgqueue.h"
#include "input-output/cons_write.h"
#include "input-output/keyboard.h"
#include "fs/fs.h"
#include "fs/fs_api.h"
#include "fs/dir_api.h"
#include "drivers/gpu_device.h" //gpu related methods
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "userspace_apps.h"

//This syscall will be used as a bridge between 
//kernel and user mode for testing purposes
//what is inside this function will change depending on what we are 
//currently testing
void void_call(){
  process* proc = get_current_process();
  proc->input_type = RAW_INPUT;
}

//This function takes a trap frame that contains inforamtion related to
//syscall taht was used and the parameters provided to the syscall and 
//call the method that was used and finally returns the value of the syscall 
//in the a0 register. 
unsigned long syscall_handler(struct trap_frame *tf) {
  uint64_t x;
  switch (tf->a7) {
    case SYSC_start:
      return start_virtual((const char*) tf->a0, (unsigned long) tf->a1,(int) tf->a2, (void *)tf->a3);
    case SYSC_getpid:
      return getpid();
    case SYSC_getprio:
      return getprio(tf->a0);
    case SYSC_chprio:
      return  chprio(tf->a0, tf->a1);
    case SYSC_kill:
      return kill(tf->a0);
    case SYSC_waitpid:
      x = waitpid(tf->a0,(int*) tf->a1);
      return x;
    case SYSC_exit:
      exit_process(tf->a0);
      return 0; 
    case SYSC_cons_write:
      return cons_write((char*) tf->a0, (unsigned long) tf->a1);
    case SYSC_cons_read:
      return cons_read((char*) tf->a0, (unsigned long) tf->a1);
    case SYSC_cons_echo:
      cons_echo((int)tf->a0);
      return 0;
    case SYSC_pcount:
      return pcount(tf->a0, (int*) tf->a1);
    case SYSC_pcreate:
      return pcreate(tf->a0);
    case SYSC_pdelete:
      return pdelete(tf->a0);
    case SYSC_preceive:
      return preceive(tf->a0, (int *)tf->a1);
    case SYSC_preset:
      return preset(tf->a0);
    case SYSC_psend:
      return psend(tf->a0, tf->a1);
    case SYSC_clock_settings:
      clock_settings((unsigned long *)tf->a0, (unsigned long *) tf->a1);
      break;
    case SYSC_sleep:
      sleep(tf->a0);
      break;
    case SYSC_wait_clock:
      wait_clock(tf->a0);
      break;
    case SYSC_current_clock:
      return current_clock();
    case SYSC_scount:
      return scount(tf->a0);             
    case SYSC_screate:
      return screate(tf->a0);            
    case SYSC_sdelete:
      return sdelete(tf->a0);            
    case SYSC_signal:
      return signal(tf->a0);             
    case SYSC_signaln:
      return signaln(tf->a0, tf->a1);           
    case SYSC_sreset:
      return sreset(tf->a0, tf->a1);             
    case SYSC_try_wait:
      return try_wait(tf->a0);           
    case SYSC_wait:
      return wait(tf->a0);      
    case SYSC_shm_create:
      return (unsigned long) shm_create(0, (const char*) tf->a0);
    case SYSC_shm_acquire:
      return (unsigned long) shm_acquire(0, (const char*) tf->a0);
    case SYSC_shm_release:
      shm_release((const char*) tf->a0);
      break; 
    case SYSC_power_off:
      exit(tf->a0);
      break;
    case SYSC_show_ps_info:
      show_ps_info();
      break;
    case SYSC_show_programs:
      show_programs();
      break;
    case SYSC_info_queue:
      info_msgqueues();
      break;
    case SYSC_display_partions:
      print_partition_status();
      break;  
    case SYSC_create_partition:
      create_partition(tf->a0, tf->a1, tf->a2);
      break;  
    case SYSC_delete_partition:
      delete_partition(tf->a0);
      break;    
    case SYSC_reset_disk:
      return set_up_mbr();
      break; 
    case SYSC_sync:
      return sync_all();
      break;    
    case SYSC_clear_disk_cache:
      return free_cache_list();
      break;    
    case SYSC_print_fs_details:
      print_fs_details();
      break;
    case SYSC_open:
      return open(0, (const char *)tf->a0, tf->a1, tf->a2);
    case SYSC_close:
      return close(tf->a0);
    case SYSC_read:
      return read(tf->a0, (void*)tf->a1, (uint64_t) tf->a2);
    case SYSC_write:
      return write(tf->a0, (void*)tf->a1, (uint64_t) tf->a2);
    case SYSC_lseek:
      return lseek(tf->a0, tf->a1, tf->a2);
    case SYSC_unlink:
      return unlink((const char *)tf->a0);
    case SYSC_getcwd:
      return ((unsigned long) getcwd((char *)tf->a0, tf->a1));
    case SYSC_mkdir:
      return mkdir((const char *)tf->a0, tf->a1);
    case SYSC_chdir:
      return chdir((const char *)tf->a0);
    case SYSC_rmdir:
      return rmdir((const char *)tf->a0);
    case SYSC_link:
      return sys_link((const char *)tf->a0, (const char *)tf->a1);
    case SYSC_dup:
      return dup(0, tf->a0);
    case SYSC_dup2:
      return dup2(0, tf->a0, tf->a1);
    case SYSC_pipe:
      return sys_pipe((int*)tf->a0);
    case SYSC_fork:
      return fork(getpid(), tf);
    case SYSC_execve:
      return execve((const char*)tf->a0, (char **const) tf->a1, (char **const) tf->a2);
    case SYSC_access:
      return access((const char *)tf->a0, tf->a1);
    case SYSC_fstat:
      return fstat(tf->a0, (struct stat *)tf->a1);
    case SYSC_print_dir_elements:
      print_dir_elements((const char*)tf->a0);    
      break;
    case SYSC_fs_info:
      fs_info((disk_info*)tf->a0);
      break;
    case SYSC_set_in_mode:
      set_input_mode(tf->a0, tf->a1);
      break;
    case SYSC_void_call:
      void_call();
      break;
    case SYSC_ld_progs_into_disk:
      write_user_apps_fs();
      break;
    case SYSC_sbrk:
      return (unsigned long) sys_sbrk(tf->a0);
    case SYSC_getdents:
      return (unsigned long) getdents(tf->a0, (struct dirent *)tf->a1, tf->a2);
    case SYSC_write_file_disk:
      return (unsigned long) write_file_disk((const char*) tf->a0);
    case SYSC_get_display_info:
      if (gpu_dev->get_display_info != NULL){
        gpu_dev->get_display_info((struct display_info*) tf->a0);
        break;
      }
    case SYSC_upd_data_display:
      if (gpu_dev->update_data != NULL){
        gpu_dev->update_data((void*) tf->a0, tf->a1, tf->a2, tf->a3, tf->a4);
        break;
      }
    case SYSC_time:
      return time((time_t *)tf->a0);      
    case SYSC_gettimeofday:
      return gettimeofday((struct timeval *)tf->a0, (struct timezone *)tf->a1);      
    case SYSC_settimeofday:
      return settimeofday((const struct timeval *)tf->a0, (const struct timezone *)tf->a1);      
    default:
      printf("Syscall code does not match any of the defined syscalls, num = %ld\n", tf->a7);
      blue_screen(tf);
      break;
  }
  return 0;
}