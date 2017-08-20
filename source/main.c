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

static union {
	struct pxi_cmd_hdr header;
	u8 buffer[128];
} pxi_work;

static void reset_pxi_pending_work(void)
{
	pxi_work.header.cmd = PXI_CMD_NONE;
}

static void reset_pending_work(void)
{
	reset_pxi_pending_work();
}

static void do_pxi_cmd_ping_work(void)
{
	struct pxi_cmd_hdr *cmd =
		(struct pxi_cmd_hdr *)&pxi_work;
	struct pxi_cmd_hdr resp;
	resp.cmd = cmd->cmd;
	resp.len = 0;

	pxi_send_cmd_response(&resp);
	reset_pxi_pending_work();
}

static void do_pxi_cmd_sdmmc_read_sector_work(void)
{
	struct pxi_cmd_sdmmc_read_sector *cmd =
		(struct pxi_cmd_sdmmc_read_sector *)&pxi_work;
	struct pxi_cmd_hdr resp;
	resp.cmd = cmd->header.cmd;
	resp.len = 0;

	//Debug("{sector = 0x%08X, paddr = 0x%08X}", cmd->sector, cmd->paddr);

	tmio_readsectors(TMIO_DEV_SDMC, cmd->sector, 1, (void *)cmd->paddr);

	pxi_send_cmd_response(&resp);
	reset_pxi_pending_work();
}

static void do_pxi_cmd_sdmmc_write_sector_work(void)
{
    struct pxi_cmd_sdmmc_write_sector *cmd =
        (struct pxi_cmd_sdmmc_write_sector *)&pxi_work;
    struct pxi_cmd_hdr resp;
    resp.cmd = cmd->header.cmd;
    resp.len = 0;

    tmio_writesectors(TMIO_DEV_SDMC, cmd->sector, 1, (void *)cmd->paddr);

    pxi_send_cmd_response(&resp);
    reset_pxi_pending_work();
}

static void do_pxi_cmd_sdmmc_get_size_work(void)
{
    struct pxi_cmd_hdr *cmd =
        (struct pxi_cmd_hdr *)&pxi_work;
    struct pxi_resp_sdmmc_get_size resp;
    resp.header.cmd = cmd->cmd;
    resp.header.len = 4;

    resp.size = tmio_dev[TMIO_DEV_SDMC].total_size;

    pxi_send_cmd_response((struct pxi_cmd_hdr*)&resp);
    reset_pxi_pending_work();
}

static void check_pending_work(void)
{
	if (pxi_work.header.cmd != PXI_CMD_NONE) {
		switch (pxi_work.header.cmd) {
		case PXI_CMD_PING:
			do_pxi_cmd_ping_work();
			break;
		case PXI_CMD_SDMMC_READ_SECTOR:
			do_pxi_cmd_sdmmc_read_sector_work();
			break;
        case PXI_CMD_SDMMC_WRITE_SECTOR:
            do_pxi_cmd_sdmmc_write_sector_work();
            break;
        case PXI_CMD_SDMMC_GET_SIZE:
            do_pxi_cmd_sdmmc_get_size_work();
            break;
		}
	}
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

	if (irq_if & IRQ_PXI_SYNC) {
		REG_IRQ_IF = IRQ_PXI_SYNC;
		irq_if &= ~IRQ_PXI_SYNC;

		pxi_recv_cmd_hdr(&pxi_work.header);
		pxi_recv_buffer(pxi_work.header.data, pxi_work.header.len);
	}

	/*if (irq_if & IRQ_PXI_NOT_EMPTY) {
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

int main(void)
{
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

	reset_pending_work();
	arm9_enable_irq();

	pxi_init();
	tmio_init();
	tmio_init_sdmc();

	for (;;) {
		//arm9_wfi();
		check_pending_work();


		/*while (pxi_recv_fifo_is_empty())
			check_poweroff();

		data = pxi_recv_fifo_pop();

		tmio_readsectors(TMIO_DEV_SDMC, data, 1, buf);

		for (i = 0; i < TMIO_BBS; i += 4) {
			while (pxi_send_fifo_is_full())
				;
			pxi_send_fifo_push(*(uint32_t *)&buf[i]);
		}*/

		check_poweroff();
	}


	return 0;
}
