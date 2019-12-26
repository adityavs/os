TARGET:=build/kernel.bin

AS:=nasm
ASFLAGS:=-felf64
CC:=x86_64-elf-gcc
CFLAGS:=-Iinclude -Wall -Wextra -ffreestanding -nostdlib -lgcc -mno-red-zone
LD:=x86_64-elf-ld
LDFLAGS:=-n

LINKER_SCRIPT:=src/linker.ld
SOURCES:=$(shell find src -type f \( -name "*.c" -or -name "*.s" \))
OBJECTS:=$(patsubst src/%,build/%.o,$(SOURCES))

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJECTS) $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -o build/kernel.elf $(OBJECTS)
	objcopy -O binary --only-section={.boot,.text,.rodata,.data} build/kernel.elf $(TARGET)
	@truncate -s 66048 $(TARGET)

build/%.s.o: src/%.s
	@mkdir -p "$(@D)"
	$(AS) $(ASFLAGS) -o $@ $<

build/%.c.o: src/%.c
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -MD -c -o $@ $<


run: $(TARGET)
	qemu-system-x86_64 -drive format=raw,file=$(TARGET)

debug: $(TARGET)
	qemu-system-x86_64 -drive format=raw,file=$(TARGET) -s -S &
	st -e gdb

clean:
	rm -rf build

-include $(OBJECTS:.o=.d)
