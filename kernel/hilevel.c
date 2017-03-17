#include "hilevel.h"

pcb_t pcb[3];
pcb_t *current = NULL;

void scheduler( ctx_t* ctx )
{
	if (current == &pcb[ 0 ]) {
		memcpy(&pcb[ 0 ].ctx, ctx, sizeof(ctx_t));
		memcpy(ctx, &pcb[ 1 ].ctx, sizeof(ctx_t));
		current = &pcb[ 1 ];
	} else if (current == &pcb[ 1 ]) {
		memcpy(&pcb[ 1 ].ctx, ctx, sizeof(ctx_t));
		memcpy(ctx, &pcb[ 0 ].ctx, sizeof(ctx_t));
		current = &pcb[ 0 ];
	}

	return;
}

extern void     main_P3();
extern uint32_t tos_P3;
extern void     main_P4();
extern uint32_t tos_P4;
extern void     main_P5();
extern uint32_t tos_P5;

inline void addProgram(size_t id, uint32_t pid, uint32_t cpsr,
		uint32_t pc, uint32_t sp) {
	memset(&pcb[id], 0, sizeof(pcb_t));
	pcb[id].pid      = pid;
	pcb[id].ctx.cpsr = 0x50;
	pcb[id].ctx.pc   = pc;
	pcb[id].ctx.sp   = sp;
}

void printLine(const char *cs) {
	size_t ptr = 0;
	while (cs[ptr] != '\0') {
		PL011_putc(UART0, cs[ptr], true);
		ptr++;
	}
	PL011_putc(UART0, '\n', true);
}

void printNum(int num) {
	if (num == 0) {
		PL011_putc(UART0, '0', true);
	} else {
		int next = num / 10;
		if (next != 0) {
			printNum(next);
		}
		int digit = num % 10;
		PL011_putc(UART0, digit + '0', true);
	}
}



//
// Initialise states
//
void hilevel_handler_rst(ctx_t *ctx) {
	addProgram(0, 1, 0x50, (uint32_t)&main_P3, (uint32_t)&tos_P3);
	addProgram(1, 2, 0x50, (uint32_t)&main_P4, (uint32_t)&tos_P4);
	addProgram(2, 3, 0x50, (uint32_t)&main_P5, (uint32_t)&tos_P5);

	printLine(" - Setting current & ctx");

	current = &pcb[0];
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

	// printLine(" - Done!");
}



//
// Handle interrupts
//
void hilevel_handler_irq(ctx_t *ctx) {
	printLine("IRQ");

	// Step 2: read	the interrupt identifier so we know the source.

	uint32_t id = GICC0->IAR;

	printNum(id);

	// Step 4: handle the interrupt, then clear (or reset) the source.

	if (id == GIC_SOURCE_TIMER0) {
		PL011_putc(UART0, 'T', true);
		TIMER0->Timer1IntClr = 0x01;
	}

	// Step 5: write the interrupt identifier to signal we're done.

	GICC0->EOIR = id;

	printLine(" - done");
}



//
// Handle system calls
//
void hilevel_handler_svc(ctx_t *ctx, uint32_t id) {
	printLine("SVC");

	switch (id) {
		case 0x00: {
			printLine(" - scheduler");
			scheduler(ctx);
			break;
		}
		case 0x01: {
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
		case 0x02: {
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
		default: {
			printLine(" - unknown/unsupported");
			break;
		}
	}

	printLine(" - done");
}
