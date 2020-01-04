TARGET:=$(patsubst %.c,build/%,$(shell find . -name "*.c"))

CC:=x86_64-elf-gcc --sysroot=$(SYSROOT) -isystem=/include
CFLAGS:=-Wall -Wextra -ffreestanding -nostdlib -emain

all: $(TARGET)
	@mkdir -p $(SYSROOT)/bin
	cp $(TARGET) $(SYSROOT)/bin

build/%: %.c
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf build/

install-headers:
