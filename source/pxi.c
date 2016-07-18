#include "pxi.h"
#include "utils.h"

uint8_t pxi_recv_buf[PXI_BUFFER_SIZE] __attribute__((aligned(32)));
uint8_t pxi_send_buf[PXI_BUFFER_SIZE] __attribute__((aligned(32)));
uint32_t pxi_recv_buf_size;
uint32_t pxi_send_buf_size;
uint32_t pxi_recv_buf_head;
uint32_t pxi_recv_buf_tail;
uint32_t pxi_send_buf_head;
uint32_t pxi_send_buf_tail;

void pxi_init(void)
{
	REG_PXI_CNT9 = PXI_CNT_SEND_FIFO_EMPTY_IRQ |
		PXI_CNT_RECV_FIFO_NOT_EMPTY_IRQ |
		PXI_CNT_SEND_FIFO_FLUSH |
		PXI_CNT_FIFO_ENABLE;

	/* PXI_SYNC IRQ enable (for local processor) */
	REG_PXI_SYNC9 = PXI_SYNC_IRQ_ENABLE;

	pxi_recv_buf_size = 0;
	pxi_send_buf_size = 0;
	pxi_recv_buf_head = 0;
	pxi_recv_buf_tail = 0;
	pxi_send_buf_head = 0;
	pxi_send_buf_tail = 0;
}

void pxi_send_fifo_push(uint32_t word)
{
	REG_PXI_SEND9 = word;
	/*REG_PXI_SYNC9 = PXI_SYNC_IRQ_ENABLE |
		PXI_SYNC_TRIGGER_PXI_SYNC11;*/
}

uint32_t pxi_recv_fifo_pop(void)
{
	return REG_PXI_RECV9;
}

void pxi_send(uint32_t word)
{
	if (pxi_send_fifo_is_empty()) {
		pxi_send_fifo_push(word);
	} else {

		while (pxi_send_buf_size + 4 >= PXI_BUFFER_SIZE)
			arm9_wfi();

		*(uint32_t *)&pxi_send_buf[pxi_send_buf_head] = word;
		pxi_send_buf_head = (pxi_send_buf_head + 4) % PXI_BUFFER_SIZE;
		pxi_send_buf_size += 4;
	}
}

uint32_t pxi_recv(void)
{
	uint32_t word;

	while (pxi_recv_buf_size < 4)
		arm9_wfi();

	word = *(uint32_t *)&pxi_recv_buf[pxi_recv_buf_tail];
	pxi_recv_buf_tail = (pxi_recv_buf_tail + 4) % PXI_BUFFER_SIZE;
	pxi_recv_buf_size -= 4;

	return word;
}

void pxi_send_buffer(const uint8_t *buffer, size_t size)
{
	size_t i;

	for (i = 0; i + 4 < size; i += 4)
		pxi_send(*(uint32_t *)&buffer[i]);
}

void pxi_recv_buffer(uint8_t *buffer, size_t size)
{
	size_t i;

	for (i = 0; i + 4 < size; i += 4)
		*(uint32_t *)&buffer[i] = pxi_recv();
}

void pxi_send_fifo_flush(void)
{
	REG_PXI_CNT9 = PXI_CNT_SEND_FIFO_EMPTY_IRQ |
		PXI_CNT_RECV_FIFO_NOT_EMPTY_IRQ |
		PXI_CNT_SEND_FIFO_FLUSH |
		PXI_CNT_FIFO_ENABLE;
}

int pxi_send_fifo_is_empty()
{
	return REG_PXI_CNT9 & PXI_CNT_SEND_FIFO_EMPTY;
}

int pxi_send_fifo_is_full()
{
	return REG_PXI_CNT9 & PXI_CNT_SEND_FIFO_FULL;
}

int pxi_recv_fifo_is_empty()
{
	return REG_PXI_CNT9 & PXI_CNT_RECV_FIFO_EMPTY;
}

int pxi_recv_fifo_is_full()
{
	return REG_PXI_CNT9 & PXI_CNT_RECV_FIFO_FULL;
}

