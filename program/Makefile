TARGET:=build/hw

AS:=x86_64-elf-as
CC:=x86_64-elf-gcc --sysroot=$(SYSROOT) -isystem=/include
LD:=$(CC)

ASFLAGS:=
CFLAGS:=-Wall -Wextra -ffreestanding -nostdlib -O0
LDFLAGS:=$(CFLAGS) -static -e main

SOURCES:=$(shell find src -type f \( -name "*.c" -or -name "*.s" \))
OBJECTS:=$(patsubst src/%,build/%.o,$(SOURCES))
LIBRARIES:=-lc
LIBRARIES_FILES:=$(SYSROOT)/lib/libc.a

all: $(TARGET)
	@mkdir -p $(SYSROOT)/bin
	cp --preserve=timestamps $(TARGET) $(SYSROOT)/bin

$(TARGET): $(OBJECTS) $(LIBRARIES_FILES)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBRARIES)

build/%.s.o: src/%.s
	@mkdir -p "$(@D)"
	echo "AS $@"
	$(AS) $(ASFLAGS) -c -o $@ $<

build/%.c.o: src/%.c
	@mkdir -p "$(@D)"
	echo "CC $@\n"
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf build/

install-headers:
