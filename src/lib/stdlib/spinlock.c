// void spinlock_lock(int* lock_address) {
//     __asm__ __volatile__(
//         "1:\n\t"
//         "lr.w a1, (%1)\n\t"            // Load the value at lock_address into a1
//         "bnez a1, 1b\n\t"              // If the value is non-zero, retry
//         "amoswap.w a0, %0, a1\n\t"     // Atomically swap the value at lock_address with a0
//         "bnez a0, 1b\n\t"              // If the swap result is non-zero, retry
//     : "=m" (*lock_address)             // Output operand - the memory pointed by lock_address
//     : "r" (lock_address)               // Input operand - lock_address
//     : "a0", "a1", "memory"             // Clobbered registers - a0, a1, and memory
//     );
// }

// // Inline assembly for spinlock_unlock function
// void spinlock_unlock(int* lock_address) {
//     __asm__ __volatile__(
//         "amoswap.w a0, %0, zero\n\t"    // Atomically swap the value at lock_address with zero
//     : "=r" (lock_address)               // Output operand - lock_address
//     : "0" (lock_address)                // Input operand - lock_address
//     : "a0"                              // Clobbered register - a0
//     );
// }
