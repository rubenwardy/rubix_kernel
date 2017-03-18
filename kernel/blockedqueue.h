#ifndef BLOCKEDQUEUE_H
#define BLOCKEDQUEUE_H
#include "hilevel.h"

struct BlockedProcess {
	pid_t pid;
	BlockedReason reason;
	pid_t other;
	int *ret1;
	struct BlockedProcess *next;
};
typedef struct BlockedProcess BlockedProcess;

extern void blockedqueue_init();
extern void blockedqueue_addProcessExit(pid_t pid, pid_t other, int *status);
extern BlockedProcess *blockedqueue_popNextProcessExit(pid_t pid);

#endif
