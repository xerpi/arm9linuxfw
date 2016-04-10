	.set ARM_MODE_IRQ, 0x12
	.set ARM_MODE_SYS, 0x1F
	.set FIQ_DISABLE,  0x40
	.set IRQ_DISABLE,  0x80

	.text

	.global _start
_start:
	@ Set IRQ stack
	msr cpsr_c, #(ARM_MODE_IRQ|IRQ_DISABLE|FIQ_DISABLE)
	ldr sp, =_irq_stack_end

	@ Set SYS mode stack
	msr cpsr_c, #(ARM_MODE_SYS|IRQ_DISABLE|FIQ_DISABLE)
	ldr sp, =_stack_end

	@ Copy interrupt vectors to 0x00000000 (ITCM)
	ldr     r0, =_interrupt_vectors_table
	ldr     r1, =0x00000000
	ldmia   r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
	stmia   r1!, {r2, r3, r4, r5, r6, r7, r8, r9}
	ldmia   r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
	stmia   r1!, {r2, r3, r4, r5, r6, r7, r8, r9}

	mov r0, #0
	mov r1, #0

	bl main

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
