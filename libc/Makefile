TARGET:=build/libc.a build/libk.a

AS:=nasm
CC:=x86_64-elf-gcc --sysroot=$(SYSROOT) -isystem=/include
AR:=x86_64-elf-ar

ASFLAGS:=-f elf64
CFLAGS:=-Wall -Wextra -ffreestanding -nostdlib -mno-red-zone
LIBK_ASFLAGS:=$(ASFLAGS)
LIBK_CFLAGS:=$(CFLAGS) -D__is_libk

SOURCES:=$(shell find src -type f \( -name "*.c" -or -name "*.s" \))
OBJECTS:=$(patsubst src/%,build/%.o,$(SOURCES))
LIBK_OBJECTS:=$(OBJECTS:.o=.libk.o)

all: $(TARGET)
	@mkdir -p "$(SYSROOT)/lib"
	cp --preserve=timestamps $(TARGET) "$(SYSROOT)/lib"

build/libc.a: $(OBJECTS)
	@mkdir -p "$(@D)"
	echo "AR $@"
	$(AR) rcs $@ $(OBJECTS)

build/libk.a: $(LIBK_OBJECTS)
	@mkdir -p "$(@D)"
	echo "AR $@"
	$(AR) rcs $@ $(LIBK_OBJECTS)

build/%.s.o: src/%.s
	@mkdir -p "$(@D)"
	echo "AS $@"
	$(AS) $(ASFLAGS) -o $@ $<

build/%.c.o: src/%.c
	@mkdir -p "$(@D)"
	echo "CC $@"
	$(CC) $(CFLAGS) -MD -c -o $@ $<

build/%.s.libk.o: src/%.s
	@mkdir -p "$(@D)"
	echo "AS $@"
	$(AS) $(LIBK_ASFLAGS) -o $@ $<

build/%.c.libk.o: src/%.c
	@mkdir -p "$(@D)"
	echo "CC $@"
	$(CC) $(LIBK_CFLAGS) -MD -c -o $@ $<


clean:
	rm -rf build/


install-headers:
	@mkdir -p "$(SYSROOT)/include"
	cp -R --preserve=timestamps "include/." "$(SYSROOT)/include"


-include $(OBJECTS:.o=.d)
-include $(LIBK_OBJECTS:.o=.d)
