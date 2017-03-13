@ Section B3.17 of
@
@ http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0406c/index.html
@
@ gives a (fairly) concise overview of co-processor 15, which is used
@ to control the MMU.  It takes some effort to decipher, but, as an
@ example, Figure B3-29 says that if we use an mcr instruction
@
@ id = p15, opc1 = 0, CRn = c2, CRm = c0, opc2 = 0
@
@ then we are writing into the TTBR0 register, i.e., the first page
@ table pointer register.  Clearly doing so requires very low-level
@ attention to detail that could and perhaps should be abstracted
@ via a higher-level API: the following functions do that, albeit
@ for an *extremely* limited sub-set of functionality wrt. the MMU.
	
.global mmu_enable
.global mmu_unable

.global mmu_flush

.global mmu_set_ptr0
.global mmu_set_ptr1
	
.global mmu_set_dom

mmu_enable:          mrc   p15, 0, r0, c1, c0, 0 @ read  SCTLR
                     orr   r0, r0, #0x1          @ set   SCTLR[ M ] = 1 => MMU  enable
                     mcr   p15, 0, r0, c1, c0, 0 @ write SCTLR

                     mov   pc, lr                @ return

mmu_unable:          mrc   p15, 0, r0, c1, c0, 0 @ read  SCTLR
                     bic   r0, r0, #0x1          @ set   SCTLR[ M ] = 0 => MMU disable
                     mcr   p15, 0, r0, c1, c0, 0 @ write SCTLR

                     mov   pc, lr                @ return

mmu_flush:           mov   r0,     #0x0
                     mcr   p15, 0, r0, c8, c7, 0 @ write TLBIALL

                     mov   pc, lr                @ return

mmu_set_ptr0:        mcr   p15, 0, r0, c2, c0, 0 @ write TTBR0

                     mov   pc, lr                @ return

mmu_set_ptr1:        mcr   p15, 0, r0, c2, c0, 1 @ write TTBR1

                     mov   pc, lr                @ return

mmu_set_dom:         add   r0, r0, r0            @ compute i (index      from domain)
	             mov   r1, r1, lsl r0        @ compute j (permission from domain)
                     mov   r2, #0x3      
                     mov   r2, r2, lsl r0        @ compute m (mask       from domain)

                     mrc   p15, 0, r0, c3, c0, 0 @ read  DACR
                     bic   r0, r0, r2            @ mask  DACR &= ~m => DACR_{i+1,i} = 0
                     orr   r0, r0, r1            @ set                 DACR_{i+1,i} = j
                     mcr   p15, 0, r0, c3, c0, 0 @ write DACR

                     mov   pc, lr                @ return

