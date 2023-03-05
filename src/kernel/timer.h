#pragma once

#include "stdint.h"

# define TIC_PER 50

/*
* Prototypes
*/


void handle_mtimer_interrupt();
void set_machine_timer_interrupt(uint64_t delta_ms);
void set_supervisor_timer_interrupt(uint64_t delta_ms);
void handle_stimer_interrupt();

