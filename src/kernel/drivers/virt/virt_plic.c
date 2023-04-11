/**
 * @file virt_plic.c
 * @author Mehdi Frikha
 * @brief 
 * @version 0.1
 * @date 2023-04-11
 * 
 * @copyright this work is inspired from the xv6 riscv os project
 * and the ensimag os project
 * 
 */
#include "stdint.h"

#include "drivers/plic.h"

#define VIRT_PLIC_BASE          0xc000000
#define VIRT_PLIC_ENABLE_OFFSET 0x2080
#define VIRT_PLIC_TARGET_OFFSET 0x200000
#define VIRT_PLIC_SOURCE_OFFSET 0x0

#define VIRT_PLIC_ENABLE        (VIRT_PLIC_BASE + VIRT_PLIC_ENABLE_OFFSET)
#define VIRT_PLIC_TARGET        (VIRT_PLIC_BASE + VIRT_PLIC_TARGET_OFFSET)
#define VIRT_PLIC_SOURCE        (VIRT_PLIC_BASE + VIRT_PLIC_SOURCE_OFFSET)

#define VIRT_VIRTIO_IRQ     1
#define VIRT_UART0_IRQ     10

static void virt_plic_init()
{
	//Enables the two sources of interrupts one for the uart and one for the virtio device 
	//which is the disk in this particular case
	*((uint32_t*) (VIRT_PLIC_SOURCE + VIRT_VIRTIO_IRQ * 0x4)) = 1; // source 1 priority (virtio interrupt)
	*((uint32_t*) (VIRT_PLIC_SOURCE + VIRT_UART0_IRQ * 0x4)) = 1; // source 10 priority (uart interrupt)
	//Enables plic interrupts for the two devices that we have chosen  
    *((uint32_t *)VIRT_PLIC_ENABLE_OFFSET) = (1 << VIRT_VIRTIO_IRQ) | (1 << VIRT_UART0_IRQ);

	// target 0 priority threshold (all interruption from source with prio > 0 will be raised
	//*((uint64_t*) VIRT_PLIC_TARGET) = 0;

	/**
	 * @brief An interrupt is raised through the plic if it has a threshold that is superior
	 * to the set priority this in this case we set the priority to 0 to make sure that 
	 * all interrupts are raised. 
	 * The following line will enable interrupts only for this core 0 
	 */
	// target 1 priority threshold (all interruption from source with prio > 0 will be raised
	*((uint32_t*) (VIRT_PLIC_TARGET)) = 0; //machine mode prio
	*((uint32_t*) (VIRT_PLIC_TARGET + 0x1000)) = 0;//supervisor mode prio
	
	// // target 0 (cpu M mode) enable -> everything
	// uint64_t* plic_addr = (uint64_t*) VIRT_PLIC_ENABLE;
	// *plic_addr = -1;
	// plic_addr++;
	// *plic_addr = -1;

	// // target 1 (cpu S mode) enable -> everything
	// plic_addr = (uint64_t*) (VIRT_PLIC_ENABLE + 0x80);
	// *plic_addr = -1;
	// plic_addr++;
	// *plic_addr = -1;
}

plic_device_t virt_plic = {
	virt_plic_init,
};
