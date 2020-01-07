LIBC_SOURCES:=$(shell find libc/src -type f \( -name "*.c" -or -name "*.s" \))
LIBC_OBJECTS:=$(patsubst libc/src/%,build/libc/%.o,$(LIBC_SOURCES))

LIBK_OBJECTS:=$(patsubst libc/src/%,build/libk/%.o,$(LIBC_SOURCES))
LIBK_CFLAGS:=$(CFLAGS) -mno-red-zone

$(LIBDIR)/libc.a: $(LIBC_OBJECTS)
	@mkdir -p "$(@D)"
	@echo "AR $@"
	@$(AR) rcs $@ $^

$(LIBDIR)/libk.a: $(LIBK_OBJECTS)
	@mkdir -p "$(@D)"
	@echo "AR $@"
	@$(AR) rcs $@ $^

build/libc/%.s.o: libc/src/%.s
	@mkdir -p "$(@D)"
	@echo "AS $@"
	@$(AS) $(ASFLAGS) -o $@ $<

build/libc/%.c.o: libc/src/%.c
	@mkdir -p "$(@D)"
	@echo "CC $@"
	@$(CC) $(CFLAGS) -MD -c -o $@ $<

build/libk/%.s.o: libc/src/%.s
	@mkdir -p "$(@D)"
	@echo "AS $@"
	@$(AS) $(ASFLAGS) -o $@ $<

build/libk/%.c.o: libc/src/%.c
	@mkdir -p "$(@D)"
	@echo "CC $@"
	@$(CC) $(LIBK_CFLAGS) -MD -c -o $@ $< -D__is_libk

-include $(LIBC_OBJECTS:.o=.d)
-include $(LIBK_OBJECTS:.o=.d)
