HW_SOURCES:=$(shell find hw/src -type f \( -name "*.c" -or -name "*.s" \))
HW_OBJECTS:=$(patsubst hw/src/%,build/hw/%.o,$(HW_SOURCES))

$(BINDIR)/hw: $(LIBDIR)/libc.a $(HW_OBJECTS)
	@mkdir -p "$(@D)"
	@echo "LD $@"
	@$(LD) $(LDFLAGS) -static -e main -o $@ $(HW_OBJECTS) -lc

build/hw/%.s.o: hw/src/%.s
	@mkdir -p "$(@D)"
	@echo "AS $@"
	@$(AS) $(ASFLAGS) -o $@ $<

build/hw/%.c.o: hw/src/%.c
	@mkdir -p "$(@D)"
	@echo "CC $@"
	@$(CC) $(CFLAGS) -MD -c -o $@ $<
