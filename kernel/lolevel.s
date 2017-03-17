/* Each of the following is a low-level interrupt handler: each one is
 * tasked with handling a different interrupt type, and acts as a sort
 * of wrapper around a high-level, C-based handler.
 */

.global lolevel_handler_rst
.global lolevel_handler_irq
.global lolevel_handler_svc

lolevel_handler_rst: bl    int_init                @ initialise interrupt vector table

                     msr   cpsr, #0xD2             @ enter IRQ mode with IRQ and FIQ interrupts disabled
                     ldr   sp, =tos_irq            @ initialise IRQ mode stack

                     msr   cpsr, #0xD3             @ enter SVC mode with IRQ and FIQ interrupts disabled
                     ldr   sp, =tos_svc            @ initialise SVC mode stack

                     sub   sp, sp, #68             @ initialise dummy context

                     mov   r0, sp                  @ set argument
                     bl    hilevel_handler_rst     @ invoke high-level C function

                     ldmia sp!, { r0, lr }         @ load   USR mode PC and CPSR
                     msr   spsr, r0                @ set    USR mode        CPSR
                     ldmia sp, { r0-r12, sp, lr }^ @ load   USR mode registers
                     add   sp, sp, #60             @ update SVC mode SP
                     movs  pc, lr                  @ return from interrupt

lolevel_handler_irq:  sub   lr, lr, #4              @ correct return address
                     sub   sp, sp, #60             @ update SVC mode stack
                     stmia sp, { r0-r12, sp, lr }^ @ store  USR registers
                     mrs   r0, spsr                @ get    USR        CPSR
                     stmdb sp!, { r0, lr }         @ store  USR PC and CPSR
                     @stmfd sp!, { r0-r3, ip, lr }  @ save    caller-save registers

                     mov   r0, sp                  @ set    high-level C function arg. = SP
                     ldr   r1, [ lr, #-4 ]         @ load svc instruction
                     bic   r1, r1, #0xFF000000     @ set    high-level C function arg. = svc immediate
                     bl    hilevel_handler_irq     @ invoke high-level C function

                     @ldmfd sp!, { r0-r3, ip, lr }  @ restore caller-save registers
                     @movs  pc, lr                  @ return from interrupt
                     ldmia sp!, { r0, lr }         @ load   USR mode PC and CPSR
                     msr   spsr, r0                @ set    USR mode        CPSR
                     ldmia sp, { r0-r12, sp, lr }^ @ load   USR mode registers
                     add   sp, sp, #60             @ update SVC mode SP
                     movs  pc, lr                  @ return from interrupt

lolevel_handler_svc:
                     sub   lr, lr, #0              @ correct return address
                     sub   sp, sp, #60             @ update SVC mode stack
                     stmia sp, { r0-r12, sp, lr }^ @ store  USR registers
                     mrs   r0, spsr                @ get    USR        CPSR
                     stmdb sp!, { r0, lr }         @ store  USR PC and CPSR
                     @stmfd sp!, { r0-r3, ip, lr }  @ save    caller-save registers

                     mov   r0, sp                  @ set    high-level C function arg. = SP
                     ldr   r1, [ lr, #-4 ]         @ load svc instruction
                     bic   r1, r1, #0xFF000000     @ set    high-level C function arg. = svc immediate
                     bl    hilevel_handler_svc     @ invoke high-level C function

                     @ldmfd sp!, { r0-r3, ip, lr }  @ restore caller-save registers
                     @movs  pc, lr                  @ return from interrupt
                     ldmia sp!, { r0, lr }         @ load   USR mode PC and CPSR
                     msr   spsr, r0                @ set    USR mode        CPSR
                     ldmia sp, { r0-r12, sp, lr }^ @ load   USR mode registers
                     add   sp, sp, #60             @ update SVC mode SP
                     movs  pc, lr                  @ return from interrupt
