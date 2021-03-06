#ifndef BLOCKEDQUEUE_H
#define BLOCKEDQUEUE_H
#include "hilevel.h"

struct BlockedProcess {
	pid_t pid;
	BlockedReason reason;
	pid_t other;
	int meta1;
	u32 fid;
	int *ret1;
	char *ret2;
	struct BlockedProcess *next;
};
typedef struct BlockedProcess BlockedProcess;

typedef bool (*BlockedProcessMap)(BlockedProcess *process, u8 *gpr0, void *meta);

extern void blockedqueue_init();
extern void blockedqueue_addProcessExit(pid_t pid, pid_t other, int *status);
extern void blockedqueue_addFileRead(pid_t pid, u32 fid, char *x, int max);

extern void blockedqueue_unblockProcessesFromCondition(BlockedProcessMap func, void *meta);

extern BlockedProcess *blockedqueue_popNextProcessExit(pid_t pid, pid_t parent);
extern void blockedqueue_checkForBlockedPipes(u32 pipe_id);
extern void blockedqueue_checkForBlockedInReads();
extern void blockedqueue_checkForBlockedFile(u32 fid);

#endif
