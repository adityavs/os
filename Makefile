USERLAND:=prog1 prog2
MODULES:=kernel libc $(USERLAND)

BUILDDIR:=build
SYSROOT:=$(BUILDDIR)/sysroot
BINDIR:=$(SYSROOT)/bin
INCLUDEDIR:=$(SYSROOT)/include
LIBDIR:=$(SYSROOT)/lib

AS:=nasm
AR:=x86_64-elf-ar
CC:=x86_64-elf-gcc --sysroot=$(SYSROOT) -isystem=/include
LD:=x86_64-elf-gcc --sysroot=$(SYSROOT) -isystem=/include

ASFLAGS:=-f elf64
ARFLAGS:=
CFLAGS:=-Wall -Wextra -ffreestanding -nostdlib -O3
LDFLAGS:=-nostdlib

all: headers $(BUILDDIR)/hdd.img

include $(patsubst %,%/module.mk,$(MODULES))

$(BUILDDIR)/hdd.img: $(BUILDDIR)/kernel.bin $(addprefix $(BINDIR)/,$(USERLAND))
	@echo "PACK $@"
	@cp $(BUILDDIR)/kernel.bin $@
	@truncate -s 64M $@
	@./mktfs $@ format
	@mkdir -p .mnt
	@./tfsfuse .mnt $@
	@cp -r $(SYSROOT)/. .mnt/
	@fusermount -u .mnt
	@rmdir .mnt

headers:
	@mkdir -p $(INCLUDEDIR)
	@for MODULE in $(MODULES); do\
		if [[ -d "$$MODULE/include" ]]; then\
			cp -au $$MODULE/include/. $(INCLUDEDIR);\
		fi;\
	done

run: all
	@echo "QEMU $(BUILDDIR)/hdd.img"
	@qemu-system-x86_64 -drive format=raw,file=$(BUILDDIR)/hdd.img

clean:
	rm -rf $(BUILDDIR)
