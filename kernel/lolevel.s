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

                     bl    hilevel_handler_rst     @ invoke high-level C function
                     b     .                       @ halt

lolevel_handler_irq: sub   lr, lr, #4              @ correct return address
                     stmfd sp!, { r0-r3, ip, lr }  @ save    caller-save registers

                     bl    hilevel_handler_irq     @ invoke high-level C function

                     ldmfd sp!, { r0-r3, ip, lr }  @ restore caller-save registers
                     movs  pc, lr                  @ return from interrupt

lolevel_handler_svc: sub   lr, lr, #0              @ correct return address
                     stmfd sp!, { r0-r3, ip, lr }  @ save    caller-save registers

                     bl    hilevel_handler_svc     @ invoke high-level C function

                     ldmfd sp!, { r0-r3, ip, lr }  @ restore caller-save registers
                     movs  pc, lr                  @ return from interrupt 
