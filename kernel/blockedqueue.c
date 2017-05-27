#include "blockedqueue.h"
#include "fides.h"
#include "fides_pipe.h"
#include "fides_file.h"
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
	printError("blockedqueue", "[BlockedProcess::getFree] Unable to get free slot");
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

void blockedqueue_unblockProcessesFromCondition(BlockedProcessMap func, void *meta) {
	for (int i = 0; i < MAX_PROCESSES; i++) {
		BlockedProcess *blocked = &blockedProcesses[i];

		u8 gpr0;
		if (blocked->pid > 0 && func(blocked, &gpr0, meta)) {
			printLine("blockedqueue", "[c4fc] unblocking process");
			pcb_t *pcb = processes_get(processes_findByPID(blocked->pid));
			if (pcb) {
				blocked->pid = 0;
				pcb->ctx.gpr[0] = gpr0;
				processes_unblockProcess(pcb);
			} else {
				printError("blockedqueue", "[c4fc] Program does not exist!");
			}
		}
	}
}

BlockedProcess *blockedqueue_popNextProcessExit(pid_t pid, pid_t parent) {
	BlockedProcess *ret = head_process;

	while (ret && ret->other != pid &&
			(ret->other != 0 || ret->pid != parent)) {
		ret = ret->next;
	}
	return ret;
}

bool l_check_pipe(BlockedProcess *blocked, u8 *gpr0, void *meta) {
	u32 pipe_id = *(u32*)meta;
	if (blocked->reason != BLOCKED_FILE) {
		return false;
	}

	FiDes *fides = fides_get(blocked->pid, blocked->fid);
	if (!fides) {
		printError("blockedqueue", "[c4bp] Unable to find fides which is blocked :/");
		blocked->pid = 0;
		return false;
	}

	if (!fides_pipe_is_pipe(fides)) {
		printLine("blockedqueue", "[c4bp] Is not pipe fides");
		return false;
	}

	if (fides->data != pipe_id) {
		printLine("blockedqueue", "[c4bp] not right pipe");
		return false;
	}

	size_t res = fides->read(fides, blocked->ret2, blocked->meta1);
	if (res == SIZE_MAX) {
		printError("blockedqueue", "[c4bp] THIS SHOULD NEVER HAPPEN");
		return false;
	}

	*gpr0 = res;
	return true;
}

void blockedqueue_checkForBlockedPipes(u32 pipe_id) {
	blockedqueue_unblockProcessesFromCondition(&l_check_pipe, (void*)pipe_id);
}

bool l_check_in_reads(BlockedProcess *blocked, u8 *gpr0, void *meta) {
	if (blocked->reason != BLOCKED_FILE) {
		return false;
	}

	FiDes *fides = fides_get(blocked->pid, blocked->fid);
	if (!fides) {
		printError("blockedqueue", "[c4bi] Unable to find fides which is blocked :/");
		return false;
	}

	if (!fides_terminal_is_terminal(fides)) {
		printLine("blockedqueue", "[c4bi] Is not pipe fides");
		return false;
	}

	size_t res = fides->read(fides, blocked->ret2, blocked->meta1);
	if (res == SIZE_MAX) {
		printError("blockedqueue", "[c4bi] THIS SHOULD NEVER HAPPEN");
		return false;
	}

	*gpr0 = res;
	return true;
}

void blockedqueue_checkForBlockedInReads() {
	blockedqueue_unblockProcessesFromCondition(&l_check_in_reads, NULL);
}

bool l_check_blocked_file(BlockedProcess *blocked, u8 *gpr0, void *meta) {
	u32 fid = *(u32*)meta;
	if (blocked->reason != BLOCKED_FILE) {
		return false;
	}

	FiDes *fides = fides_get(blocked->pid, blocked->fid);
	if (fides && fides->id == fid && fides_file_is_file(fides)) {
		size_t res = fides->read(fides, blocked->ret2, blocked->meta1);
		if (res == SIZE_MAX) {
			printError("blockedqueue", "[c4bf] THIS SHOULD NEVER HAPPEN");
		} else {
			*gpr0 = res;
			return true;
		}
	} else {
		printLine("blockedqueue", "[c4bf] Is not file fides, or not right file fides");
	}

	return false;
}

void blockedqueue_checkForBlockedFile(u32 fid) {
	blockedqueue_unblockProcessesFromCondition(&l_check_blocked_file, (void*)fid);
}
