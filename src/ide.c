#include "kernel/ide.h"

#include "kernel/clock.h"
#include "kernel/io.h"
#include "kernel/stdio.h"

struct ide_channel_registers channels[2];
uint8_t ide_buffer[2048] = {0};
struct ide_device ide_devices[4];

void ide_write(uint8_t channel, uint8_t reg, uint8_t data) {
	if (reg > 0x7 && reg < 0xC)
		ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].no_interrupt);
	if (reg < 0x8)
		out8(channels[channel].io_base + reg, data);
	else if (reg < 0xC)
		out8(channels[channel].io_base + reg - 0x6, data);
	else if (reg < 0xE)
		out8(channels[channel].control_base + reg - 0xA, data);
	else if (reg < 0x16)
		out8(channels[channel].bus_master_ide + reg - 0xE, data);
	if (reg > 0x7 && reg < 0xC)
		ide_write(channel, ATA_REG_CONTROL, channels[channel].no_interrupt);
}

uint8_t ide_read(uint8_t channel, uint8_t reg) {
	uint8_t result;
	if (reg > 0x7 && reg < 0xC)
		ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].no_interrupt);
	if (reg < 0x8)
		result = in8(channels[channel].io_base + reg);
	else if (reg < 0xC)
		result = in8(channels[channel].io_base + reg - 0x6);
	else if (reg < 0xE)
		result = in8(channels[channel].control_base + reg - 0xA);
	else if (reg < 0x16)
		result = in8(channels[channel].bus_master_ide + reg - 0xE);
	if (reg > 0x7 && reg < 0xC)
		ide_write(channel, ATA_REG_CONTROL, channels[channel].no_interrupt);
	return result;
}

void ide_read_buffer(uint8_t channel, uint8_t reg, uint64_t buffer, uint32_t count) {
	if (reg > 0x7 && reg < 0xC)
		ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].no_interrupt);
	if (reg < 0x8)
		insl(channels[channel].io_base + reg, buffer, count);
	else if (reg < 0xC)
		insl(channels[channel].io_base + reg - 0x6, buffer, count);
	else if (reg < 0xE)
		insl(channels[channel].control_base + reg - 0xA, buffer, count);
	else if (reg < 0x16)
		insl(channels[channel].bus_master_ide + reg - 0xE, buffer, count);
	if (reg > 0x7 && reg < 0xC)
		ide_write(channel, ATA_REG_CONTROL, channels[channel].no_interrupt);
}

void ide_init(uint32_t bar0, uint32_t bar1, uint32_t bar2, uint32_t bar3, uint32_t bar4) {
	// Detect IO ports which interface IDE controller
	channels[ATA_PRIMARY].io_base = (bar0 & 0xFFFFFFFC) + 0x1F0 * (!bar0);
	channels[ATA_PRIMARY].control_base = (bar1 & 0xFFFFFFFC) + 0x3F6 * (!bar1);
	channels[ATA_PRIMARY].bus_master_ide = (bar4 & 0xFFFFFFFC) + 0;

	channels[ATA_SECONDARY].io_base = (bar2 & 0xFFFFFFFC) + 0x170 * (!bar2);
	channels[ATA_SECONDARY].control_base = (bar3 & 0xFFFFFFFC) + 0x376 * (!bar3);
	channels[ATA_SECONDARY].bus_master_ide = (bar4 & 0xFFFFFFFC) + 8;

	// Disable IRQs
	ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
	ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

	// Detect ATA/ATAPI devices
	int count = 0;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			// Assume no drive here
			ide_devices[count].reserved = 0;

			// Select drive
			ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
			sleep(1);

			// Send ATA identify command
			ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
			sleep(1);

			// Polling
			if (ide_read(i, ATA_REG_STATUS) == 0) continue;

			uint8_t error = 0, status;
			for (;;) {
				status = ide_read(i, ATA_REG_STATUS);
				if (status & ATA_SR_ERROR) { error = 1; break; }
				if (!(status & ATA_SR_BUSY) && (status & ATA_SR_DRQ)) break;
			}

			// Probe for ATAPI devices
			uint8_t type;
			if (error != 0) {
				uint8_t cl = ide_read(i, ATA_REG_LBA1);
				uint8_t ch = ide_read(i, ATA_REG_LBA2);

				if ((cl == 0x14 && ch == 0xEB) || (cl == 0x69 && ch == 0x96))
					type = IDE_ATAPI;
				else
					continue;

				ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
				sleep(1);
			}

			// Read identification space of the device
			ide_read_buffer(i, ATA_REG_DATA, (uint64_t) ide_buffer, 128);

			// Read device parameters
			ide_devices[count].reserved = 1;
			ide_devices[count].type = type;
			ide_devices[count].channel = i;
			ide_devices[count].drive = j;
			ide_devices[count].signature =
				*((uint16_t*) (ide_buffer + ATA_IDENT_DEVICETYPE));
			ide_devices[count].capabilities =
				*((uint16_t*) (ide_buffer + ATA_IDENT_CAPABILITIES));
			ide_devices[count].command_sets =
				*((uint16_t*) (ide_buffer + ATA_IDENT_COMMANDSETS));

			// Get size
			if (ide_devices[count].command_sets & (1 << 26))
				ide_devices[count].size = *((uint32_t*) (ide_buffer + ATA_IDENT_MAX_LBA_EXT));
			else
				ide_devices[count].size = *((uint32_t*) (ide_buffer + ATA_IDENT_MAX_LBA));

			// String indicates model of device
			for (int k = 0; k < 40; k += 2) {
				ide_devices[count].model[k] = ide_buffer[ATA_IDENT_MODEL + k + 1];
				ide_devices[count].model[k + 1] = ide_buffer[ATA_IDENT_MODEL + k];
			}
			char *k = (char*) ide_devices[count].model + 39;
			while (*k == ' ')
				*k-- = '\0';

			// Next!
			count++;
		}
	}

	// Print summary
	for (int i = 0; i < 4; i++) {
		if (ide_devices[i].reserved == 1) {
			printf("Found %s drive @ %d: %s (%dMiB)\n",
					(const char *[]){ "ATA", "ATAPI" }[ide_devices[i].type],
					i,
					ide_devices[i].model,
					ide_devices[i].size);
		}
	}
}
