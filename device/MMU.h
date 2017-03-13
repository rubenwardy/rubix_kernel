#ifndef __MMU_H
#define __MMU_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//  enable MMU
void mmu_enable();
// disable MMU
void mmu_unable();

// flush   TLB
void mmu_flush();

// configure MMU: set page table pointer #0 to x
void mmu_set_ptr0( uint32_t* x );
// configure MMU: set page table pointer #1 to x
void mmu_set_ptr1( uint32_t* x );

// configure MMU: set 2-bit permission field of domain d to x
void mmu_set_dom( int d, uint8_t x );

#endif
