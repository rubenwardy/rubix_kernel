#include "blockedqueue.h"

BlockedProcess blockedProcesses[MAX_PROCESSES];
BlockedProcess *head_process = 0;

BlockedProcess *getTail(BlockedProcess *head) {
	if (head == 0) {
		return 0;
	}

	while (head->next) {
		head = head->next;
	}

	return head;
}

BlockedProcess *getFree() {
	int ptr = 0;
	while (blockedProcesses[ptr].pid != 0) {
		ptr++;
	}
	return &blockedProcesses[ptr];
}

void blockedqueue_init() {
	memset(&blockedProcesses[0], 0, sizeof(BlockedProcess) * MAX_PROCESSES);
}

void blockedqueue_addProcessExit(pid_t pid, pid_t other, int *status) {
	BlockedProcess *new  = getFree();
	new->pid    = pid;
	new->reason = BLOCKED_PROCESS;
	new->other  = other;
	new->ret1   = status;
	new->next   = 0;

	BlockedProcess *head = getTail(head_process);
	if (head) {
		head->next = new;
	} else {
		head_process = new;
	}
}

BlockedProcess *blockedqueue_popNextProcessExit(pid_t pid) {
	BlockedProcess *ret = head_process;
	while (ret && ret->other != pid) {
		ret = ret->next;
	}
	return ret;
}
