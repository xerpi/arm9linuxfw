#ifndef UTILS_H
#define UTILS_H

#include "types.h"

#define REG_IRQ_IE (*(vu32 *)0x10001000)
#define REG_IRQ_IF (*(vu32 *)0x10001004)

#define REG_TIMER_CNT(n) (*(vu32 *)(0x10003002 + 4*(n)))

#define IRQ_DMAC_1_0		(1 << 0)
#define IRQ_DMAC_1_1		(1 << 1)
#define IRQ_DMAC_1_2		(1 << 2)
#define IRQ_DMAC_1_3		(1 << 3)
#define IRQ_DMAC_1_4		(1 << 4)
#define IRQ_DMAC_1_5		(1 << 5)
#define IRQ_DMAC_1_6		(1 << 6)
#define IRQ_DMAC_1_7		(1 << 7)
#define IRQ_TIMER_0		(1 << 8)
#define IRQ_TIMER_1		(1 << 9)
#define IRQ_TIMER_2		(1 << 10)
#define IRQ_TIMER_3		(1 << 11)
#define IRQ_PXI_SYNC		(1 << 12)
#define IRQ_PXI_NOT_FULL	(1 << 13)
#define IRQ_PXI_NOT_EMPTY	(1 << 14)
#define IRQ_AES			(1 << 15)
#define IRQ_SDIO_1		(1 << 16)
#define IRQ_SDIO_1_ASYNC	(1 << 17)
#define IRQ_SDIO_3		(1 << 18)
#define IRQ_SDIO_3_ASYNC	(1 << 19)
#define IRQ_DEBUG_RECV		(1 << 20)
#define IRQ_DEBUG_SEND		(1 << 21)
#define IRQ_RSA			(1 << 22)
#define IRQ_CTR_CARD_1		(1 << 23)
#define IRQ_CTR_CARD_2		(1 << 24)
#define IRQ_CGC			(1 << 25)
#define IRQ_CGC_DET		(1 << 26)
#define IRQ_DS_CARD		(1 << 27)
#define IRQ_DMAC_2		(1 << 28)
#define IRQ_DMAC_2_ABORT	(1 << 29)

void arm9_enable_irq(void);
void arm9_disable_irq(void);
void arm9_set_regular_exception_vectors(void);
void arm9_wfi(void);
void arm9_disable_timers(void);

#endif
