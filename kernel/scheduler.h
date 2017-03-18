#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "hilevel.h"

extern void scheduler_init();
extern pid_t scheduler_getNext();
extern void scheduler_add(pid_t pid, u32 priority);
extern bool scheduler_remove(pid_t pid);

#endif
