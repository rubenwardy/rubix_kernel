#include "hilevel.h"
#include "scheduler.h"
#include "blockedqueue.h"
#include "fides.h"
#include "fides_pipe.h"
#include "fides_file.h"
#include "fides_terminal.h"
#include "fs/fs.h"
#include "utils.h"

// We're using staticly linked programs
extern void main_console();
extern void main_P3();
extern void main_P4();
extern void main_P5();
extern void main_philo();

u32 getProgramInstAddress(const char *name) {
	if (strcmp(name, "console") == 0) {
		return (u32)&main_console;
	} else if (strcmp(name, "philo") == 0) {
		return (u32)&main_philo;
	} else if (strcmp(name, "p3") == 0) {
		return (u32)&main_P3;
	} else if (strcmp(name, "p4") == 0) {
		return (u32)&main_P4;
	} else if (strcmp(name, "p5") == 0) {
		return (u32)&main_P5;
	} else {
		printError("hilevel", "Unable to find program");
		printError("hilevel", name);
		return 0;
	}
}

void postHandlerCheckForValidCurrentProcessOrWait(ctx_t *ctx) {
	printLine("hi:phc", "postHandlerCheckForValidCurrentProcessOrWait");

	if (ctx && ctx->pc < 0x1000) {
		printError("hi:phc", " - Invalid PC detected!");
	}

	pcb_t *current = processes_getCurrent();
	if (!current) {
		if (processes_runScheduler(ctx)) {
			if (ctx && ctx->pc < 0x1000) {
				printError("hi:phc", " - Invalid PC detected!");
			}
			return;
		}
	} else if (current->blocked == NOT_BLOCKED) {
		if (ctx && ctx->pc < 0x1000) {
			printError("hi:phc", " - Invalid PC detected!");
		}
		return;
	} else {
		memcpy(&current->ctx, ctx, sizeof(ctx_t));
		processes_setCurrent(NULL);
	}

	printLine("hi:phc", "entering WFI mode...");
	asm("WFI");
}


//
// Initialise states
//
void hilevel_handler_rst(ctx_t *ctx) {
	printError("hi:rst", "RESET");

	processes_init();
	scheduler_init();
	blockedqueue_init();
	fides_init();
	fides_pipe_init();
	fs_init();
	fides_file_init();

	processes_start(PRIORITY_NORMAL, 0x50, getProgramInstAddress("console"));

	printLine("hi:rst", " - Setting current & ctx");

	processes_switchTo(ctx, 0);

	printLine("hi:rst", " - Initialising timer and GIC");

	TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
	TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
	TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
	TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
	TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

	UART1->IMSC        |= 0x00000010; // enable UART    (Rx) interrupt
	UART1->CR           = 0x00000301; // enable UART (Tx+Rx)

	UART2->IMSC        |= 0x00000010; // enable UART    (Rx) interrupt
	UART2->CR           = 0x00000301; // enable UART (Tx+Rx)

	GICC0->PMR          = 0x000000F0; // unmask all            interrupts
	GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
	GICD0->ISENABLER1  |= 0x00002000; // enable UART1    (Rx) interrupt
	GICD0->ISENABLER1  |= 0x00004000; // enable UART2    (Rx) interrupt
	GICC0->CTLR         = 0x00000001; // enable GIC interface
	GICD0->CTLR         = 0x00000001; // enable GIC distributor

	printLine("hi:rst", " - Enabling irq interrupts");

	int_enable_irq();

	printLine("hi:rst", " - Done!");
}



//
// Handle interrupts
//
void hilevel_handler_irq(ctx_t *ctx) {
	printLine("hi:irq", "IRQ");

	// Step 2: read	the interrupt identifier so we know the source.

	u32 id = GICC0->IAR;

	kprintmod("hi:irq");
	printNum(id);

	// Step 4: handle the interrupt, then clear (or reset) the source.

	switch (id) {
	case GIC_SOURCE_TIMER0: {
		kprint(" timer\n");

		// HACK
		fs_disk_on_interrupt();

		TIMER0->Timer1IntClr = 0x01;
		processes_runScheduler(ctx);
		break;
	}
	case GIC_SOURCE_UART1: {
		kprint(" uart1\n");

		while (PL011_can_getc(UART1)) {
			u8 c = PL011_getc(UART1, true);
			fides_terminal_input(c);
			printLine("hilevel", "Reading char");
		}

		blockedqueue_checkForBlockedInReads();

		UART1->ICR = 0x10;
		break;
	}
	case GIC_SOURCE_UART2: {
		kprint(" uart2\n");

		fs_disk_on_interrupt();
		UART2->ICR = 0x10;
		break;
	} }

	// Step 5: write the interrupt identifier to signal we're done.

	GICC0->EOIR = id;

	printLine("hi:irq", " - done");

	postHandlerCheckForValidCurrentProcessOrWait(ctx);
}

u32 svc_handle_write(ctx_t *ctx, pcb_t *current) {
	kprintmod("hi:svc");
	kprint(" - write ");

	int   fd = (int  )(ctx->gpr[0]);
	char*  x = (char*)(ctx->gpr[1]);
	int    n = (int  )(ctx->gpr[2]);
	printNum(fd);
	kprint(" size ");
	printNum(n);
	kprint(": ");
	const char *x1 = x;
	for (int i = 0; i < n && *x1 != '\n'; i++) {
		PL011_putc(UART0, *x1++, true);
	}
	kprint("\n");

	if (n == 0 || n > 2048) {
		printError("hi:svc", "Length is 0, or too large");
		return -1;
	}

	FiDes *node = fides_get(current->pid, fd);
	if (node) {
		if (node->write) {
			node->write(node, x, n);
		} else {
			printError("hi:svc", "Operation not permitted");
		}
	} else {
		printError("hi:svc", "Unable to find fides to write to.");
	}

	return n;
}

u32 svc_handle_read(ctx_t *ctx, pcb_t *current) {
	printLine("hi:svc", " - read");

	int   fd = (int  )(ctx->gpr[0]);
	char*  x = (char*)(ctx->gpr[1]);
	int    n = (int  )(ctx->gpr[2]);

	FiDes *node = fides_get(current->pid, fd );
	if (node) {
		if (node->read) {
			size_t res = node->read(node, x, n);
			if (res == SIZE_MAX) {
				if (node->is_blocking) {
					printLine("hi:svc", " - blocked");
					scheduler_remove(current->pid);
					current->blocked = BLOCKED_FILE;
					blockedqueue_addFileRead(current->pid, fd, x, n);
					processes_runScheduler(ctx);
					return ctx->gpr[0];
				} else {
					printLine("hi:svc", " - nothing read, but non-blocking");
					return -1;
				}
			} else {
				return res;
			}
		} else {
			printError("hi:svc", "Operation not permitted");
			return 0;
		}
	} else {
		printError("hi:svc", "Unable to find fides to write to.");
		return 0;
	}
}

u32 svc_handle_fork(ctx_t *ctx, pcb_t *current) {
	printLine("hi:svc", " - fork");

	size_t new_id = processes_findByPID(processes_startByCtx(current->priority, current->pid, ctx));
	if (new_id == SIZE_MAX) {
		return -1;
	}

	pcb_t *new = processes_get(new_id);

	int offset = current->stack_start - ctx->sp;
	if (offset < 0) {
		printError("hi:svc", "Stack size < 0!");
		processes_remove(new->pid);
		return -1;
	}

	new->stack_start = processes_allocateStack(new->pid);
	new->ctx.sp = new->stack_start - offset;
	memcpy((u32*)new->ctx.sp, (u32*)ctx->sp, offset);
	new->ctx.gpr[0] = 0;

	new->parent = current->pid;

	if (offset != new->stack_start - new->ctx.sp) {
		printError("hi:svc", "Mismatching stack sizes!");
	}

	return new->pid;
}

void svc_handle_exit(ctx_t *ctx, pcb_t *current) {
	printLine("hi:svc", " - exit");

	// Check for processes that are wait*()-ing for exit code
	BlockedProcess *bl = blockedqueue_popNextProcessExit(current->pid, current->parent);
	if (bl) {
		size_t id = processes_findByPID(bl->pid);
		if (id < SIZE_MAX) {
			pcb_t *proc = processes_get(id);
			proc->ctx.gpr[0] = current->pid;
			*bl->ret1 = (int)ctx->gpr[0];
			proc->blocked = NOT_BLOCKED;
			scheduler_add(proc->pid, proc->priority);
		} else {
			printError("hi:svc", "unable to find process.");
		}
	}

	processes_remove(current->pid);
}

u32 svc_handle_exec(ctx_t *ctx, pcb_t *current) {
	printLine("hi:svc", " - exec");

	char *path = (char*) ctx->gpr[0];
	u32 addr = getProgramInstAddress(path);
	if (addr == 0) {
		return -1;
	}

	fides_dropallOnExec(current->pid);

	processes_deallocateStack(current->stack_start);
	current->stack_start = processes_allocateStack(current->pid);
	ctx->cpsr = 0x50;
	ctx->pc = addr;
	for (int i = 0; i < 13; i++) {
		ctx->gpr[i] = 0;
	}
	ctx->sp = current->stack_start;
	ctx->lr = 0;
	return 0;
}

u32 svc_handle_kill(ctx_t *ctx, pcb_t *current) {
	printLine("hi:svc", " - kill");

	int pid = (int) ctx->gpr[0];
	int sig = (int) ctx->gpr[1];

	if (pid > 0) {
		printLine("hi:svc", "   - pid exact");
		if (!processes_sendKill(pid, sig)) {
			return -1;
		} else if (current->pid == 0) { // current process was killed
			processes_runScheduler(ctx);
			return ctx->gpr[0];
		} else {
			return 0;
		}
	} else if (pid == 0 || pid == -1) {
		printLine("hi:svc", "   - pid = 0 / -1");
		// TODO: proper process groups
		if (!processes_sendKillToChildren(current->pid, sig, 0)) {
			return -1;
		}

		processes_sendKill(current->pid, sig);
		processes_runScheduler(ctx);
		return ctx->gpr[0];
	} else {
		printLine("hi:svc", "   - pid exact, pgroup");
		// TODO: proper process groups
		if (!processes_sendKillToChildren(current->pid, sig, -pid)) {
			return -1;
		} else if (current->pid == 0) { // current process was killed
			processes_runScheduler(ctx);
			return ctx->gpr[0];
		} else {
			return 0;
		}
		return -1;
	}

	return ctx->gpr[0];
}

void svc_handle_wait(ctx_t *ctx, pcb_t *current) {
	printLine("hi:svc", " - wait");

	pid_t pid   = (pid_t) ctx->gpr[0];
	int *status = (int*)  ctx->gpr[1];

	scheduler_remove(current->pid);
	current->blocked = BLOCKED_PROCESS;
	blockedqueue_addProcessExit(current->pid, pid, status);
	processes_runScheduler(ctx);
}

u32 svc_handle_pipe(ctx_t *ctx, pcb_t *current) {
	printLine("hi:svc", " - pipe");

	FiDes *p_out = fides_create(current->pid, current->fid_counter++);
	FiDes *p_in = fides_create(current->pid, current->fid_counter++);
	if (p_out && p_in) {
		fides_pipe_create(p_in, p_out);
		int *fd  = (int*)ctx->gpr[0];
		*(&fd[0]) = (int)p_in->id;
		*(&fd[1]) = (int)p_out->id;

		return 0;
	} else {
		printError("hi:svc", "Unable to create pipe");
		return -1;
	}
}

u32 svc_handle_close(ctx_t *ctx, pcb_t *current) {
	printLine("hi:svc", " - close ");

	int   fd = (int  )(ctx->gpr[0]);
	printNum(fd);
	kprint("\n");

	if (fides_drop(current->pid, fd)) {
		return 0;
	} else {
		printError("hi:svc", "Unable to close file descriptor");
		return -1;
	}
}

u32 svc_handle_dup2(ctx_t *ctx, pcb_t *current) {
	printLine("hi:svc", " - dup2");

	int old = (int)ctx->gpr[0];
	int new = (int)ctx->gpr[1];

	if (old == new) {
		printLine("hi:svc", "  - no need to duplicate FiDes of same ID");
		return new;
	}

	FiDes *old_f = fides_get(current->pid, old);
	if (!old_f) {
		return -1;
	}

	FiDes *new_f = fides_get(current->pid, new);
	if (new_f) {
		fides_drop(current->pid, new);
	}
	fides_duplicateAlias(current->pid, old, new);

	return new;
}

u32 svc_handle_fd_setblock(ctx_t *ctx, pcb_t *current) {
	printLine("hi:svc", " - fd_setblock");

	int  fd          = (int)(ctx->gpr[0]);
	bool is_blocking = (int)(ctx->gpr[1]);

	printNum(fd);
	kprint("\n");

	FiDes *fides = fides_get(current->pid, fd);
	if (fides) {
		fides->is_blocking = is_blocking;
		return 0;
	} else {
		return -1;
	}
}

u32 svc_handle_setpriority(ctx_t *ctx, pcb_t *current) {
	kprintmod("hi:svc");
	kprint(" - setpriority ");

	int who  = (int)(ctx->gpr[0]);
	int prio = (int)(ctx->gpr[1]);

	printNum(who);
	kprint("\n");

	size_t id = processes_findByPID(who);
	if (id == SIZE_MAX) {
		printError("hilevel", "Unable to find process to set priority of");
		return -1;
	}

	pcb_t* proc = processes_get(id);
	if (!proc) {
		printError("hilevel", "Unable to get process by id");
		return -1;
	}

	if (proc->parent != current->pid) {
		printError("hilevel", "Process attempted to change priority of non-child!");
		return -1;
	}

	proc->priority = prio;
	processes_schedulerPriorityChanged(proc);
	return 0;
}

u32 svc_handle_fopen(ctx_t *ctx, pcb_t *current) {
	kprintmod("hi:svc");
	kprint(" - fopen ");

	char *path = (char*)(ctx->gpr[0]);
	char  mode = (char )(ctx->gpr[1]);

	kprint(path);
	kprint(" mode=");
	PL011_putc(UART0, mode, true);
	kprint("\n");

	FiDes *fides = fides_create(current->pid, current->fid_counter);
	if (fides) {
		fides_file_create(fides, path, mode);
		return fides->id;
	}

	return -1;
}

//
// Handle system calls
//
#define SYS_YIELD       ( 0x00 )
#define SYS_WRITE       ( 0x01 )
#define SYS_READ        ( 0x02 )
#define SYS_FORK        ( 0x03 )
#define SYS_EXIT        ( 0x04 )
#define SYS_EXEC        ( 0x05 )
#define SYS_KILL        ( 0x06 )
#define SYS_WAIT        ( 0x07 )
#define SYS_PIPE        ( 0x08 )
#define SYS_CLOSE       ( 0x09 )
#define SYS_DUP2        ( 0x10 )
#define SYS_FD_SETBLOCK ( 0x11 )
#define SYS_SETPRIORITY ( 0x12 )
#define SYS_FOPEN       ( 0x13 )
void hilevel_handler_svc(ctx_t *ctx, u32 id) {
	printLine("hi:svc", "SVC");

	pcb_t *current = processes_getCurrent();

	kprintmod("hi:svc");
	kprint("Stack size is: ");
	printNum(current->stack_start - ctx->sp);
	kprint("\n");
	kprintmod("hi:svc");
	kprint("Start: ");
	printNum(current->stack_start);
	kprint("\n");
	kprintmod("hi:svc");
	kprint("Current: ");
	printNum(ctx->sp);
	kprint("\n");
	if (ctx->sp > current->stack_start) {
		printError("hi:svc", "Stack pointer out of bounds!");
	}

	switch (id) {
	case SYS_YIELD:
		printLine("hi:svc", " - scheduler");
		processes_runScheduler(ctx);
		break;
	case SYS_WRITE:
		ctx->gpr[0] = svc_handle_write(ctx, current);
		break;
	case SYS_READ:
		ctx->gpr[0] = svc_handle_read(ctx, current);
		break;
	case SYS_FORK:
		ctx->gpr[0] = svc_handle_fork(ctx, current);
		break;
	case SYS_EXIT:
		svc_handle_exit(ctx, current);
		break;
	case SYS_EXEC:
		ctx->gpr[0] = svc_handle_exec(ctx, current);
		break;
	case SYS_WAIT:
		svc_handle_wait(ctx, current);
		break;
	case SYS_PIPE:
		ctx->gpr[0] = svc_handle_pipe(ctx, current);
		break;
	case SYS_CLOSE:
		ctx->gpr[0] = svc_handle_close(ctx, current);
		break;
	case SYS_DUP2:
		ctx->gpr[0] = svc_handle_dup2(ctx, current);
		break;
	case SYS_FD_SETBLOCK:
		ctx->gpr[0] = svc_handle_fd_setblock(ctx, current);
		break;
	case SYS_SETPRIORITY:
		ctx->gpr[0] = svc_handle_setpriority(ctx, current);
		break;
	case SYS_FOPEN:
		ctx->gpr[0] = svc_handle_fopen(ctx, current);
		break;
	case SYS_KILL:
		ctx->gpr[0] = svc_handle_kill(ctx, current);
		break;
	default:
		printError("hi:svc", " - unknown/unsupported");
		printNum(id);
		break;
	}

	printLine("hi:svc", " - done");

	postHandlerCheckForValidCurrentProcessOrWait(ctx);
}
