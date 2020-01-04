export SYSROOT=$(shell pwd)/build/sysroot

TARGET:=build/hdd.img
PROJECTS=libc kernel programs

$(TARGET): all
	cp kernel/build/kernel.bin $(TARGET)
	truncate -s 64M $(TARGET)
	./mktfs $(TARGET) format
	mkdir -p mnt
	./tfsfuse mnt $(TARGET)
	-cp -r $(SYSROOT)/. mnt/
	fusermount -u mnt
	rmdir mnt

all: install-headers
	@for PROJECT in $(PROJECTS); do\
		$(MAKE) -C $$PROJECT;\
	done

install-headers:
	@for PROJECT in $(PROJECTS); do\
		$(MAKE) -C $$PROJECT install-headers;\
	done


run: $(TARGET)
	qemu-system-x86_64 -drive format=raw,file=$(TARGET)


clean:
	@for PROJECT in $(PROJECTS); do\
		$(MAKE) -C $$PROJECT clean;\
	done
	rm -rf build/

