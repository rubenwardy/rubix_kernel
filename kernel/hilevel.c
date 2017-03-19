#include "hilevel.h"
#include "scheduler.h"
#include "blockedqueue.h"
#include "inode.h"
#include "inode_pipe.h"
#include "inode_terminal.h"
#include "utils.h"

//
// PROCESS MANAGEMENT
//


pcb_t processes[MAX_PROCESSES];
pcb_t *current = NULL;
pid_t pid_count = 0;

void initProcessTable() {
	memset(&processes[0], 0, sizeof(pcb_t) * MAX_PROCESSES);
}

size_t findProcessByPID(pid_t pid) {
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

void removeProcess(pid_t pid) {
	int ptr = 0;
	for (; ptr < MAX_PROCESSES; ptr++) {
		pcb_t *process = &processes[ptr];
		if (process->pid == 0) {
			printLine("Can't remove process if it doesn't exist!");
			return;
		} else if (process->pid == pid) {
			break;
		}
	}

	printf("Found pid at ");
	printNum(ptr);
	printf("\n");

	while (ptr + 1 < MAX_PROCESSES && processes[ptr + 1].pid != 0) {
		printf("Switching ");
		printNum(ptr);
		printf(" and ");
		printNum(ptr + 1);
		printf("\n");

		memcpy(&processes[ptr], &processes[ptr + 1], sizeof(pcb_t));
		ptr++;
	}

	printf("Setting to 0: ");
	printNum(ptr);
	printf("\n");
	processes[ptr].pid = 0;

	scheduler_remove(pid);
}

size_t getNumProcesses() {
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
#define PAGE_SIZE 0x800
#define MAX_PAGES 0x0010000 / PAGE_SIZE
Page pages[MAX_PAGES];

u32 allocateStack(pid_t pid) {
	size_t ptr = 0;
	while (pages[ptr].pid != 0) {
		ptr++;
	}

	pages[ptr].pid = pid;

	printf("Allocated page ");
	printNum(ptr);
	printf(" to pid=");
	printNum(pid);
	printf("\n");

	return (u32)&tos_UserSpace - ptr * PAGE_SIZE;
}

void deallocateStack(uint32_t stack_start) {
	// TODO: house cleaning
}

inline pid_t startProcess(u8 priority, u32 cpsr, u32 pc) {
	if (pc == 0) {
		printLine("Unable to start process with null instructions");
	}

	size_t id = getNumProcesses();
	pid_t pid = ++pid_count;

	if (id == MAX_PROCESSES) {
		printLine("Unable to start process as maximum processes limit was reached");
		return 0;
	}

	printf("Starting process pid=");
	printNum(pid);
	printf(" at pcb=");
	printNum(id);
	printf("\n");

	u32 sp = allocateStack(pid);

	memset(&processes[id], 0, sizeof(pcb_t));
	processes[id].pid      = pid;
	processes[id].priority = priority;
	processes[id].stack_start = sp;
	processes[id].time_since_last_ran = 0;
	processes[id].ctx.cpsr = cpsr;
	processes[id].ctx.pc   = pc;
	processes[id].ctx.sp   = sp;

	scheduler_add(pid, priority);

	return pid;
}

inline pid_t startProcessByCtx(u8 priority, ctx_t *ctx) {
	size_t id = getNumProcesses();
	pid_t pid = ++pid_count;

	if (id == MAX_PROCESSES) {
		printLine("Unable to start process as maximum processes limit was reached");
		return 0;
	}

	printf("Starting process pid=");
	printNum(pid);
	printf(" at pcb=");
	printNum(id);
	printf("\n");

	memset(&processes[id], 0, sizeof(pcb_t));
	processes[id].pid      = pid;
	processes[id].priority = priority;
	processes[id].stack_start = 0;
	processes[id].time_since_last_ran = 0;
	memcpy(&processes[id].ctx, ctx, sizeof(ctx_t));

	scheduler_add(pid, priority);

	return pid;
}

void switchTo(ctx_t* ctx, int id)
{
	printf("=========== ");
	printNum(processes[id].pid);
	printf(" ===========\n");

	if (current) {
		memcpy(&current->ctx, ctx, sizeof(ctx_t));
	}
	memcpy(ctx, &processes[id].ctx, sizeof(ctx_t));
	current = &processes[id];
	processes[id].time_since_last_ran = 0;

	if (current->blocked != NOT_BLOCKED) {
		printLine("############### SWITCHING TO BLOCKED PROCESS ###############");
	}
}


//
// PROCESS SCHEDULER
//
void scheduler(ctx_t* ctx)
{
	pid_t  next = scheduler_getNext();
	size_t id   = findProcessByPID(next);

	if (id != SIZE_MAX) {
		scheduler_add(next, processes[id].priority);
		switchTo(ctx, id);
	}
}



// We're using staticly linked programs
extern void main_P3();
extern void main_P4();
extern void main_P5();

u32 getProgramInstAddress(const char *name) {
	if (strcmp(name, "p3") == 0) {
		return (u32)&main_P3;
	} else if (strcmp(name, "p4") == 0) {
		return (u32)&main_P4;
	} else if (strcmp(name, "p5") == 0) {
		return (u32)&main_P5;
	} else {
		printLine("Unable to find program");
		printLine(name);
		return 0;
	}
}


//
// Initialise states
//
INode *inode_stdout;
void hilevel_handler_rst(ctx_t *ctx) {
	printLine("RESET");

	initProcessTable();
	scheduler_init();
	blockedqueue_init();
	inode_init();
	inode_pipe_init();
	startProcess(PRIORITY_NORMAL, 0x50, getProgramInstAddress("p3"));
	startProcess(PRIORITY_NORMAL, 0x50, getProgramInstAddress("p4"));
	startProcess(PRIORITY_NORMAL, 0x50, getProgramInstAddress("p5"));

	inode_stdout = inode_create(1, 1);
	inode_terminal_create(inode_stdout);

	printLine(" - Setting current & ctx");

	switchTo(ctx, 0);

	printLine(" - Initialising timer and GIC");

	TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
	TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
	TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
	TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
	TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

	GICC0->PMR          = 0x000000F0; // unmask all            interrupts
	GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
	GICC0->CTLR         = 0x00000001; // enable GIC interface
	GICD0->CTLR         = 0x00000001; // enable GIC distributor

	printLine(" - Enabling irq interrupts");

	int_enable_irq();

	printLine(" - Done!");
}



//
// Handle interrupts
//
void hilevel_handler_irq(ctx_t *ctx) {
	printLine("IRQ");

	// Step 2: read	the interrupt identifier so we know the source.

	u32 id = GICC0->IAR;

	printNum(id);

	// Step 4: handle the interrupt, then clear (or reset) the source.

	if (id == GIC_SOURCE_TIMER0) {
		PL011_putc(UART0, 'T', true);
		TIMER0->Timer1IntClr = 0x01;
		scheduler(ctx);
	}

	// Step 5: write the interrupt identifier to signal we're done.

	GICC0->EOIR = id;

	printLine(" - done");
}



//
// Handle system calls
//
#define SYS_YIELD     ( 0x00 )
#define SYS_WRITE     ( 0x01 )
#define SYS_READ      ( 0x02 )
#define SYS_FORK      ( 0x03 )
#define SYS_EXIT      ( 0x04 )
#define SYS_EXEC      ( 0x05 )
#define SYS_KILL      ( 0x06 )
#define SYS_WAIT      ( 0x07 )
#define SYS_PIPE      ( 0x08 )
void hilevel_handler_svc(ctx_t *ctx, u32 id) {
	printLine("SVC");

	switch (id) {
		case SYS_YIELD: {
			printLine(" - scheduler");
			scheduler(ctx);
			break;
		}
		case SYS_WRITE: {
			printLine(" - write");

			int   fd = (int  )(ctx->gpr[0]);
			char*  x = (char*)(ctx->gpr[1]);
			int    n = (int  )(ctx->gpr[2]);

			// TODO: support file descriptors
			INode *node = inode_get(fd);
			if (node) {
				if (inode_check_perm(node, 1, 1, OPERATION_WRITE)) {
					node->write(node, x, n);
				} else {
					printLine("Operation not permitted");
				}
			} else {
				printLine("Unable to find inode to write to.");
			}

			ctx->gpr[ 0 ] = n;
			break;
		}
		case SYS_READ: {
			printLine(" - read");

			int   fd = (int  )(ctx->gpr[0]);
			char*  x = (char*)(ctx->gpr[1]);
			int    n = (int  )(ctx->gpr[2]);

			// TODO: support file descriptors
			INode *node = inode_get(fd + 1);
			if (node) {
				if (inode_check_perm(node, 1, 1, OPERATION_READ)) {
					node->read(node, x, n);
				} else {
					printLine("Operation not permitted");
				}
			} else {
				printLine("Unable to find inode to write to.");
			}

			ctx->gpr[0] = n;
			break;
		}
		case SYS_FORK:
			printLine(" - fork");

			size_t new_id = findProcessByPID(startProcessByCtx(current->priority, ctx));
			if (new_id >= 0) {
				pcb_t *new = &processes[new_id];
				ctx->gpr[0] = new->pid;

				u32 offset = current->stack_start - ctx->sp;
				new->stack_start = allocateStack(new->pid);
				new->ctx.sp = new->stack_start - offset;
				memcpy((u32*)new->ctx.sp, (u32*)ctx->sp, offset);
				new->ctx.gpr[0] = 0;
				new->parent = current->pid;
			} else {
				ctx->gpr[0] = -1;
			}

			break;
		case SYS_EXIT:
			printLine(" - exit");

			// Check for processes that are wait*()-ing for exit code
			BlockedProcess *bl = blockedqueue_popNextProcessExit(current->pid, current->parent);
			if (bl) {
				size_t id = findProcessByPID(bl->pid);
				if (id < SIZE_MAX) {
					pcb_t *proc = &processes[id];
					proc->ctx.gpr[0] = current->pid;
					*bl->ret1 = (int)ctx->gpr[0];
					proc->blocked = NOT_BLOCKED;
					scheduler_add(proc->pid, proc->priority);
				} else {
					printLine("unable to find process.");
				}
			}

			removeProcess(current->pid);
			current = 0;
			scheduler(ctx);
			break;
		case SYS_EXEC:
			printLine(" - exec");

			char *path = (char*) ctx->gpr[0];
			u32 addr = getProgramInstAddress(path);
			if (addr == 0) {
				ctx->gpr[0] = -1;
				return;
			}

			deallocateStack(current->stack_start);
			current->stack_start = allocateStack(current->pid);
			ctx->cpsr = 0x50;
			ctx->pc = addr;
			for (int i = 0; i < 13; i++) {
				ctx->gpr[i] = 0;
			}
			ctx->sp = current->stack_start;
			ctx->lr = 0;

			break;
		case SYS_WAIT:
			printLine(" - wait");

			pid_t pid   = (pid_t) ctx->gpr[0];
			int *status = (int*)  ctx->gpr[1];

			scheduler_remove(current->pid);
			current->blocked = BLOCKED_PROCESS;
			blockedqueue_addProcessExit(current->pid, pid, status);
			scheduler(ctx);

			break;

		case SYS_PIPE:
			printLine(" - pipe");

			INode *node1 = inode_create(1, 1);
			INode *node2 = inode_create(1, 1);
			if (node1 && node2) {
				inode_pipe_create(node1, node2);
				int *fd  = (int*)ctx->gpr[0];
				*(&fd[0]) = (int)node1->id;
				*(&fd[1]) = (int)node1->id;
				ctx->gpr[0] = 0;
			} else {
				ctx->gpr[0] = -1;
			}

			break;

		case SYS_KILL:
			printLine(" - kill unimplemented");
			break;
		default: {
			printLine(" - unknown/unsupported");
			printNum(id);
			break;
		}
	}

	printLine(" - done");
}
