/**
 * @file virt_plic.c
 * @brief Virtual Platform-Level Interrupt Controller (PLIC) implementation
 * @version 0.1
 * @date 2023-04-11
 * @author Mehdi Frikha 
 * 
 * This file contains the implementation of the Virtual Platform-Level Interrupt Controller (PLIC),
 * which handles interrupt management for devices in the virtual environment.
 * 
 * This work is inspired by the xv6 RISC-V OS project at MIT
 * and the ENSIMAG OS project.
 * reference: https://github.com/mit-pdos/xv6-riscv
 */

#include "stdint.h"
#include "drivers/plic.h"
#include <stdint.h>

#define VIRT_PLIC_BASE              0x0c000000
#define VIRT_PLIC_ENABLE_OFFSET_M   0x2000 //Mmode
#define VIRT_PLIC_ENABLE_OFFSET_S   0x2080 //Smode
#define VIRT_PLIC_TARGET_OFFSET     0x200000
#define VIRT_PLIC_SOURCE_OFFSET     0x0

#define VIRT_PLIC_ENABLE_M          (VIRT_PLIC_BASE + VIRT_PLIC_ENABLE_OFFSET_M)
#define VIRT_PLIC_ENABLE_S          (VIRT_PLIC_BASE + VIRT_PLIC_ENABLE_OFFSET_S)
#define VIRT_PLIC_TARGET            (VIRT_PLIC_BASE + VIRT_PLIC_TARGET_OFFSET)
#define VIRT_PLIC_SOURCE            (VIRT_PLIC_BASE + VIRT_PLIC_SOURCE_OFFSET)

#define VIRT_VIRTIO_IRQ             1
#define VIRT_UART0_IRQ              10


/**
 * @brief Initializes the Virtual PLIC by configuring the interrupt sources and enabling interrupts for devices.
 */
 void virt_plic_init()
{
  //Configure the interrupt sources for the virtual PLIC.
  // Here, we enable interrupts for the UART and VirtIO device (disk).
  *((uint32_t*) (VIRT_PLIC_SOURCE + VIRT_VIRTIO_IRQ * 4)) = 1; // Source 1 priority (VirtIO interrupt)
  *((uint32_t*) (VIRT_PLIC_SOURCE + VIRT_UART0_IRQ * 4)) = 1; // Source 10 priority (UART interrupt)

  // Enable PLIC interrupts for the selected devices(s mode).
  *((uint32_t *)(VIRT_PLIC_ENABLE_M)) = 
                (1 << VIRT_VIRTIO_IRQ) 
              | (1 << VIRT_UART0_IRQ);

  *((uint32_t *)(VIRT_PLIC_ENABLE_S)) = 
                (1 << VIRT_VIRTIO_IRQ) 
              | (1 << VIRT_UART0_IRQ);


  // Set the priority threshold for interrupt handling.
  // In this case, we set the priority to 0 to ensure that all interrupts are raised.
  // The following line enables interrupts only for core 0 in machine mode.
  //*((uint32_t*) (VIRT_PLIC_TARGET)) = 0; // Machine mode priority(not needed)
  *((uint32_t*) (VIRT_PLIC_TARGET + 0x1000)) = 0; // Supervisor mode priority
}

// PLIC device structure
plic_device_t virt_plic = {
    virt_plic_init,
};
