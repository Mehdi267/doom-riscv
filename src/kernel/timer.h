#pragma once

#include "stdint.h"

//50 miliseconds
#define TIC_PER 10

/*
* Prototypes
*/
extern uint64_t time_counter;
void handle_mtimer_interrupt();
void set_machine_timer_interrupt(uint64_t delta_ms);
void set_supervisor_timer_interrupt(uint64_t delta_ms);
void handle_stimer_interrupt();
void sleep_cpu(unsigned int sec);
