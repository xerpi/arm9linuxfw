typedef volatile unsigned int vu32;

#define REG_IRQ_IE	(*(vu32 *)0x10001000)
#define REG_IRQ_IF	(*(vu32 *)0x10001004)

#define PXI_SYNC9	(*(vu32 *)0x10008000)
#define PXI_CNT9	(*(vu32 *)0x10008004)
#define PXI_SEND9	(*(vu32 *)0x10008008)
#define PXI_RECV9	(*(vu32 *)0x1000800C)

#define IRQ_PXI_SYNC		(1 << 12)
#define IRQ_PXI_NOT_FULL	(1 << 13)
#define IRQ_PXI_NOT_EMPTY	(1 << 14)

static void arm9_enable_irq()
{
	__asm__ volatile(
		"mrs r0, cpsr\n\t"
		"bic r0, r0, #0x80\n\t"
		"msr cpsr_c, r0\n\t"
		: : : "r0");
}

static void arm9_disable_irq()
{
	__asm__ volatile(
		"mrs r0, cpsr\n\t"
		"orr r0, r0, #0x80\n\t"
		"msr cpsr_c, r0\n\t"
		: : : "r0");
}

static void arm9_set_regular_exception_vectors()
{
	__asm__ volatile(
		"mrc p15, 0, r0, c1, c0, 0\n\t"
		"bic r0, r0, #(1 << 13)\n\t"
		"mcr p15, 0, r0, c1, c0, 0\n\t"
		: : : "r0");
}

void __attribute__((interrupt("IRQ"))) interrupt_vector(void)
{
	register vu32 irq_if = REG_IRQ_IF;

	if (irq_if & IRQ_PXI_SYNC) {
		/* Acknowledge interrupt */
		REG_IRQ_IF = IRQ_PXI_SYNC;
		irq_if &= ~IRQ_PXI_SYNC;
	}

	if (irq_if & IRQ_PXI_NOT_EMPTY) {
		/* Acknowledge interrupt */
		REG_IRQ_IF = IRQ_PXI_NOT_EMPTY;
		irq_if &= ~IRQ_PXI_NOT_EMPTY;
	}

	if (irq_if & IRQ_PXI_NOT_FULL) {
		/* Acknowledge interrupt */
		REG_IRQ_IF = IRQ_PXI_NOT_FULL;
		irq_if &= ~IRQ_PXI_NOT_FULL;
	}

	/* Acknowledge unhandled interrupts */
	REG_IRQ_IF = irq_if;
}

int main()
{
	arm9_disable_irq();
	arm9_set_regular_exception_vectors();

	/* Acknowledge all the tnterrupts */
	REG_IRQ_IE = ~0;
	/* Enable all the tnterrupts */
	REG_IRQ_IF = ~0;

	arm9_enable_irq();

	unsigned char i = 0;

	for (;;)
		PXI_SYNC9 = (1 << 31) | (i++ << 8);

	return 0;
}
