ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/base_tools

TARGET	= $(notdir $(CURDIR))
OBJS	= source/start.o source/main.o source/tmio.o source/delay.o \
	source/draw.o source/i2c.o source/utils.o source/pxi.o source/tiny-printf.o source/libc.o
ARCH	= -mcpu=arm946e-s -march=armv5te -mlittle-endian -mthumb-interwork
ASFLAGS	= $(ARCH) -x assembler-with-cpp
CFLAGS 	= -Wall -O0 -fno-builtin -nostartfiles -nostdlib -fomit-frame-pointer $(ARCH) -Iinclude
DEPS	= $(OBJS:.o=.d)

all: $(TARGET).bin

debug: CFLAGS += -DDEBUG
debug: ASFLAGS += -DDEBUG
debug: $(TARGET).bin

$(TARGET).elf: $(OBJS)
	$(CC) -T linker.ld $(CFLAGS) $^ -o $@

%.bin: %.elf
	$(OBJCOPY) -S -O binary --set-section-flags .bss=alloc,load,contents $< $@

.s.o:
	$(CC) $(ASFLAGS) -MMD -MP -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	@rm -f $(TARGET).elf $(TARGET).bin $(OBJS) $(DEPS)

copy: $(TARGET).bin
	cp $(TARGET).bin $(SD3DS)/linux && sync
	@echo "Copied!"

-include $(DEPS)
