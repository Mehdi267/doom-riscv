#include "syscall_num.h"

#define syscall(name) \
    .text                       ;\
    .globl name                 ;\
    .align 2                    ;\
    name:                       ;\
        li a7, SYSC_ ## name    ;\
        ecall                   ;\
        ret                     ;\


syscall(start)
syscall(getpid)
syscall(getprio)
syscall(chprio)
syscall(kill)
syscall(waitpid)
syscall(exit)
syscall(cons_write)
syscall(cons_read)
syscall(cons_echo)
syscall(scount)
syscall(screate)
syscall(sdelete)
syscall(signal)
syscall(signaln)
syscall(sreset)
syscall(try_wait)
syscall(wait)
syscall(pcount)
syscall(pcreate)
syscall(pdelete)
syscall(preceive)
syscall(preset)
syscall(psend)
syscall(clock_settings)
syscall(current_clock)
syscall(wait_clock)
# syscall(sys_info)
syscall(shm_create)
syscall(shm_acquire)
syscall(shm_release)
