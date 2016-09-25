#include "pxi.h"
#include "utils.h"

static inline int pxi_send_fifo_is_empty()
{
	return REG_PXI_CNT9 & PXI_CNT_SEND_FIFO_EMPTY;
}

static inline int pxi_send_fifo_is_full()
{
	return REG_PXI_CNT9 & PXI_CNT_SEND_FIFO_FULL;
}

static inline int pxi_recv_fifo_is_empty()
{
	return REG_PXI_CNT9 & PXI_CNT_RECV_FIFO_EMPTY;
}

static inline int pxi_recv_fifo_is_full()
{
	return REG_PXI_CNT9 & PXI_CNT_RECV_FIFO_FULL;
}

static inline void pxi_send_fifo_push(u32 word)
{
	REG_PXI_SEND9 = word;
}

static inline u32 pxi_recv_fifo_pop(void)
{
	return REG_PXI_RECV9;
}

static inline void pxi_trigger_sync11_irq(void)
{
	REG_PXI_SYNC9 |= PXI_SYNC_TRIGGER_PXI_SYNC11;
}

static void pxi_reset(void)
{
	unsigned int i;

	REG_PXI_SYNC9 = 0;
	REG_PXI_CNT9 = PXI_CNT_SEND_FIFO_FLUSH;

	for (i = 0; i < 32; i++) {
		REG_PXI_RECV9;
	}

	REG_PXI_CNT9 = PXI_CNT_SEND_FIFO_EMPTY_IRQ |
		PXI_CNT_RECV_FIFO_NOT_EMPTY_IRQ |
		PXI_CNT_FIFO_ENABLE;

	REG_PXI_SYNC9 = PXI_SYNC_IRQ_ENABLE;
}

void pxi_init(void)
{
	pxi_reset();
}

void pxi_deinit(void)
{
}

void pxi_recv_cmd_hdr(struct pxi_cmd_hdr *cmd)
{
	*(u32 *)cmd = pxi_recv_fifo_pop();
}

void pxi_recv_buffer(void *data, u32 size)
{
	u32 i;

	for (i = 0; i < size; i+=4) {
		while (pxi_recv_fifo_is_empty())
			;
		((u32 *)data)[i/4] = pxi_recv_fifo_pop();
	}
}

void pxi_send_cmd_response(struct pxi_cmd_hdr *cmd)
{
	unsigned int i;

	while (pxi_send_fifo_is_full())
		;

	/*
	 * Send command ID and length.
	 */
	pxi_send_fifo_push(*(u32 *)cmd);
	pxi_trigger_sync11_irq();

	/*
	 * Send the command payload (if any).
	 */
	for (i = 0; i < cmd->len; i+=4) {
		pxi_send_fifo_push(cmd->data[i/4]);
	}
}

