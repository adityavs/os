KERNEL_SOURCES:=$(shell find kernel/src -type f \( -name "*.c" -or -name "*.s" \))
KERNEL_OBJECTS:=$(patsubst kernel/src/%,build/kernel/%.o,$(KERNEL_SOURCES))
KERNEL_LINKER:=kernel/src/linker.ld
KERNEL_CFLAGS:=$(CFLAGS) -mno-red-zone

$(BUILDDIR)/kernel.bin: $(LIBDIR)/libk.a $(KERNEL_OBJECTS) $(KERNEL_LINKER)
	@mkdir -p "$(@D)"
	@echo "LD $@"
	@$(LD) $(LDFLAGS) -T $(KERNEL_LINKER) -o build/kernel/kernel.elf $(KERNEL_OBJECTS) -lk
	@objcopy -O binary --only-section={.boot,.text,.rodata,.data} build/kernel/kernel.elf $@

build/kernel/%.s.o: kernel/src/%.s
	@mkdir -p "$(@D)"
	@echo "AS $@"
	@$(AS) $(ASFLAGS) -o $@ $<

build/kernel/%.c.o: kernel/src/%.c
	@mkdir -p "$(@D)"
	@echo "CC $@"
	@$(CC) $(KERNEL_CFLAGS) -MD -c -o $@ $<

-include $(KERNEL_OBJECTS:.o=.d)
