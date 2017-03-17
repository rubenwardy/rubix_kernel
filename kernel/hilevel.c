#include "hilevel.h"
#include "utils.h"

// #include "heaps.h"

size_t getNumProcesses() {
	return 3;
}

pcb_t processes[3];
pcb_t *current = NULL;

void switchTo(ctx_t* ctx, int id)
{
	memcpy(&current->ctx, ctx, sizeof(ctx_t));
	memcpy(ctx, &processes[id].ctx, sizeof(ctx_t));
	current = &processes[id];
	processes[id].time_since_last_ran = 0;
}

void scheduler(ctx_t* ctx)
{
	int best_id = -1;
	int best_priority = 0;
	for (int i = 0; i < getNumProcesses(); i++) {
		pcb_t *prog = &processes[i];
		int priority = prog->priority + prog->time_since_last_ran++;
		if (priority > best_priority) {
			best_id = i;
			best_priority = priority;
		}
	}

	if (best_id >= 0) {
		switchTo(ctx, best_id);
	}
}

extern void main_P3();
extern u32  tos_P3;
extern void main_P4();
extern u32  tos_P4;
extern void main_P5();
extern u32  tos_P5;

inline void addProgram(size_t id, pid_t pid, u8 priority, u32 cpsr,
		u32 pc, u32 sp) {
	memset(&processes[id], 0, sizeof(pcb_t));
	processes[id].pid      = pid;
	processes[id].priority = priority;
	processes[id].time_since_last_ran = 0;
	processes[id].ctx.cpsr = 0x50;
	processes[id].ctx.pc   = pc;
	processes[id].ctx.sp   = sp;
}


//
// Initialise states
//
void hilevel_handler_rst(ctx_t *ctx) {
	printLine("RESET");

	addProgram(0, 1, 3, 0x50, (u32)&main_P3, (u32)&tos_P3);
	addProgram(1, 2, 1, 0x50, (u32)&main_P4, (u32)&tos_P4);
	addProgram(2, 3, 1, 0x50, (u32)&main_P5, (u32)&tos_P5);

	printLine(" - Setting current & ctx");

	current = &processes[0];
	memcpy(ctx, &current->ctx, sizeof(ctx_t));

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

			for (int i = 0; i < n; i++) {
				PL011_putc(UART0, *x++, true);
			}

			ctx->gpr[ 0 ] = n;
			break;
		}
		case SYS_READ: {
			printLine(" - read");

			int   fd = (int  )(ctx->gpr[0]);
			char*  x = (char*)(ctx->gpr[1]);
			int    n = (int  )(ctx->gpr[2]);

			for (int i = 0; i < n; i++ ) {
				*x++ = PL011_getc(UART0, true);
			}

			ctx->gpr[0] = n;
			break;
		}
		case SYS_FORK:
			printLine(" - fork unimplemented");
			break;
		case SYS_EXIT:
			printLine(" - exit unimplemented");
			break;
		case SYS_EXEC:
			printLine(" - exec unimplemented");
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
