TARGET:=build/kernel.bin

AS:=nasm
CC:=x86_64-elf-gcc --sysroot=$(SYSROOT) -isystem=/include
LD:=$(CC)

ASFLAGS:=-f elf64
CFLAGS:=-Wall -Wextra -ffreestanding -mno-red-zone
LDFLAGS:=$(CFLAGS)

LINKER_SCRIPT:=src/linker.ld
SOURCES:=$(shell find src -type f \( -name "*.c" -or -name "*.s" -or -name "*.asm" \))
OBJECTS:=$(patsubst src/%,build/%.o,$(SOURCES))
LIBS:=-nostdlib -lk -lgcc
LIBS_DEPS:=$(SYSROOT)/lib/libk.a

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJECTS) $(LINKER_SCRIPT) $(LIBS_DEPS)
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -o build/kernel.elf $(OBJECTS) $(LIBS)
	echo "LD $@"
	objcopy -O binary --only-section={.boot,.text,.rodata,.data} build/kernel.elf $(TARGET)

build/%.s.o: src/%.s
	@mkdir -p "$(@D)"
	echo "AS $@"
	$(AS) $(ASFLAGS) -o $@ $<

build/%.c.o: src/%.c
	@mkdir -p "$(@D)"
	echo "CC $@"
	$(CC) $(CFLAGS) -MD -c -o $@ $< $(LIBS)


clean:
	rm -rf build/


install-headers:
	@mkdir -p "$(SYSROOT)/include"
	cp -R --preserve=timestamps "include/." "$(SYSROOT)/include"


-include $(OBJECTS:.o=.d)
