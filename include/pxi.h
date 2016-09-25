#ifndef PXI_H
#define PXI_H

#include "types.h"
#include "pxi_cmd.h"

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

void pxi_init(void);
void pxi_deinit(void);
void pxi_recv_cmd_hdr(struct pxi_cmd_hdr *cmd);
void pxi_recv_buffer(void *data, u32 size);
void pxi_send_cmd_response(struct pxi_cmd_hdr *cmd);

#endif
