OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

SECTIONS
{
	. = 0x08000000;
	.text.start : { *(.text.start) }
	.text       : { *(.text) }
	.rodata     : { *(.rodata) }
	.data       : { *(.data) }
	.bss        : { *(.bss COMMON) }
	. = ALIGN(0x1000);
	.stack : {
		. += 0x1000;
		_stack_end = .;
	}
	. = ALIGN(0x1000);
	.irq_stack : {
		. += 0x1000;
		_irq_stack_end = .;
	}
	__end__ = ABSOLUTE(.);
}
