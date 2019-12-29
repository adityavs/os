#include "kernel/pci.h"

#include <stdio.h>

#include "kernel/io.h"
#include "kernel/panic.h"

uint32_t pci_read_field(uint32_t device, uint8_t field, uint8_t size) {
	out32(PCI_CONFIG_ADDRESS, pci_make_address(device, field));
	if (size == 1) return in8(PCI_CONFIG_DATA + (field & 3));
	if (size == 2) return in16(PCI_CONFIG_DATA + (field & 2));
	if (size == 4) return in32(PCI_CONFIG_DATA);
	panic("invalid size for pci_read_field: %d\n", size); return -1;
}

uint16_t pci_get_type(uint32_t device) {
	return (pci_read_field(device, PCI_CLASS, 1) << 8) |
		pci_read_field(device, PCI_SUBCLASS, 1);
}

uint32_t pci_scan_bus(uint16_t, uint8_t);
uint32_t pci_scan_function(uint16_t type, uint8_t bus, uint8_t slot, uint8_t function) {
	uint32_t device = pci_make_device(bus, slot, function);
	if (pci_get_type(device) == type)
		return device;
	if (pci_get_type(device) == 0x0604) // Bridge
		return pci_scan_bus(type, pci_read_field(device, PCI_SECONDARY_BUS, 1));
	return (uint32_t) -1;
}

uint32_t pci_scan_slot(uint16_t type, uint8_t bus, uint8_t slot) {
	uint32_t device = pci_make_device(bus, slot, 0);
	if (pci_read_field(device, PCI_VENDOR_ID, 2) == 0xFFFF)
		return (uint32_t) -1;
	if (pci_scan_function(type, bus, slot, 0) != (uint32_t) -1)
		return device;
	if (!pci_read_field(device, PCI_HEADER_TYPE, 1))
		return (uint32_t) -1;
	for (uint8_t function = 1; function < 8; function++) {
		uint32_t device = pci_make_device(bus, slot, function);
		if (pci_read_field(device, PCI_VENDOR_ID, 2) != 0xFFFF)
			if (pci_scan_function(type, bus, slot, function) != (uint32_t) -1)
				return device;
	}
	return (uint32_t) -1;
}

uint32_t pci_scan_bus(uint16_t type, uint8_t bus) {
	for (uint8_t slot = 0; slot < 32; slot++) {
		uint32_t out = pci_scan_slot(type, bus, slot);
		if (out != (uint32_t) -1) return out;
	}
	return (uint32_t) -1;
}

uint32_t pci_scan(uint16_t type) {
	if (!(pci_read_field(0, PCI_HEADER_TYPE, 1) & 0x80))
		return pci_scan_bus(type, 0);
	for (uint8_t function = 0; function < 8; function++) {
		uint32_t device = pci_make_device(0, 0, function);
		if (pci_read_field(device, PCI_VENDOR_ID, 2) == 0xFFFF)
			break;
		uint32_t out = pci_scan_bus(type, function);
		if (out != (uint32_t) -1) return out;
	}
	return (uint32_t) -1;
}
