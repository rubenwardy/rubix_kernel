#include "blockedqueue.h"
#include "fides.h"
#include "fides_pipe.h"
#include "fides_terminal.h"
#include "utils.h"

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
	for (int ptr = 0; ptr < MAX_PROCESSES; ptr++) {
		if (blockedProcesses[ptr].pid == 0) {
			return &blockedProcesses[ptr];
		}
	}
	printError("[BlockedProcess::getFree] Unable to get free slot");
	return NULL;
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

void blockedqueue_addFileRead(pid_t pid, u32 fid, char *x, int max) {
	BlockedProcess *new  = getFree();
	new->pid    = pid;
	new->reason = BLOCKED_FILE;
	new->fid    = fid;
	new->ret2   = x;
	new->meta1  = max;
	new->next   = 0;
}

BlockedProcess *blockedqueue_popNextProcessExit(pid_t pid, pid_t parent) {
	BlockedProcess *ret = head_process;

	while (ret && ret->other != pid &&
			(ret->other != 0 || ret->pid != parent)) {
		ret = ret->next;
	}
	return ret;
}

void blockedqueue_checkForBlockedPipes(u32 pipe_id) {
	for (int i = 0; i < MAX_PROCESSES; i++) {
		BlockedProcess *blocked = &blockedProcesses[i];
		if (blocked->pid > 0 && blocked->reason == BLOCKED_FILE) {
			FiDes *fides = fides_get(blocked->pid, blocked->fid);
			if (fides) {
				if (fides_pipe_is_pipe(fides)) {
					if (fides->data == pipe_id) {
						size_t res = fides->read(fides, blocked->ret2, blocked->meta1);
						if (res == SIZE_MAX) {
							printError("[c4bp] THIS SHOULD NEVER HAPPEN");
						} else {
							printLine("[c4bp] unblocking process");
							pcb_t *pcb = processes_get(processes_findByPID(blocked->pid));
							pcb->ctx.gpr[0] = res;
							processes_unblockProcess(pcb);
						}
					} else {
						printLine("[c4bp] not right pipe");
					}
				} else {
					printLine("[c4bp] Is not pipe fides");
				}
			} else {
				printError("[c4bp] Unable to find fides which is blocked :/");
			}
		}
	}
}

void blockedqueue_checkForBlockedInReads() {
	for (int i = 0; i < MAX_PROCESSES; i++) {
		BlockedProcess *blocked = &blockedProcesses[i];
		if (blocked->pid > 0 && blocked->reason == BLOCKED_FILE) {
			FiDes *fides = fides_get(blocked->pid, blocked->fid);
			if (fides) {
				if (fides_terminal_is_terminal(fides)) {
					size_t res = fides->read(fides, blocked->ret2, blocked->meta1);
					if (res == SIZE_MAX) {
						printError("[c4bi] THIS SHOULD NEVER HAPPEN");
					} else {
						printLine("[c4bi] unblocking process");
						pcb_t *pcb = processes_get(processes_findByPID(blocked->pid));
						pcb->ctx.gpr[0] = res;
						blocked->pid = 0;
						processes_unblockProcess(pcb);
					}
				} else {
					printLine("[c4bi] Is not pipe fides");
				}
			} else {
				printError("[c4bi] Unable to find fides which is blocked :/");
			}
		}
	}
}
