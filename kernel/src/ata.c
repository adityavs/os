#include "kernel/ata.h"

#include <stddef.h>
#include <stdio.h>

#include "kernel/clock.h"
#include "kernel/cpu.h"
#include "kernel/panic.h"
#include "kernel/pci.h"

struct ata_device ata_devices[4] = {
	{ .base = 0x1F0, .control = 0x3F6, .slave = 0 },
	{ .base = 0x1F0, .control = 0x3F6, .slave = 1 },
	{ .base = 0x170, .control = 0x376, .slave = 0 },
	{ .base = 0x170, .control = 0x376, .slave = 1 },
};

struct ata_device* ata_get_device(int index) {
	return index < 4 ? &ata_devices[index] : NULL;
}

void ata_io_wait(struct ata_device *device) {
	for (int i = 0; i < 4; i++)
		in8(device->base + ATA_REG_ALTSTATUS);
}

void ata_device_detect(struct ata_device *device) {
	out8(device->control, 1 << 1);
	ata_io_wait(device);

	out8(device->base + ATA_REG_HDDEVSEL, 0xA0 | device->slave << 4);
	ata_io_wait(device);

	out8(device->base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	ata_io_wait(device);

	if (!in8(device->base + ATA_REG_STATUS)) return;
	while (in8(device->base + ATA_REG_STATUS) & ATA_SR_BSY);

	uint16_t c = (in8(device->base + ATA_REG_LBA2) << 8) | in8(device->base + ATA_REG_LBA1);
	if (c == 0xFFFF) return;
	if (c == 0xEB14 || c == 0x9669) {
		device->is_atapi = 1;
		out8(device->base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
		ata_io_wait(device);
	}

	out8(device->base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	ata_io_wait(device);

	ins32(device->base + ATA_REG_DATA, (uint64_t) &device->identify, 128);
	for (int i = 0; i < 40; i += 2) {
		char c = device->identify.model[i];
		device->identify.model[i] = device->identify.model[i + 1];
		device->identify.model[i + 1] = c;
	}
	char *k = &device->identify.model[39];
	while (*k == ' ')
		*k-- = '\0';
}

void ata_init() {
	uint32_t ata_pci = pci_scan(0x0101);
	if (ata_pci == (uint32_t) -1)
		panic("no IDE Controller.\n");

	for (int i = 0; i < 4; i++)
		ata_device_detect(&ata_devices[i]);
}

void ata_pio_out_lba(struct ata_device *device, uint64_t lba, uint16_t count) {
	out8(device->control, (1 << 7) | (1 << 1));
	out8(device->base + ATA_REG_SECCOUNT1 - 6, 0);
	out8(device->base + ATA_REG_LBA3 - 6, (lba >> 24) & 0xFF);
	out8(device->base + ATA_REG_LBA4 - 6, (lba >> 32) & 0xFF);
	out8(device->base + ATA_REG_LBA5 - 6, (lba >> 40) & 0xFF);
	out8(device->control, (1 << 1));
	out8(device->base + ATA_REG_SECCOUNT0, count & 0xFF);
	out8(device->base + ATA_REG_LBA0, (lba >>  0) & 0xFF);
	out8(device->base + ATA_REG_LBA1, (lba >>  8) & 0xFF);
	out8(device->base + ATA_REG_LBA2, (lba >> 16) & 0xFF);
}

void ata_read_sector(struct ata_device *device, uint64_t lba, void *buffer) {
	if (!(device->identify.capabilities & 0x200))
		panic("device does not support LBA!\n");
	while (in8(device->base + ATA_REG_STATUS) & ATA_SR_BSY);
	out8(device->base + ATA_REG_HDDEVSEL, 0xE0 | (device->slave << 4));
	ata_pio_out_lba(device, lba, 1);
	out8(device->base + ATA_REG_COMMAND, ATA_CMD_READ_PIO_EXT);
	ata_io_wait(device);
	while (in8(device->base + ATA_REG_STATUS) & ATA_SR_BSY);
	ins32(device->base + ATA_REG_DATA, (uint64_t) buffer, 128);
}

void ata_write_sector(struct ata_device *device, uint64_t lba, const void *buffer) {
	if (!(device->identify.capabilities & 0x200))
		panic("device does not support LBA!\n");
	while (in8(device->base + ATA_REG_STATUS) & ATA_SR_BSY);
	out8(device->base + ATA_REG_HDDEVSEL, 0xE0 | (device->slave << 4));
	ata_pio_out_lba(device, lba, 1);
	out8(device->base + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO_EXT);
	ata_io_wait(device);
	while (in8(device->base + ATA_REG_STATUS) & ATA_SR_BSY);
	outs32(device->base + ATA_REG_DATA, (uint64_t) buffer, 128);
	out8(device->base + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH_EXT);
}
