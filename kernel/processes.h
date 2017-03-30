#ifndef PROCESSES_H
#define PROCESSES_H
#include "hilevel.h"

/* The kernel source code is made simpler via three type definitions:
 *
 * - a type that captures a Process IDentifier (PID), which is really
 *   just an integer,
 * - a type that captures each component of an execution context (i.e.,
 *   processor state) in a compatible order wrt. the low-level handler
 *   preservation and restoration prologue and epilogue, and
 * - a type that captures a process PCB.
 */

typedef enum {
    NOT_BLOCKED,
    BLOCKED_PROCESS,
    BLOCKED_FILE
} BlockedReason;


#define MAX_PROCESSES 50
// ^ Linux is 31,000 per user

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
  pid_t parent;
  ctx_t ctx;
  u8 priority;
  u32 time_since_last_ran;
  u32 stack_start;
  BlockedReason blocked;
  u32 fid_counter;
} pcb_t;

extern void processes_init();
extern pcb_t *processes_get(size_t i);
extern pcb_t *processes_getCurrent();
extern void processes_setCurrent(pcb_t *v);
extern size_t processes_findByPID(pid_t pid);
extern void processes_remove(pid_t pid);
extern int processes_sendKill(pid_t pid, int sig);
extern int processes_sendKillToChildren(pid_t parent_pid, int sig, pid_t pid);
extern size_t processes_getCount();
extern u32 processes_allocateStack(pid_t pid);
extern void processes_deallocateStack(uint32_t stack_start);
extern pid_t processes_start(u8 priority, u32 cpsr, u32 pc);
extern pid_t processes_startByCtx(u8 priority, pid_t oldpid, ctx_t *ctx);
extern void processes_switchTo(ctx_t* ctx, int id);
extern int processes_runScheduler(ctx_t* ctx);
extern void processes_unblockProcess(pcb_t *pcb);
extern void processes_schedulerPriorityChanged(pcb_t *proc);

#endif
