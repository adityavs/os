PROG1_SOURCES:=$(shell find prog1/src -type f \( -name "*.c" -or -name "*.s" \))
PROG1_OBJECTS:=$(patsubst prog1/src/%,build/prog1/%.o,$(PROG1_SOURCES))

$(BINDIR)/prog1: $(LIBDIR)/libc.a $(PROG1_OBJECTS)
	@mkdir -p "$(@D)"
	@echo "LD $@"
	@$(LD) $(LDFLAGS) -static -e main -o $@ $(PROG1_OBJECTS) -lc

build/prog1/%.s.o: prog1/src/%.s
	@mkdir -p "$(@D)"
	@echo "AS $@"
	@$(AS) $(ASFLAGS) -o $@ $<

build/prog1/%.c.o: prog1/src/%.c
	@mkdir -p "$(@D)"
	@echo "CC $@"
	@$(CC) $(CFLAGS) -MD -c -o $@ $<

-include $(PROG1_OBJECTS:.o=.d)
