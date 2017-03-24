#include "hilevel.h"
#include "scheduler.h"
#include "blockedqueue.h"
#include "fides.h"
#include "fides_pipe.h"
#include "fides_terminal.h"
#include "utils.h"

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
FiDes *fides_stdout;
void hilevel_handler_rst(ctx_t *ctx) {
	printLine("RESET");

	processes_init();
	scheduler_init();
	blockedqueue_init();
	fides_init();
	fides_pipe_init();
	processes_start(PRIORITY_NORMAL, 0x50, getProgramInstAddress("p3"));
	processes_start(PRIORITY_NORMAL, 0x50, getProgramInstAddress("p4"));
	processes_start(PRIORITY_NORMAL, 0x50, getProgramInstAddress("p5"));

	printLine(" - Setting current & ctx");

	processes_switchTo(ctx, 0);

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
		processes_runScheduler(ctx);
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
#define SYS_CLOSE     ( 0x09 )
#define SYS_DUP2      ( 0x10 )
void hilevel_handler_svc(ctx_t *ctx, u32 id) {
	printLine("SVC");

	pcb_t *current = processes_getCurrent();

	switch (id) {
		case SYS_YIELD: {
			printLine(" - scheduler");
			processes_runScheduler(ctx);
			break;
		}
		case SYS_WRITE: {
			printf(" - write ");

			int   fd = (int  )(ctx->gpr[0]);
			char*  x = (char*)(ctx->gpr[1]);
			int    n = (int  )(ctx->gpr[2]);
			printNum(fd);
			printf(" size ");
			printNum(n);
			printf(": ");
			char *x1 = x;
			for (int i = 0; i < n; i++) {
				PL011_putc(UART0, *x1++, true);
			}
			printf("\n");

			FiDes *node = fides_get(current->pid, fd);
			if (node) {
				if (node->write) {
					node->write(node, x, n);
				} else {
					printLine("Operation not permitted");
				}
			} else {
				printLine("Unable to find fides to write to.");
			}

			ctx->gpr[ 0 ] = n;
			break;
		}
		case SYS_READ: {
			printLine(" - read");

			int   fd = (int  )(ctx->gpr[0]);
			char*  x = (char*)(ctx->gpr[1]);
			int    n = (int  )(ctx->gpr[2]);

			FiDes *node = fides_get(current->pid, fd );
			if (node) {
				if (node->read) {
					size_t res = node->read(node, x, n);
					if (res == SIZE_MAX) {
						printLine(" - blocked");
						scheduler_remove(current->pid);
						current->blocked = BLOCKED_FILE;
						blockedqueue_addFileRead(current->pid, fd, x, n);
						processes_runScheduler(ctx);
					} else {
						ctx->gpr[0] = res;
					}
				} else {
					printLine("Operation not permitted");
					ctx->gpr[0] = 0;
				}
			} else {
				printLine("Unable to find fides to write to.");
				ctx->gpr[0] = 0;
			}
			break;
		}
		case SYS_FORK: {
			printLine(" - fork");

			size_t new_id = processes_findByPID(processes_startByCtx(current->priority, current->pid, ctx));
			if (new_id != SIZE_MAX) {
				pcb_t *new = processes_get(new_id);
				ctx->gpr[0] = new->pid;

				u32 offset = current->stack_start - ctx->sp;
				new->stack_start = processes_allocateStack(new->pid);
				new->ctx.sp = new->stack_start - offset;
				memcpy((u32*)new->ctx.sp, (u32*)ctx->sp, offset);
				new->ctx.gpr[0] = 0;
				new->parent = current->pid;
			} else {
				ctx->gpr[0] = -1;
			}

			break;
		}
		case SYS_EXIT: {
			printLine(" - exit");

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
					printLine("unable to find process.");
				}
			}

			processes_remove(current->pid);
			current = 0;
			processes_runScheduler(ctx);
			break;
		}
		case SYS_EXEC: {
			printLine(" - exec");

			char *path = (char*) ctx->gpr[0];
			u32 addr = getProgramInstAddress(path);
			if (addr == 0) {
				ctx->gpr[0] = -1;
				return;
			}

			processes_deallocateStack(current->stack_start);
			current->stack_start = processes_allocateStack(current->pid);
			ctx->cpsr = 0x50;
			ctx->pc = addr;
			for (int i = 0; i < 13; i++) {
				ctx->gpr[i] = 0;
			}
			ctx->sp = current->stack_start;
			ctx->lr = 0;

			break;
		}
		case SYS_WAIT: {
			printLine(" - wait");

			pid_t pid   = (pid_t) ctx->gpr[0];
			int *status = (int*)  ctx->gpr[1];

			scheduler_remove(current->pid);
			current->blocked = BLOCKED_PROCESS;
			blockedqueue_addProcessExit(current->pid, pid, status);
			processes_runScheduler(ctx);

			break;
		}
		case SYS_PIPE: {
			printLine(" - pipe");

			FiDes *p_out = fides_create(current->pid, current->fid_counter++);
			FiDes *p_in = fides_create(current->pid, current->fid_counter++);
			if (p_out && p_in) {
				fides_pipe_create(p_in, p_out);
				int *fd  = (int*)ctx->gpr[0];
				*(&fd[0]) = (int)p_in->id;
				*(&fd[1]) = (int)p_out->id;
				ctx->gpr[0] = 0;
			} else {
				ctx->gpr[0] = -1;
			}

			break;
		}
		case SYS_CLOSE: {
			printf(" - close ");

			int   fd = (int  )(ctx->gpr[0]);
			printNum(fd);
			printf("\n");

			if (fides_drop(current->pid, fd)) {
				ctx->gpr[0] = 0;
			} else {
				ctx->gpr[0] = -1;
			}
			break;
		}
		case SYS_DUP2: {
			printLine(" - dup2");

			int old = (int)ctx->gpr[0];
			int new = (int)ctx->gpr[1];

			if (old == new) {
				printLine("  - no need to duplicate FiDes of same ID");
			} else {
				FiDes *old_f = fides_get(current->pid, old);
				if (old_f) {
					FiDes *new_f = fides_get(current->pid, new);
					if (new_f) {
						fides_drop(current->pid, new);
					}
					fides_duplicateAlias(current->pid, old, new);

					ctx->gpr[0] = new;
				} else {
					ctx->gpr[0] = -1;
				}
			}

			break;
		}
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
