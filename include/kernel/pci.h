#ifndef _KERNEL_PCI_H
#define _KERNEL_PCI_H 1

#include <stdint.h>

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

void pci_check_all_buses();

#endif
