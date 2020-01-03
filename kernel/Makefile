SYSROOT?=../base
TARGET:=build/kernel.bin

AS:=nasm
CC:=x86_64-elf-gcc --sysroot=$(SYSROOT) -isystem=/include
LD:=$(CC)

ASFLAGS:=-felf64
CFLAGS:=-Wall -Wextra -ffreestanding -mno-red-zone
LDFLAGS:=-Wall -Wextra -ffreestanding

LINKER_SCRIPT:=src/linker.ld
SOURCES:=$(shell find src -type f \( -name "*.c" -or -name "*.s" \))
OBJECTS:=$(patsubst src/%,build/%.o,$(SOURCES))
LIBS:=-nostdlib -lk -lgcc

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJECTS) $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -o build/kernel.elf $(OBJECTS) $(LIBS)
	objcopy -O binary --only-section={.boot,.text,.rodata,.data} build/kernel.elf $(TARGET)

build/%.s.o: src/%.s
	@mkdir -p "$(@D)"
	$(AS) $(ASFLAGS) -o $@ $<

build/%.c.o: src/%.c
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -MD -c -o $@ $< $(LIBS)


clean:
	rm -rf build/


install: install-headers install-kernel

install-headers:
	@mkdir -p "$(SYSROOT)/include"
	cp -R --preserve=timestamps "include/." "$(SYSROOT)/include/."

install-kernel: $(TARGET)
	@mkdir -p "$(SYSROOT)/.."
	cp "$(TARGET)" "$(SYSROOT)/.."


-include $(OBJECTS:.o=.d)
