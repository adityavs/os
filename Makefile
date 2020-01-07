export SYSROOT=$(shell pwd)/build/sysroot

TARGET:=build/hdd.img
PROJECTS=libc kernel program

$(TARGET): all
	@cp kernel/build/kernel.bin $(TARGET)
	@truncate -s 64M $(TARGET)
	@./mktfs $(TARGET) format
	@mkdir -p mnt
	@./tfsfuse mnt $(TARGET)
	@-cp -r $(SYSROOT)/. mnt/
	@fusermount -u mnt
	@rmdir mnt

all: install-headers
	@for PROJECT in $(PROJECTS); do\
		printf "\033[1;37m$$PROJECT\033[0m\n";\
		$(MAKE) -s -C $$PROJECT;\
		echo "";\
	done

install-headers:
	@for PROJECT in $(PROJECTS); do\
		$(MAKE) -s -C $$PROJECT install-headers;\
	done


run: $(TARGET)
	qemu-system-x86_64 -drive format=raw,file=$(TARGET)


clean:
	@for PROJECT in $(PROJECTS); do\
		$(MAKE) -C $$PROJECT clean;\
	done
	rm -rf build/
