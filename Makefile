ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/base_tools

TARGET = $(notdir $(CURDIR))
OBJS   = source/start.o source/main.o source/tmio.o source/delay.o

ARCH = -mcpu=arm946e-s -march=armv5te -mlittle-endian -mthumb-interwork
ASFLAGS = $(ARCH) -x assembler-with-cpp
CFLAGS  = -Wall -O0 -fno-builtin -nostartfiles -fomit-frame-pointer $(ARCH) -Iinclude

all: $(TARGET).bin

$(TARGET).elf: $(OBJS)
	$(CC) -T linker.ld $(CFLAGS) $^ -o $@

%.bin: %.elf
	$(OBJCOPY) -O binary --set-section-flags .bss=alloc,load,contents $< $@

.s.o:
	$(CC) $(ASFLAGS) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(TARGET).elf $(TARGET).bin $(OBJS)

copy: $(TARGET).bin
	cp $(TARGET).bin $(SD3DS)/linux && sync
	@echo "Copied!"
