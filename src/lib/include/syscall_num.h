#ifndef ___SYSCALL_NUM_H___
#define ___SYSCALL_NUM_H___

#define SYSC_chprio             0
#define SYSC_clock_settings     1
#define SYSC_cons_echo          2
#define SYSC_cons_read          3
#define SYSC_cons_write         4
#define SYSC_current_clock      5
#define SYSC_exit               6
#define SYSC_getpid             7
#define SYSC_getprio            8
#define SYSC_kill               9

//message queue
#define SYSC_pcount             10
#define SYSC_pcreate            11
#define SYSC_pdelete            12
#define SYSC_preceive           13
#define SYSC_preset             14
#define SYSC_psend              15

//process management
#define SYSC_start              16
#define SYSC_wait_clock         17
#define SYSC_waitpid            18

#define SYSC_sleep              19
#define SYSC_getname            20
#define SYSC_getstatus          21
#define SYSC_psize              22
#define SYSC_waitpid_nohang     23
#define SYSC_sleepms            24
#define SYSC_cons_chbuffer      25
#define SYSC_cons_wait          26
#define SYSC_reboot             27

//memory api
#define SYSC_shm_create         28
#define SYSC_shm_acquire        29
#define SYSC_shm_release        30

//Sempahores
#define SYSC_scount             31
#define SYSC_screate            32
#define SYSC_sdelete            33
#define SYSC_signal             34
#define SYSC_signaln            35
#define SYSC_sreset             36
#define SYSC_try_wait           37
#define SYSC_wait               38

#define SYSC_power_off          39

//Information 
#define SYSC_show_ps_info       40
#define SYSC_show_programs      41
#define SYSC_info_queue         42

//Mbr sys calls
#define SYSC_display_partions   43
#define SYSC_create_partition   44
#define SYSC_delete_partition   45
#define SYSC_reset_disk         46
//File system sys calls
#define SYSC_sync               47
#define SYSC_clear_disk_cache   48
#define SYSC_print_fs_details   49
//File system api
#define SYSC_open               50               
#define SYSC_close              51
#define SYSC_read               52
#define SYSC_write              53
#define SYSC_lseek              54
#define SYSC_unlink             55
#define SYSC_chdir              56
#define SYSC_mkdir              57
#define SYSC_getcwd             58

#define SYSC_print_dir_elements 66 
#define SYSC_fs_info            67
#define NB_SYSCALLS             68

#endif
