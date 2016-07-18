	.set ARM_MODE_IRQ, 0x12
	.set ARM_MODE_SYS, 0x1F
	.set FIQ_DISABLE,  0x40
	.set IRQ_DISABLE,  0x80

	.section .text.start

	.global _start
_start:
	@ Set IRQ stack
	msr cpsr_c, #(ARM_MODE_IRQ|IRQ_DISABLE|FIQ_DISABLE)
	ldr sp, =_irq_stack_end

	@ Set SYS mode stack
	msr cpsr_c, #(ARM_MODE_SYS|IRQ_DISABLE|FIQ_DISABLE)
	ldr sp, =_stack_end

	@ Disable caches / mpu
	mrc p15, 0, r4, c1, c0, 0  @ read control register
	bic r4, #(1 << 12)          @ - instruction cache disable
	bic r4, #(1 << 2)           @ - data cache disable
	bic r4, #(1 << 0)           @ - mpu disable
	mcr p15, 0, r4, c1, c0, 0  @ write control register

	@ Give read/write access to all the memory regions
	ldr r5, =0x33333333
	mcr p15, 0, r5, c5, c0, 2 @ write data access
	mcr p15, 0, r5, c5, c0, 3 @ write instruction access

	@ Sets MPU permissions and cache settings
	ldr r0, =0x0000001D	@ ITCM              - 00000000 32k
	ldr r1, =0x01FF801D	@ ITCM              - 01ff8000 32k
	ldr r2, =0x08000027	@ ARM9 internal mem - 08000000 1M
	ldr r3, =0x10000021	@ I/O mem           - 10000000 128k
	ldr r4, =0x10100025	@ I/O mem           - 10100000 512k
	ldr r5, =0x20000035	@ FCRAM             - 20000000 128M
	ldr r6, =0x1FF00027	@ DSP mem           - 1FF00000 1M
	ldr r7, =0x1800002D	@ VRAM              - 18000000 8M
	mov r8, #0x25
	mcr p15, 0, r0, c6, c0, 0
	mcr p15, 0, r1, c6, c1, 0
	mcr p15, 0, r2, c6, c2, 0
	mcr p15, 0, r3, c6, c3, 0
	mcr p15, 0, r4, c6, c4, 0
	mcr p15, 0, r5, c6, c5, 0
	mcr p15, 0, r6, c6, c6, 0
	mcr p15, 0, r7, c6, c7, 0
	mcr p15, 0, r8, c3, c0, 0	@ Write bufferable 0, 2, 5
	mcr p15, 0, r8, c2, c0, 0	@ Data cacheable 0, 2, 5
	mcr p15, 0, r8, c2, c0, 1	@ Inst cacheable 0, 2, 5

	@ Enable caches
	mrc p15, 0, r4, c1, c0, 0  @ read control register
	orr r4, r4, #(1<<18)       @ - itcm enable
	orr r4, r4, #(1<<12)       @ - instruction cache enable
	orr r4, r4, #(1<<2)        @ - data cache enable
	orr r4, r4, #(1<<0)        @ - mpu enable
	mcr p15, 0, r4, c1, c0, 0  @ write control register

	@ Flush caches
	mov r5, #0
	mcr p15, 0, r5, c7, c5, 0  @ flush I-cache
	mcr p15, 0, r5, c7, c6, 0  @ flush D-cache
	mcr p15, 0, r5, c7, c10, 4 @ drain write buffer

	@ Fixes mounting of SDMC
	ldr r0, =0x10000020
	mov r1, #0x340
	str r1, [r0]

	@ Copy interrupt vectors to 0x00000000 (ITCM)
	ldr     r0, =_interrupt_vectors_table
	ldr     r1, =0x00000000
	ldmia   r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
	stmia   r1!, {r2, r3, r4, r5, r6, r7, r8, r9}
	ldmia   r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
	stmia   r1!, {r2, r3, r4, r5, r6, r7, r8, r9}

	blx main

_stub_interrupt_vector_handler:
	subs pc, lr, #4

_interrupt_vectors_table:
	ldr pc, _reset_h
	ldr pc, _undefined_instruction_vector_h
	ldr pc, _software_interrupt_vector_h
	ldr pc, _prefetch_abort_vector_h
	ldr pc, _data_abort_vector_h
	ldr pc, _reserved_handler_h
	ldr pc, _interrupt_vector_h
	ldr pc, _fast_interrupt_vector_h

	_reset_h:                           .word _start
	_undefined_instruction_vector_h:    .word _stub_interrupt_vector_handler
	_software_interrupt_vector_h:       .word _stub_interrupt_vector_handler
	_prefetch_abort_vector_h:           .word _stub_interrupt_vector_handler
	_data_abort_vector_h:               .word _stub_interrupt_vector_handler
	_reserved_handler_h:                .word _stub_interrupt_vector_handler
	_interrupt_vector_h:                .word interrupt_vector
	_fast_interrupt_vector_h:           .word _stub_interrupt_vector_handler
