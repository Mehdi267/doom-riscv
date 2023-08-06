#ifndef _LOGGER_H_
#define _LOGGER_H_

/**
 * @brief the following macros are used when debugging the c
 * code. Inspired from :
 * https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c
 */
#define DEBUG_LEVEL 0 //Indicates if debug type is active 

#define debug_print(fmt, ...)                                                  \
  do {                                                                         \
    if (DEBUG_LEVEL == 1) {                                                    \
      printf(fmt, __VA_ARGS__);                                                \
    }                                                                          \
    if (DEBUG_LEVEL == 2) {                                                    \
      printf("File = %s : Line = %d: Func = %s(): " fmt, __FILE__, __LINE__,   \
             __func__, __VA_ARGS__);                                           \
    }                                                                          \
  } while (0)

#define debug_print_no_arg(fmt, ...) \
        do {if (DEBUG_LEVEL){ printf(fmt);} } while (0)

/**
 * @brief the following macro are used to debug the scheduler,
 *  meaning when we debug the scheduler we use the debug_print_scheduler
 */
#define DEBUG_SCHEDULER_LEVEL 0 //Indicates if debug type is active

#define debug_print_scheduler(fmt, ...) \
        do {if (DEBUG_SCHEDULER_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_SCHEDULER_LEVEL == 2){ printf("\033[0;32mFile/Line/Func [%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

#define debug_print_scheduler_no_arg(fmt, ...) \
        do {if (DEBUG_SCHEDULER_LEVEL){ printf(fmt);} } while (0)



/**
 * @brief the following macro are used to debug the processes,
 *  meaning when we debug the scheduler we use the debug_print_process
 */
#define DEBUG_PROCESS_LEVEL 0 // Indicates if debug type is active

#define debug_print_process(fmt, ...)                                          \
  do {                                                                         \
    if (DEBUG_PROCESS_LEVEL == 1) {                                            \
      printf(fmt, __VA_ARGS__);                                                \
    }                                                                          \
    if (DEBUG_PROCESS_LEVEL == 2) {                                            \
      printf("\033[0;32mFile/Line/Func [%s][%d][%s]: \033[0;m" fmt, __FILE__, __LINE__,          \
             __func__, __VA_ARGS__);                                           \
    }                                                                          \
  } while (0)

/**
 * @brief the following macro are used to debug the processes,
 *  meaning when we debug the scheduler we use the debug_print_process
 */
#define DEBUG_EXIT_METHODS_LEVEL 0 // Indicates if debug type is active

#define debug_print_exit_m(fmt, ...)                                           \
  do {                                                                         \
    if (DEBUG_EXIT_METHODS_LEVEL == 1) {                                       \
      printf(fmt, __VA_ARGS__);                                                \
    }                                                                          \
    if (DEBUG_EXIT_METHODS_LEVEL == 2) {                                       \
      printf("\033[0;32mFile/Line/Func [%s][%d][%s]: \033[0;m" fmt, __FILE__, __LINE__,          \
             __func__, __VA_ARGS__);                                           \
    }                                                                          \
  } while (0)

/**
 * @brief the following macro are used when running the tests
 */
#define DEBUG_TESTING_LEVEL 0 // Indicates if debug type is active

#define debug_print_tests(fmt, ...)                                            \
  do {                                                                         \
    if (DEBUG_TESTING_LEVEL == 1) {                                            \
      printf(fmt, __VA_ARGS__);                                                \
    }                                                                          \
    if (DEBUG_TESTING_LEVEL == 2) {                                            \
      printf("\033[0;32mFile/Line/Func [%s][%d][%s]: \033[0;m" fmt, __FILE__, __LINE__,          \
             __func__, __VA_ARGS__);                                           \
    }                                                                          \
  } while (0)

#define print_test_no_arg(fmt, ...)                                            \
  do {                                                                         \
    if (DEBUG_TESTING_LEVEL) {                                                 \
      printf(fmt);                                                             \
    }                                                                          \
  } while (0)


/**
 * @brief the following macro are used to debug the memory management
 */
#define DEBUG_MEMORY_LEVEL 0 //Indicates if debug type is active

#define debug_print_memory(fmt, ...) \
        do {if (DEBUG_MEMORY_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_MEMORY_LEVEL == 2){ printf("\033[0;32mFile/Line/Func [%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

#define print_memory_no_arg(fmt, ...) \
        do {if (DEBUG_MEMORY_LEVEL){ printf(fmt);} } while (0)


/**
 * @brief the following macro are used to debug the memory api 
 */
#define DEBUG_MEMORY_API_LEVEL 0 //Indicates if debug type is active

#define debug_print_memory_api(fmt, ...) \
        do {if (DEBUG_MEMORY_API_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_MEMORY_API_LEVEL == 2){ printf("\033[0;32mFile/Line/Func [%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

#define print_memory_api_no_arg(fmt, ...) \
        do {if (DEBUG_MEMORY_API_LEVEL){ printf(fmt);} } while (0)

/**
 * @brief the following macro are used to debug the semaphore api 
 */
#define DEBUG_SEMAPHORE_API_LEVEL 0 //Indicates if debug type is active

#define debug_print_sem(fmt, ...) \
        do {if (DEBUG_SEMAPHORE_API_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_SEMAPHORE_API_LEVEL == 2){ printf("\033[0;32mFile/Line/Func [%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

#define print_sem_api_no_arg(fmt, ...) \
        do {if (DEBUG_SEMAPHORE_API_LEVEL){ printf(fmt);} } while (0)


/**
 * @brief the following macro are used to debug the timer for the 
 * virt qemu machine 
 */
#define DEBUG_VIRT_TIMER_LEVEL 0 //Indicates if debug type is active

#define debug_print_virt_timer(fmt, ...) \
        do {if (DEBUG_VIRT_TIMER_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_VIRT_TIMER_LEVEL == 2){ printf("\033[0;32mFile/Line/Func [%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)


#define print_virt_timer_no_arg(fmt, ...) \
        do {if (DEBUG_SEMAPHORE_API_LEVEL){ printf(fmt);} } while (0)

/**
 * @brief the following macro are used to debug the  disk driver 
 * virt qemu machine 
 */
#define DEBUG_VIRT_DISK_LEVEL 0 //Indicates if debug type is active

#define debug_print_v_disk(fmt, ...) \
    do {if (DEBUG_VIRT_DISK_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
        if (DEBUG_VIRT_DISK_LEVEL == 2){ printf("\033[0;32m[%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                            __LINE__, __func__, __VA_ARGS__);} } while (0)

#define print_v_disk_no_arg(fmt, ...) \
        do {if (DEBUG_VIRT_DISK_LEVEL){ printf(fmt);} } while (0)

/**
 * @brief the following macro are used to display the  
 * debug messages of the file system the file system 
 */
#define DEBUG_VIRT_FS_LEVEL 0 //Indicates if debug type is active

#define debug_print_v_fs(fmt, ...) \
        do {if (DEBUG_VIRT_FS_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_VIRT_FS_LEVEL == 2){ printf("\033[44m[%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

#define print_fs_no_arg(fmt, ...) \
        do {if (DEBUG_VIRT_FS_LEVEL == 1){ printf(fmt);}  \
            if (DEBUG_VIRT_FS_LEVEL == 2){ printf("\033[44m[%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__);} } while (0)

/**
 * @brief the following macro are used to display the  
 * debug messages of the file system operations 
 */ 
#define DEBUG_INODE_LEVEL 2 //Indicates if debug type is active

#define debug_print_inode(fmt, ...) \
        do {if (DEBUG_INODE_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_INODE_LEVEL == 2){ printf("\033[0;32m[%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

#define print_inode_no_arg(fmt, ...) \
        do {if (DEBUG_INODE_LEVEL == 1){ printf(fmt);}  \
            if (DEBUG_INODE_LEVEL == 2){ printf("\033[0;32m[%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__);} } while (0)


/**
 * @brief the following macro are used to display the  
 * debug messages of the file system api
 */
#define DEBUG_FS_API_LEVEL 0 //Indicates if debug type is active

#define debug_print_fsapi(fmt, ...) \
        do {if (DEBUG_FS_API_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_FS_API_LEVEL == 2){ printf("\033[0;35m[%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

#define print_fsapi_no_arg(fmt, ...) \
        do {if (DEBUG_FS_API_LEVEL == 1){ printf(fmt);}  \
            if (DEBUG_FS_API_LEVEL == 2){ printf("\033[0;35m[%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__);} } while (0)

/**
 * @brief the following macro are used to display the  
 * debug messages of the file system api
 */
#define DEBUG_DIR_API_LEVEL 0 //Indicates if debug type is active

#define debug_print_dirapi(fmt, ...) \
        do {if (DEBUG_DIR_API_LEVEL == 1){ printf(fmt, __VA_ARGS__);} \
            if (DEBUG_DIR_API_LEVEL == 2){ printf("\033[0;31m[%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__);} } while (0)

#define print_dirapi_no_arg(fmt, ...) \
        do {if (DEBUG_DIR_API_LEVEL == 1){ printf(fmt);}  \
            if (DEBUG_DIR_API_LEVEL == 2){ printf("\033[0;31m[%s][%d][%s]: \033[0;m" fmt, __FILE__, \
                                __LINE__, __func__);} } while (0)


#endif