export SYSROOT="$(shell pwd)/build/sysroot"

PROJECTS=libc kernel

all: hdd.img

hdd.img: install
	dd if=build/kernel.bin of=hdd.img conv=notrunc status=none
	./mktfs hdd.img 131072 update

install: install-headers
	@for PROJECT in $(PROJECTS); do\
		$(MAKE) -C $$PROJECT install;\
	done

install-headers:
	@for PROJECT in $(PROJECTS); do\
		$(MAKE) -C $$PROJECT install-headers;\
	done


run: hdd.img
	qemu-system-x86_64 -drive format=raw,file=hdd.img


clean:
	@for PROJECT in $(PROJECTS); do\
		$(MAKE) -C $$PROJECT clean;\
	done
	rm -rf build/

