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

#define ARM_CP15_CACHE_PREPARE_MVA(mva) \
	((const void *) (((uint32_t) (mva)) & ~0x1fU))

void data_cache_invalidate_line(const volatile void *mva)
{
	mva = ARM_CP15_CACHE_PREPARE_MVA(mva);

	asm volatile (
	"mcr p15, 0, %[mva], c7, c6, 1\n"
	:
	: [mva] "r" (mva)
	: "memory"
	);
}

void data_cache_clean_line(const volatile void *mva)
{
	mva = ARM_CP15_CACHE_PREPARE_MVA(mva);

	asm volatile (
	"mcr p15, 0, %[mva], c7, c10, 1\n"
	:
	: [mva] "r" (mva)
	: "memory"
	);
}

static inline int isalpha(int c)
{
	return ((unsigned)c | 32) - 'a' < 26;
}

static inline int isdigit(int c)
{
	return (unsigned)c - '0' < 10;
}

static inline int isalnum(int c)
{
	return isalpha(c) || isdigit(c);
}

static inline int isprint(int c)
{
	return (unsigned)c-0x20 < 0x5f;
}


extern void ClearTop(unsigned char *screen, int color);
int main(void)
{
	ClearBot();
	ClearTop(TOP_SCREEN0, RGB(255, 255, 255));
	ClearTop(TOP_SCREEN1, RGB(255, 255, 255));
	Debug("arm9linuxfw by xerpi");
	Debug("Linux LL debug");

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

	#define ARM9_FW_SHARED_BUF_PA (0x27000000)
	*(volatile char *)ARM9_FW_SHARED_BUF_PA = 0;
	data_cache_clean_line((void *)ARM9_FW_SHARED_BUF_PA);

	int x = 2, y = 2;

	for (;;) {
		//arm9_wfi();
		check_pending_work();

		volatile char *p = (char *)ARM9_FW_SHARED_BUF_PA;
		data_cache_invalidate_line(p);
		char c = *p;
		if (c != 0) {
			if (isprint(c)) {
				DrawCharacter(TOP_SCREEN0, c, x, y, RGB(0, 0, 0), RGB(255, 255, 255));
				DrawCharacter(TOP_SCREEN1, c, x, y, RGB(0, 0, 0), RGB(255, 255, 255));

				x += 8;
				if (x > 400) {
					x = 2;
					y += 8;
					DrawString(TOP_SCREEN0, "___________________________________________", x, y+8, RGB(0, 0, 0), RGB(255, 255, 255));
					DrawString(TOP_SCREEN0, "___________________________________________", x, y+8, RGB(0, 0, 0), RGB(255, 255, 255));
				}
				if (y > 240) {
					y = 2;
				}
			} else if (c == '\n') {
				x = 2;
				y += 8;
				DrawString(TOP_SCREEN0, "___________________________________________", x, y+8, RGB(0, 0, 0), RGB(255, 255, 255));
				DrawString(TOP_SCREEN0, "___________________________________________", x, y+8, RGB(0, 0, 0), RGB(255, 255, 255));
			}

			//volatile int i = 0;
			//for (i = 0; i < 50000; i++)
			//	;

			*p = 0;
			data_cache_clean_line(p);
		}

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
