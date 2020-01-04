TARGET=build/libk.a

AR=x86_64-elf-ar
CC=x86_64-elf-gcc --sysroot=$(SYSROOT) -isystem=/include

CFLAGS=-ffreestanding -Wall -Wextra -mno-red-zone
LIBK_CFLAGS=$(CFLAGS) -D__is_libk

FREEOBJS=$(patsubst %,build/%.o,\
	ctype.c\
	stdio.c\
	stdlib.c\
	string.c\
)
HOSTEDOBJS=$(patsubst %,build/%.o,\
)

OBJS=$(FREEOBJS) $(HOSTEDOBJS)
LIBK_OBJS=$(FREEOBJS:.o=.libk.o)

all: $(TARGET)
	@mkdir -p "$(SYSROOT)/lib"
	cp $(TARGET) "$(SYSROOT)/lib"

build/libc.a: $(OBJS)
	@mkdir -p "$(@D)"
	$(AR) rcs $@ $(OBJS)

build/libk.a: $(LIBK_OBJS)
	@mkdir -p "$(@D)"
	$(AR) rcs $@ $(LIBK_OBJS)

build/%.c.o: src/%.c
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -MD -c -o $@ $<

build/%.c.libk.o: src/%.c
	@mkdir -p "$(@D)"
	$(CC) $(LIBK_CFLAGS) -MD -c -o $@ $<


clean:
	rm -rf build/


install-headers:
	@mkdir -p "$(SYSROOT)/include"
	cp -R --preserve=timestamps "include/." "$(SYSROOT)/include"


-include $(OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)
