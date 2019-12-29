#ifndef _KERNEL_PCI_H
#define _KERNEL_PCI_H 1

#include <stdint.h>

#define PCI_CONFIG_ADDRESS	0xCF8
#define PCI_CONFIG_DATA		0xCFC

#define PCI_VENDOR_ID		0x00
#define PCI_DEVICE_ID		0x02
#define PCI_COMMAND			0x04
#define PCI_STATUS			0x06
#define PCI_REVISION_ID		0x08
#define PCI_PROG_IF			0x09
#define PCI_SUBCLASS		0x0A
#define PCI_CLASS			0x0B
#define PCI_CACHE_LINE_SIZE	0x0C
#define PCI_LATENCY_TIMER	0x0D
#define PCI_HEADER_TYPE		0x0E
#define PCI_BIST			0x0F
#define PCI_BAR0			0x10
#define PCI_BAR1			0x14
#define PCI_BAR2			0x18
#define PCI_BAR3			0x1C
#define PCI_BAR4			0x20
#define PCI_BAR5			0x24

#define PCI_SECONDARY_BUS	0x19

static inline uint8_t pci_get_bus(uint32_t device) { return (uint8_t) (device >> 16); }
static inline uint8_t pci_get_slot(uint32_t device) { return (uint8_t) (device >> 8); }
static inline uint8_t pci_get_function(uint32_t device) { return (uint8_t) (device); }

static inline uint32_t pci_make_address(uint32_t device, uint8_t field) {
	return 0x80000000 | (pci_get_bus(device) << 16) | (pci_get_slot(device) << 11) |
		(pci_get_function(device) << 8) | (field & 0xFC);
}

static inline uint32_t pci_make_device(uint8_t bus, uint8_t slot, uint8_t function) {
	return (uint32_t) ((bus << 16) | (slot << 8) | function);
}

uint32_t pci_read_field(uint32_t, uint8_t, uint8_t);
uint16_t pci_get_type(uint32_t);
uint32_t pci_scan(uint16_t);

#endif
