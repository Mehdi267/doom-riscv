#ifndef _LIBC_RISCV_ENCODING_H_
#define _LIBC_RISCV_ENCODING_H_


#define MSTATUS_UIE         0x00000001
#define MSTATUS_SIE         0x00000002
#define MSTATUS_HIE         0x00000004
#define MSTATUS_MIE         0x00000008
#define MSTATUS_UPIE        0x00000010
#define MSTATUS_SPIE        0x00000020
#define MSTATUS_HPIE        0x00000040
#define MSTATUS_MPIE        0x00000080
#define MSTATUS_SPP         0x00000100
#define MSTATUS_HPP         0x00000600
#define MSTATUS_MPP         0x00001800
#define MSTATUS_MPP_0       0x00000800
#define MSTATUS_MPP_1       0x00001000
#define MSTATUS_FS          0x00006000
#define MSTATUS_XS          0x00018000
#define MSTATUS_MPRV        0x00020000
#define MSTATUS_SUM         0x00040000
#define MSTATUS_MXR         0x00080000
#define MSTATUS_TVM         0x00100000
#define MSTATUS_TW          0x00200000
#define MSTATUS_TSR         0x00400000
#define MSTATUS32_SD        0x80000000
#define MSTATUS64_SD        0x8000000000000000

#define MSTATUS_MPP_M       0x00001800
#define MSTATUS_MPP_S       0x00000800
#define MSTATUS_MPP_U       0x00000000

#define SSTATUS_UIE         0x00000001
#define SSTATUS_SIE         0x00000002
#define SSTATUS_UPIE        0x00000010
#define SSTATUS_SPIE        0x00000020
#define SSTATUS_SPP         0x00000100
#define SSTATUS_FS          0x00006000
#define SSTATUS_XS          0x00018000
#define SSTATUS_SUM         0x00040000
#define SSTATUS_MXR         0x00080000
#define SSTATUS32_SD        0x80000000
#define SSTATUS64_SD        0x8000000000000000

#define DCSR_XDEBUGVER      (3U<<30)
#define DCSR_NDRESET        (1<<29)
#define DCSR_FULLRESET      (1<<28)
#define DCSR_EBREAKM        (1<<15)
#define DCSR_EBREAKH        (1<<14)
#define DCSR_EBREAKS        (1<<13)
#define DCSR_EBREAKU        (1<<12)
#define DCSR_STOPCYCLE      (1<<10)
#define DCSR_STOPTIME       (1<<9)
#define DCSR_CAUSE          (7<<6)
#define DCSR_DEBUGINT       (1<<5)
#define DCSR_HALT           (1<<3)
#define DCSR_STEP           (1<<2)
#define DCSR_PRV            (3<<0)

#define DCSR_CAUSE_NONE     0
#define DCSR_CAUSE_SWBP     1
#define DCSR_CAUSE_HWBP     2
#define DCSR_CAUSE_DEBUGINT 3
#define DCSR_CAUSE_STEP     4
#define DCSR_CAUSE_HALT     5

#define MCONTROL_TYPE(xlen)    (0xfULL<<((xlen)-4))
#define MCONTROL_DMODE(xlen)   (1ULL<<((xlen)-5))
#define MCONTROL_MASKMAX(xlen) (0x3fULL<<((xlen)-11))

#define MCONTROL_SELECT     (1<<19)
#define MCONTROL_TIMING     (1<<18)
#define MCONTROL_ACTION     (0x3f<<12)
#define MCONTROL_CHAIN      (1<<11)
#define MCONTROL_MATCH      (0xf<<7)
#define MCONTROL_M          (1<<6)
#define MCONTROL_H          (1<<5)
#define MCONTROL_S          (1<<4)
#define MCONTROL_U          (1<<3)
#define MCONTROL_EXECUTE    (1<<2)
#define MCONTROL_STORE      (1<<1)
#define MCONTROL_LOAD       (1<<0)

#define MCONTROL_TYPE_NONE      0
#define MCONTROL_TYPE_MATCH     2

#define MCONTROL_ACTION_DEBUG_EXCEPTION   0
#define MCONTROL_ACTION_DEBUG_MODE        1
#define MCONTROL_ACTION_TRACE_START       2
#define MCONTROL_ACTION_TRACE_STOP        3
#define MCONTROL_ACTION_TRACE_EMIT        4

#define MCONTROL_MATCH_EQUAL     0
#define MCONTROL_MATCH_NAPOT     1
#define MCONTROL_MATCH_GE        2
#define MCONTROL_MATCH_LT        3
#define MCONTROL_MATCH_MASK_LOW  4
#define MCONTROL_MATCH_MASK_HIGH 5

#define MIP_USIP            (1 << IRQ_U_SOFT)
#define MIP_SSIP            (1 << IRQ_S_SOFT)
#define MIP_HSIP            (1 << IRQ_H_SOFT)
#define MIP_MSIP            (1 << IRQ_M_SOFT)
#define MIP_UTIP            (1 << IRQ_U_TIMER)
#define MIP_STIP            (1 << IRQ_S_TIMER)
#define MIP_HTIP            (1 << IRQ_H_TIMER)
#define MIP_MTIP            (1 << IRQ_M_TIMER)
#define MIP_UEIP            (1 << IRQ_U_EXT)
#define MIP_SEIP            (1 << IRQ_S_EXT)
#define MIP_HEIP            (1 << IRQ_H_EXT)
#define MIP_MEIP            (1 << IRQ_M_EXT)

#define MIE_MTIE            (1 << IRQ_M_TIMER)
#define SIE_STIE            (1 << IRQ_S_TIMER)
#define SIE_INST_PAGE_FAULT     (1 << 12)
#define SIE_LOAD_PAGE_FAULT     (1 << 13)
#define SIE_STORE_PAGE_FAULT    (1 << 15)
#define SIE_SEI                 (1 << IRQ_S_EXT) //external interrupts


#define SIP_SSIP MIP_SSIP
#define SIP_STIP MIP_STIP

#define PRV_U 0
#define PRV_S 1
#define PRV_H 2
#define PRV_M 3

#define PMP_R     0x01
#define PMP_W     0x02
#define PMP_X     0x04
#define PMP_A     0x18
#define PMP_L     0x80
#define PMP_SHIFT     2
#define PMPCFG_COUNT  4
#define PMPADDR_COUNT 16

#define PMP_OFF   0x00
#define PMP_TOR   0x08
#define PMP_NA4   0x10
#define PMP_NAPOT 0x18

#define IRQ_U_SOFT   0
#define IRQ_S_SOFT   1
#define IRQ_H_SOFT   2
#define IRQ_M_SOFT   3
#define IRQ_U_TIMER  4
#define IRQ_S_TIMER  5
#define IRQ_H_TIMER  6
#define IRQ_M_TIMER  7
#define IRQ_U_EXT	 8
#define IRQ_S_EXT    9
#define IRQ_H_EXT    10
#define IRQ_M_EXT    11
#define IRQ_COP      12
#define IRQ_HOST     13

#define DEFAULT_RSTVEC     0x00001000
#define EXT_IO_BASE        0x40000000
#define DRAM_BASE          0x80000000

// page table entry (PTE) fields
#define PTE_V     0x001 // Valid
#define PTE_R     0x002 // Read
#define PTE_W     0x004 // Write
#define PTE_X     0x008 // Execute
#define PTE_U     0x010 // User
#define PTE_G     0x020 // Global
#define PTE_A     0x040 // Accessed
#define PTE_D     0x080 // Dirty
#define PTE_SOFT  0x300 // Reserved for Software

#define PTE_PPN_SHIFT 10

#define PTE_TABLE(PTE) (((PTE) & (PTE_V | PTE_R | PTE_W | PTE_X)) == PTE_V)

#ifdef __riscv

#if __riscv_xlen == 64
# define MSTATUS_SD MSTATUS64_SD
# define SSTATUS_SD SSTATUS64_SD
# define RISCV_PGLEVEL_BITS 9
# define SPTBR_MODE SPTBR64_MODE
#else
# define MSTATUS_SD MSTATUS32_SD
# define SSTATUS_SD SSTATUS32_SD
# define RISCV_PGLEVEL_BITS 10
# define SPTBR_MODE SPTBR32_MODE
#endif
#define RISCV_PGSHIFT 12
#define RISCV_PGSIZE (1 << RISCV_PGSHIFT)

#endif

// my encodings

#define MAX_INT 0xFFFFFFFFFFFFFFFF

#endif  /* _LIBC_RISCV_ENCODING_H_ */
