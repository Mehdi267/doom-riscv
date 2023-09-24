/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef __ASM_GENERIC_POSIX_TYPES_H
#define __ASM_GENERIC_POSIX_TYPES_H

# define __BITS_PER_LONG 64
/*
 * This file is generally used by user-level software, so you need to
 * be a little careful about namespace pollution etc.
 *
 * First the types that are often defined in different ways across
 * architectures, so that you can override them.
 */

#ifndef __kernel_long_t
typedef long		__kernel_long_t;
typedef unsigned long	__kernel_ulong_t;
#endif

#ifndef __kernel_ino_t
typedef __kernel_ulong_t __kernel_ino_t;
#endif

#ifndef __kernel_mode_t
typedef unsigned int	__kernel_mode_t;
#endif

#ifndef __kernel_pid_t
typedef int		__kernel_pid_t;
#endif

#ifndef __kernel_ipc_pid_t
typedef int		__kernel_ipc_pid_t;
#endif

#ifndef __kernel_uid_t
typedef unsigned int	__kernel_uid_t;
typedef unsigned int	__kernel_gid_t;
#endif

#ifndef __kernel_suseconds_t
typedef __kernel_long_t		__kernel_suseconds_t;
#endif

#ifndef __kernel_daddr_t
typedef int		__kernel_daddr_t;
#endif

#ifndef __kernel_uid32_t
typedef unsigned int	__kernel_uid32_t;
typedef unsigned int	__kernel_gid32_t;
#endif

#ifndef __kernel_old_uid_t
typedef __kernel_uid_t	__kernel_old_uid_t;
typedef __kernel_gid_t	__kernel_old_gid_t;
#endif

#ifndef __kernel_old_dev_t
typedef unsigned int	__kernel_old_dev_t;
#endif

/*
 * Most 32 bit architectures use "unsigned int" size_t,
 * and all 64 bit architectures use "unsigned long" size_t.
 */
#ifndef __kernel_size_t
#if __BITS_PER_LONG != 64
typedef unsigned int	__kernel_size_t;
typedef int		__kernel_ssize_t;
typedef int		__kernel_ptrdiff_t;
#else
typedef __kernel_ulong_t __kernel_size_t;
typedef __kernel_long_t	__kernel_ssize_t;
typedef __kernel_long_t	__kernel_ptrdiff_t;
#endif
#endif

#ifndef __kernel_fsid_t
typedef struct {
	int	val[2];
} __kernel_fsid_t;
#endif

/*
 * anything below here should be completely generic
 */
typedef __kernel_long_t	__kernel_off_t;
typedef long long	__kernel_loff_t;
typedef __kernel_long_t	__kernel_old_time_t;
typedef __kernel_long_t	__kernel_time_t;
typedef long long __kernel_time64_t;
typedef __kernel_long_t	__kernel_clock_t;
typedef int		__kernel_timer_t;
typedef int		__kernel_clockid_t;
typedef char *		__kernel_caddr_t;
typedef unsigned short	__kernel_uid16_t;
typedef unsigned short	__kernel_gid16_t;


typedef unsigned int    __kernel_dev_t;

/* be careful with __kernel_gid_t and __kernel_uid_t
 * these are defined as 16-bit for legacy reason, but
 * the kernel uses 32-bits instead.
 *
 * 32-bit valuea are required for Android, so use
 * __kernel_uid32_t and __kernel_gid32_t
 */

typedef __kernel_clock_t     clock_t;
typedef __kernel_clockid_t   clockid_t;
typedef __kernel_dev_t       dev_t;
typedef __kernel_gid32_t     gid_t;
typedef __kernel_ino_t       ino_t;
typedef __kernel_mode_t      mode_t;
#ifndef _OFF_T_DEFINED_
#define _OFF_T_DEFINED_
typedef __kernel_off_t       off_t;
#endif
typedef __kernel_loff_t      loff_t;
typedef loff_t               off64_t;  /* GLibc-specific */

typedef __kernel_pid_t		 pid_t;

/* while POSIX wants these in <sys/types.h>, we
 * declare then in <pthread.h> instead */
#if 0
typedef  .... pthread_attr_t;
typedef  .... pthread_cond_t;
typedef  .... pthread_condattr_t;
typedef  .... pthread_key_t;
typedef  .... pthread_mutex_t;
typedef  .... pthread_once_t;
typedef  .... pthread_rwlock_t;
typedef  .... pthread_rwlock_attr_t;
typedef  .... pthread_t;
#endif

#ifndef _SIZE_T_DEFINED_
#define _SIZE_T_DEFINED_
// typedef unsigned int  size_t;
#endif

/* size_t is defined by the GCC-specific <stddef.h> */
#ifndef _SSIZE_T_DEFINED_
#define _SSIZE_T_DEFINED_
typedef long int  ssize_t;
#endif

typedef __kernel_suseconds_t  suseconds_t;
typedef __kernel_time_t       time_t;
typedef __kernel_uid32_t        uid_t;
typedef signed long           useconds_t;

typedef __kernel_daddr_t	daddr_t;
typedef __kernel_timer_t	timer_t;

typedef __kernel_caddr_t    caddr_t;
typedef unsigned int        uint_t;
typedef unsigned int        uint;

/* for some applications */

#ifdef __BSD_VISIBLE
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;

typedef uint32_t       u_int32_t;
typedef uint16_t       u_int16_t;
typedef uint8_t        u_int8_t;
typedef uint64_t       u_int64_t;
#endif


#define O_ACCMODE 00000003
#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_RDWR 00000002
#ifndef O_CREAT
#define O_CREAT 00000100  
#endif
#ifndef O_EXCL
#define O_EXCL 00000200  
#endif
#ifndef O_NOCTTY
#define O_NOCTTY 00000400  
#endif
#ifndef O_TRUNC
#define O_TRUNC 00001000  
#endif
#ifndef O_APPEND
#define O_APPEND 00002000
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK 00004000
#endif
#ifndef O_SYNC
#define O_SYNC 00010000
#endif
#ifndef FASYNC
#define FASYNC 00020000  
#endif
#ifndef O_DIRECT
#define O_DIRECT 00040000  
#endif
#ifndef O_LARGEFILE
#define O_LARGEFILE 00100000
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY 00200000  
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 00400000  
#endif
#ifndef O_NOATIME
#define O_NOATIME 01000000
#endif
#ifndef O_NDELAY
#define O_NDELAY O_NONBLOCK
#endif

#define F_DUPFD 0  
#define F_GETFD 1  
#define F_SETFD 2  
#define F_GETFL 3  
#define F_SETFL 4  
#ifndef F_GETLK
#define F_GETLK 5
#define F_SETLK 6
#define F_SETLKW 7
#endif
#ifndef F_SETOWN
#define F_SETOWN 8  
#define F_GETOWN 9  
#endif
#ifndef F_SETSIG
#define F_SETSIG 10  
#define F_GETSIG 11  
#endif

#define FD_CLOEXEC 1  

#ifndef F_RDLCK
#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2
#endif

#ifndef F_EXLCK
#define F_EXLCK 4  
#define F_SHLCK 8  
#endif

#ifndef F_INPROGRESS
#define F_INPROGRESS 16
#endif

#define LOCK_SH 1  
#define LOCK_EX 2  
#define LOCK_NB 4  
#define LOCK_UN 8  

#define LOCK_MAND 32  
#define LOCK_READ 64  
#define LOCK_WRITE 128  
#define LOCK_RW 192  

#endif
