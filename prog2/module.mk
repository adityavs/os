PROG2_SOURCES:=$(shell find prog2/src -type f \( -name "*.c" -or -name "*.s" \))
PROG2_OBJECTS:=$(patsubst prog2/src/%,build/prog2/%.o,$(PROG2_SOURCES))

$(BINDIR)/prog2: $(LIBDIR)/libc.a $(PROG2_OBJECTS)
	@mkdir -p "$(@D)"
	@echo "LD $@"
	@$(LD) $(LDFLAGS) -static -e main -o $@ $(PROG2_OBJECTS) -lc

build/prog2/%.s.o: prog2/src/%.s
	@mkdir -p "$(@D)"
	@echo "AS $@"
	@$(AS) $(ASFLAGS) -o $@ $<

build/prog2/%.c.o: prog2/src/%.c
	@mkdir -p "$(@D)"
	@echo "CC $@"
	@$(CC) $(CFLAGS) -MD -c -o $@ $<
