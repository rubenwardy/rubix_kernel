#ifndef __INT_H
#define __INT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// initialise interrupt vector table
extern void int_init();

//  enable IRQ interrupts
extern void int_enable_irq();
// disable IRQ interrupts
extern void int_unable_irq();
//  enable FIQ interrupts
extern void int_enable_fiq();
// disable FIQ interrupts
extern void int_unable_fiq();

#endif
