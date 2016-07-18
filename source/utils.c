#include "utils.h"

void arm9_enable_irq(void)
{
	__asm__ volatile(
		"mrs r0, cpsr\n\t"
		"bic r0, r0, #0x80\n\t"
		"msr cpsr_c, r0\n\t"
		: : : "r0");
}

void arm9_disable_irq(void)
{
	__asm__ volatile(
		"mrs r0, cpsr\n\t"
		"orr r0, r0, #0x80\n\t"
		"msr cpsr_c, r0\n\t"
		: : : "r0");
}

void arm9_set_regular_exception_vectors(void)
{
	__asm__ volatile(
		"mrc p15, 0, r0, c1, c0, 0\n\t"
		"bic r0, r0, #(1 << 13)\n\t"
		"mcr p15, 0, r0, c1, c0, 0\n\t"
		: : : "r0");
}

void arm9_wfi(void)
{
	__asm__ volatile(
		"mcr p15, 0, %0, c7, c0, 4\n\t"
		: : "r"(0)
	);
}

void arm9_disable_timers(void)
{
	REG_TIMER_CNT(0) = 0;
	REG_TIMER_CNT(1) = 0;
	REG_TIMER_CNT(2) = 0;
	REG_TIMER_CNT(3) = 0;
}
