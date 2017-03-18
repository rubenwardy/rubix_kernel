#ifndef __HILEVEL_H
#define __HILEVEL_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>

// Include functionality relating to the platform.

#include   "GIC.h"
#include "PL011.h"
#include "SP804.h"

// Include functionality relating to the   kernel.

#include "lolevel.h"
#include     "int.h"

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

/* The kernel source code is made simpler via three type definitions:
 *
 * - a type that captures a Process IDentifier (PID), which is really
 *   just an integer,
 * - a type that captures each component of an execution context (i.e.,
 *   processor state) in a compatible order wrt. the low-level handler
 *   preservation and restoration prologue and epilogue, and
 * - a type that captures a process PCB.
 */


#define MAX_PROCESSES 10
// ^ Linux is 31,000 per user

typedef int pid_t;

typedef struct {
  uint32_t cpsr;    // Current program status register
  uint32_t pc;      // Program counter
  uint32_t gpr[13]; // Registers?
  uint32_t sp;      // Stack pointer
  uint32_t lr;      // Link register
} ctx_t;

// CPSR contains:
//  - the APSR flags
//  - the current processor mode
//  - interrupt disable flags
//  - current processor state (ARM, Thumb, ThumbEE, or Jazelle®)
//  - endianness state (on ARMv4T and later)
//  - execution state bits for the IT block (on ARMv6T2 and later).

#define PRIORITY_HIGHEST 3
#define PRIORITY_NORMAL  1

typedef struct {
  pid_t pid;
  ctx_t ctx;
  u8 priority;
  u32 time_since_last_ran;
  u32 stack_start;
  u32 blocked;
} pcb_t;

#endif
