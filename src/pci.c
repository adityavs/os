#include "kernel/pci.h"

#include "kernel/io.h"
#include "kernel/stdio.h"

uint8_t pci_read_config8(uint8_t bus, uint8_t device, uint8_t function, uint8_t field) {
	out32(0xCF8, (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (field & 0xFC));
	return in8(0xCFC + (field & 3));
}

uint16_t pci_read_config16(uint8_t bus, uint8_t device, uint8_t function, uint8_t field) {
	out32(0xCF8, (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (field & 0xFC));
	return in16(0xCFC + (field & 2));
}

uint16_t pci_read_field32(uint8_t bus, uint8_t device, uint8_t func, uint8_t field) {
	out32(0xCF8, (1 << 31) | (bus << 16) | (device << 11) | (func << 8) | (field & 0xFC));
	return in32(0xCFC);
}

void check_function(uint8_t bus, uint8_t device, uint8_t function) {
	printf("\033[97mPCI (%d, %d, %d)\033[0m: ", bus, device, function);
	uint8_t class = pci_read_config8(bus, device, function, PCI_CLASS);
	uint8_t subclass = pci_read_config8(bus, device, function, PCI_SUBCLASS);
	uint8_t prog_if = pci_read_config8(bus, device, function, PCI_PROG_IF);
	printf("0x%02x 0x%02x 0x%02x\n", class, subclass, prog_if);
}

void check_device(uint8_t bus, uint8_t device) {
	if (pci_read_config16(bus, device, 0, PCI_VENDOR_ID) == 0xFFFF)
		return;
	check_function(bus, device, 0);
	if (pci_read_config8(bus, device, 0, PCI_HEADER_TYPE) & 0x80) {
		for (uint8_t function = 1; function < 8; function++) {
			if (pci_read_config16(bus, device, function, PCI_VENDOR_ID) != 0xFFFF)
				check_function(bus, device, function);
		}
	}
}

void pci_check_all_buses() {
	for (uint16_t bus = 0; bus < 256; bus++) {
		for (uint8_t device = 0; device < 32; device++) {
			check_device(bus, device);
		}
	}
}
