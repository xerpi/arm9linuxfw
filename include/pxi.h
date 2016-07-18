#ifndef PXI_H
#define PXI_H

#include "types.h"

#define REG_PXI_SYNC9	(*(vu32 *)0x10008000)
#define REG_PXI_CNT9	(*(vu32 *)0x10008004)
#define REG_PXI_SEND9	(*(vu32 *)0x10008008)
#define REG_PXI_RECV9	(*(vu32 *)0x1000800C)

#define PXI_CNT_SEND_FIFO_EMPTY		(1 << 0)
#define PXI_CNT_SEND_FIFO_FULL		(1 << 1)
#define PXI_CNT_SEND_FIFO_EMPTY_IRQ	(1 << 2)
#define PXI_CNT_SEND_FIFO_FLUSH		(1 << 3)
#define PXI_CNT_RECV_FIFO_EMPTY		(1 << 8)
#define PXI_CNT_RECV_FIFO_FULL		(1 << 9)
#define PXI_CNT_RECV_FIFO_NOT_EMPTY_IRQ	(1 << 10)
#define PXI_CNT_FIFO_ENABLE		(1 << 15)

#define PXI_SYNC_TRIGGER_PXI_SYNC11	(1 << 29)
#define PXI_SYNC_TRIGGER_PXI_SYNC9	(1 << 30)
#define PXI_SYNC_IRQ_ENABLE		(1 << 31)

#define PXI_BUFFER_SIZE 512

extern uint8_t pxi_recv_buf[PXI_BUFFER_SIZE];
extern uint8_t pxi_send_buf[PXI_BUFFER_SIZE];
extern uint32_t pxi_recv_buf_size;
extern uint32_t pxi_send_buf_size;
extern uint32_t pxi_recv_buf_head;
extern uint32_t pxi_recv_buf_tail;
extern uint32_t pxi_send_buf_head;
extern uint32_t pxi_send_buf_tail;

void pxi_init(void);

/* Do not use these directly */
void pxi_send_fifo_push(uint32_t word);
uint32_t pxi_recv_fifo_pop(void);

void pxi_send(uint32_t word);
uint32_t pxi_recv(void);

void pxi_send_buffer(const uint8_t *buffer, size_t size);
void pxi_recv_buffer(uint8_t *buffer, size_t size);

void pxi_send_fifo_flush(void);
int pxi_send_fifo_is_empty();
int pxi_send_fifo_is_full();
int pxi_recv_fifo_is_empty();
int pxi_recv_fifo_is_full();

#endif
