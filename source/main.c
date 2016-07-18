#include "types.h"
#include "utils.h"
#include "pxi.h"
#include "draw.h"
#include "hid.h"
#include "i2c.h"
#include "tmio.h"

#define POWEROFF_MASK (BUTTON_L | BUTTON_R | BUTTON_DOWN | BUTTON_B)

static void mcu_poweroff()
{
	i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
	while (1) ;
}

static void check_poweroff()
{
	if ((BUTTON_HELD(HID_PAD, POWEROFF_MASK) & 0x7FF) == POWEROFF_MASK)
		mcu_poweroff();
}

void __attribute__((interrupt("IRQ"))) interrupt_vector(void)
{
	register u32 irq_if = REG_IRQ_IF;

	/* if (irq_if & (IRQ_PXI_SYNC | IRQ_PXI_NOT_EMPTY | IRQ_PXI_NOT_FULL))
		Debug("IRQ: 0x%08X", irq_if); */

	if (irq_if & IRQ_SDIO_1) {
		/* Acknowledge interrupt */
		REG_IRQ_IF = IRQ_SDIO_1;
		irq_if &= ~IRQ_SDIO_1;

		/* Tell the TMIO controller that we have an interrupt */
		sdio_1_irq = 1;
	}

	/*if (irq_if & IRQ_PXI_SYNC) {
		REG_IRQ_IF = IRQ_PXI_SYNC;
		irq_if &= ~IRQ_PXI_SYNC;
	}

	if (irq_if & IRQ_PXI_NOT_EMPTY) {
		REG_IRQ_IF = IRQ_PXI_NOT_EMPTY;
		irq_if &= ~IRQ_PXI_NOT_EMPTY;

		do {
			word = pxi_recv_fifo_pop();
			Debug("wrd: 0x%08X  head: %d  tail: %d", word, pxi_recv_buf_head, pxi_recv_buf_tail);
			*(uint32_t *)&pxi_recv_buf[pxi_recv_buf_head] = word;
			pxi_recv_buf_head = (pxi_recv_buf_head + 4) % PXI_BUFFER_SIZE;
			pxi_recv_buf_size += 4;
		} while (!pxi_recv_fifo_is_empty() && (pxi_recv_buf_size + 4) < PXI_BUFFER_SIZE);
	}

	if (irq_if & IRQ_PXI_NOT_FULL) {
		REG_IRQ_IF = IRQ_PXI_NOT_FULL;
		irq_if &= ~IRQ_PXI_NOT_FULL;

		do {
			word = *(uint32_t *)&pxi_send_buf[pxi_send_buf_tail];
			pxi_send_fifo_push(word);
			pxi_send_buf_tail = (pxi_send_buf_tail + 4) % PXI_BUFFER_SIZE;
			pxi_send_buf_size -= 4;
		} while (!pxi_send_fifo_is_full() && pxi_send_buf_size > 4);
	}*/

	/* Acknowledge unhandled interrupts */
	REG_IRQ_IF = irq_if;
}

static u8 buf[TMIO_BBS] __attribute__((aligned(TMIO_BBS)));

int main(void)
{
	u32 data;
	u32 i;

	ClearBot();
	Debug("arm9linuxfw by xerpi");

	arm9_disable_irq();
	arm9_set_regular_exception_vectors();
	arm9_disable_timers();

	/* Disable all the interrupts */
	REG_IRQ_IE = 0;
	/* Acknowledge all the tnterrupts */
	REG_IRQ_IF = ~0;
	/* Enable PXI and SD Interrupts */
	REG_IRQ_IE = IRQ_PXI_SYNC | IRQ_PXI_NOT_FULL | IRQ_PXI_NOT_EMPTY |
		IRQ_SDIO_1;

	arm9_enable_irq();

	pxi_init();
	tmio_init();
	tmio_init_sdmc();

	for (;;) {
		while (pxi_recv_fifo_is_empty())
			check_poweroff();

		data = pxi_recv_fifo_pop();

		tmio_readsectors(TMIO_DEV_SDMC, data, 1, buf);

		for (i = 0; i < TMIO_BBS; i += 4) {
			while (pxi_send_fifo_is_full())
				;
			pxi_send_fifo_push(*(uint32_t *)&buf[i]);
		}

		check_poweroff();
	}


	return 0;
}
