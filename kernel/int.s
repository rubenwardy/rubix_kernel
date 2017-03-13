/* The following captures the interrupt vector table, plus a function
 * to copy it into place (which is called on reset): note that 
 * 
 * - for interrupts we don't handle an infinite loop is realised (to
 *   to approximate halting the processor), and
 * - we copy the table itself, *plus* the associated addresses stored
 *   as static data: this preserves the relative offset between each 
 *   ldr instruction and wherever it loads from.
 */
	
int_data:            ldr   pc, int_addr_rst        @ reset                 vector -> SVC mode
                     b     .                       @ undefined instruction vector -> UND mode
                     ldr   pc, int_addr_svc        @ supervisor call       vector -> SVC mode
                     b     .                       @ pre-fetch abort       vector -> ABT mode
                     b     .                       @      data abort       vector -> ABT mode
                     b     .                       @ reserved
                     ldr   pc, int_addr_irq        @ IRQ                   vector -> IRQ mode
                     b     .                       @ FIQ                   vector -> FIQ mode

int_addr_rst:        .word lolevel_handler_rst
int_addr_svc:        .word lolevel_handler_svc
int_addr_irq:        .word lolevel_handler_irq
	
.global int_init
	
int_init:            mov   r0, #0                  @ set destination address
                     ldr   r1, =int_data           @ set source      address = start of     data
                     ldr   r2, =int_init           @ set source      limit   = start of function

l0:                  ldr   r3, [ r1 ], #4          @ load  word, inc. source      address
                     str   r3, [ r0 ], #4          @ store word, inc. destination address

                     cmp   r1, r2                  
                     bne   l0                      @ loop if address != limit
               
                     mov   pc, lr                  @ return

/* These function enable and disable IRQ and FIQ interrupts, toggling
 * either the 6-th or 7-th bit of CPSR to 0 or 1 respectively.
 */

.global int_enable_irq
.global int_unable_irq
.global int_enable_fiq
.global int_unable_fiq
	
int_enable_irq:      mrs   r0,   cpsr              @ get USR mode CPSR
                     bic   r0, r0, #0x80           @  enable IRQ interrupts
                     msr   cpsr_c, r0              @ set USR mode CPSR
        
                     mov   pc, lr                  @ return

int_unable_irq:      mrs   r0,   cpsr              @ get USR mode CPSR
                     orr   r0, r0, #0x80           @ disable IRQ interrupts
                     msr   cpsr_c, r0              @ set USR mode CPSR
        
                     mov   pc, lr                  @ return

int_enable_fiq:      mrs   r0,   cpsr              @ get USR mode CPSR
                     bic   r0, r0, #0x40           @  enable FIQ interrupts
                     msr   cpsr_c, r0              @ set USR mode CPSR
        
                     mov   pc, lr                  @ return

int_unable_fiq:      mrs   r0,   cpsr              @ get USR mode CPSR
                     orr   r0, r0, #0x40           @ disable FIQ interrupts
                     msr   cpsr_c, r0              @ set USR mode CPSR
        
                     mov   pc, lr                  @ return
