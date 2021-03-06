#include "processes.h"
#include "scheduler.h"
#include "blockedqueue.h"
#include "fides.h"
#include "fides_pipe.h"
#include "fides_terminal.h"
#include "utils.h"

pcb_t processes[MAX_PROCESSES];
pcb_t *current = NULL;
pid_t pid_count = 0;

void processes_init() {
	memset(&processes[0], 0, sizeof(pcb_t) * MAX_PROCESSES);
}

pcb_t *processes_get(size_t i) {
	return &processes[i];
}

pcb_t *processes_getCurrent() {
	if (current->pid == 0) {
		current = NULL;
	}
	return current;
}

void processes_setCurrent(pcb_t *v) {
	current = v;
}

size_t processes_findByPID(pid_t pid) {
	for (int i = 0; i < MAX_PROCESSES; i++) {
		pcb_t *process = &processes[i];
		if (process->pid == 0) {
			break;
		} else if (process->pid == pid) {
			return i;
		}
	}
	return SIZE_MAX;
}

void processes_remove(pid_t pid) {
	if (current && current->pid == pid) {
		current = NULL;
	}

	size_t ptr = processes_findByPID(pid);
	if (ptr == SIZE_MAX) {
		printError("processes", "Can't remove process if it doesn't exist!");
		return;
	}

	if (pid == 1) {
		printError("processes", "Attempt to remove pid=1");
		while(1) {}
	}

	kprintmod("processes");
	kprint("Found pid at ");
	printNum(ptr);
	kprint("\n");
	kprint("Setting to 0: ");
	printNum(ptr);
	kprint("\n");
	processes[ptr].pid = 0;

	while (ptr + 1 < MAX_PROCESSES && processes[ptr + 1].pid != 0) {
		kprintmod("processes");
		kprint("Switching ");
		printNum(ptr);
		kprint(" and ");
		printNum(ptr + 1);
		kprint("\n");

		memcpy(&processes[ptr], &processes[ptr + 1], sizeof(pcb_t));
		ptr++;

		if (current && processes[ptr + 1].pid == current->pid) {
			current = &processes[ptr];
		}
	}

	scheduler_remove(pid);
	fides_dropall(pid);
}

int processes_sendKill(pid_t pid, int sig) {
	// TODO: actually send signal to program

	processes_remove(pid);

	return 1;
}

int processes_sendKillToChildren(pid_t parent_pid, int sig, pid_t pid) {
	int has_at_least_one_child = 0;

	for (int i = 0; i < MAX_PROCESSES; i++) {
		pcb_t *process = &processes[i];
		if (process->pid == 0) {
			break;
		} else if (process->parent == parent_pid && (pid == 0 || process->pid == pid)) {
			processes_sendKill(process->pid, sig);
			has_at_least_one_child = 1;
		}
	}

	return has_at_least_one_child;
}

size_t processes_getCount() {
	int ptr = 0;
	while (ptr < MAX_PROCESSES && processes[ptr].pid != 0) {
		ptr++;
	}
	return ptr;
}

typedef struct {
	pid_t pid;
} Page;


extern u32 tos_UserSpace;
#define PAGE_SIZE 4096
#define MAX_PAGES 0x0100000 / PAGE_SIZE
Page pages[MAX_PAGES];

u32 processes_allocateStack(pid_t pid) {
	size_t ptr = 0;
	while (pages[ptr].pid != 0) {
		ptr++;
		if (ptr >= MAX_PAGES) {
			printError("processes", "FATAL: no stack space remaining");
			return 0;
		}
	}

	pages[ptr].pid = pid;

	kprintmod("processes");
	kprint("Allocated page ");
	printNum(ptr);
	kprint(" to pid=");
	printNum(pid);
	kprint("\n");

	return (u32)&tos_UserSpace - ptr * PAGE_SIZE;
}

void processes_deallocateStack(uint32_t stack_start) {
	// TODO: house cleaning
}

pid_t processes_start(u8 priority, u32 cpsr, u32 pc) {
	if (pc == 0) {
		printLine("processes", "Unable to start process with null instructions");
	}

	size_t id = processes_getCount();
	pid_t pid = ++pid_count;

	if (id == MAX_PROCESSES) {
		printLine("processes", "Unable to start process as maximum processes limit was reached");
		return 0;
	}

	kprintmod("processes");
	kprint("Starting process pid=");
	printNum(pid);
	kprint(" at pcb=");
	printNum(id);
	kprint("\n");

	u32 sp = processes_allocateStack(pid);

	memset(&processes[id], 0, sizeof(pcb_t));
	processes[id].pid      = pid;
	processes[id].priority = priority;
	processes[id].stack_start = sp;
	processes[id].time_since_last_ran = 0;
	processes[id].ctx.cpsr = cpsr;
	processes[id].ctx.pc   = pc;
	processes[id].ctx.sp   = sp;

	FiDes *fd_in = fides_create(pid, 0);
	FiDes *fd_out = fides_create(pid, 1);
	FiDes *fd_err = fides_create(pid, 2);
	processes[id].fid_counter = 3;
	fides_terminal_create(fd_in, fd_out, fd_err);

	scheduler_add(pid, priority);

	return pid;
}

pid_t processes_startByCtx(u8 priority, pid_t oldpid, ctx_t *ctx) {
	size_t id = processes_getCount();
	pid_t pid = ++pid_count;

	if (id == MAX_PROCESSES) {
		printLine("processes", "Unable to start process as maximum processes limit was reached");
		return 0;
	}

	kprintmod("processes");
	kprint("Starting process pid=");
	printNum(pid);
	kprint(" at pcb=");
	printNum(id);
	kprint("\n");

	memset(&processes[id], 0, sizeof(pcb_t));
	processes[id].pid      = pid;
	processes[id].priority = priority;
	processes[id].stack_start = 0;
	processes[id].time_since_last_ran = 0;
	memcpy(&processes[id].ctx, ctx, sizeof(ctx_t));

	processes[id].fid_counter = fides_duplicate_all(oldpid, pid);

	scheduler_add(pid, priority);

	return pid;
}

void processes_switchTo(ctx_t* ctx, int id)
{
	kprint("================ ");
	printNum(processes[id].pid);
	if (processes[id].blocked != NOT_BLOCKED) {
		kprint(" (BLOCKED)");
	}
	kprint(" ================\n");

	if (current) {
		memcpy(&current->ctx, ctx, sizeof(ctx_t));
		if (processes[id].pid == current->pid) {
			printLine("processes", "Already running process, no need to switch!");
			return;
		}
	}
	memcpy(ctx, &processes[id].ctx, sizeof(ctx_t));
	current = &processes[id];
	processes[id].time_since_last_ran = 0;
}


//
// PROCESS SCHEDULER
//
int processes_runScheduler(ctx_t* ctx)
{
	pid_t  next = scheduler_getNext();
	size_t id   = processes_findByPID(next);

	if (id != SIZE_MAX) {
		scheduler_add(next, processes[id].priority);
		processes_switchTo(ctx, id);
		return 1;
	} else {
		printLine("processes", "All processes are blocked. Unable to switch");
		return 0;
	}
}

extern void processes_unblockProcess(pcb_t *pcb) {
	pcb->blocked = NOT_BLOCKED;
	scheduler_add(pcb->pid, pcb->priority);
}

extern void processes_schedulerPriorityChanged(pcb_t *proc) {
	scheduler_remove(proc->pid);
	scheduler_add(proc->pid, proc->priority);
}
