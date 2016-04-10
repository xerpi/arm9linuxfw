include $(DEVKITARM)/base_tools

TARGET = $(notdir $(CURDIR))
OBJS   = source/start.o source/main.o

ASFLAGS = -mthumb-interwork -mcpu=arm946e-s -march=armv5te -mlittle-endian
CFLAGS  = -Iinclude $(ASFLAGS) -Wall -O0 -fno-builtin -nostartfiles -fomit-frame-pointer

all: $(TARGET).bin

$(TARGET).elf: $(OBJS)
	$(CC) -Tlink.ld $(CFLAGS) $^ -o $@

.s.o:
	$(AS) $(ASFLAGS) $< -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	@rm -f $(TARGET).elf $(TARGET).bin $(OBJS)

copy: $(TARGET).bin
	@cp $(TARGET).bin /mnt/3DS/
	@sync
	@echo "Copied!"
