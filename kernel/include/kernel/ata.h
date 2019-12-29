#ifndef _KERNEL_ATA_H
#define _KERNEL_ATA_H 1

#include <stdint.h>

#define ATA_SR_BSY		0x80
#define ATA_SR_DRDY		0x40
#define ATA_SR_DF		0x20
#define ATA_SR_DSC		0x10
#define ATA_SR_DRQ		0x08
#define ATA_SR_CORR		0x04
#define ATA_SR_IDX		0x02
#define ATA_SR_ERR		0x01

#define ATA_CMD_READ_PIO		0x20
#define ATA_CMD_READ_PIO_EXT	0x24
#define ATA_CMD_READ_DMA		0xC8
#define ATA_CMD_READ_DMA_EXT	0x25
#define ATA_CMD_WRITE_PIO		0x30
#define ATA_CMD_WRITE_PIO_EXT	0x34
#define ATA_CMD_WRITE_DMA		0xCA
#define ATA_CMD_WRITE_DMA_EXT	0x35
#define ATA_CMD_CACHE_FLUSH		0xE7
#define ATA_CMD_CACHE_FLUSH_EXT	0xEA
#define ATA_CMD_PACKET			0xA0
#define ATA_CMD_IDENTIFY_PACKET	0xA1
#define ATA_CMD_IDENTIFY		0xEC

#define ATA_REG_DATA		0x00
#define ATA_REG_ERROR		0x01
#define ATA_REG_FEATURES	0x01
#define ATA_REG_SECCOUNT0	0x02
#define ATA_REG_LBA0		0x03
#define ATA_REG_LBA1		0x04
#define ATA_REG_LBA2		0x05
#define ATA_REG_HDDEVSEL	0x06
#define ATA_REG_COMMAND		0x07
#define ATA_REG_STATUS		0x07
#define ATA_REG_SECCOUNT1	0x08
#define ATA_REG_LBA3		0x09
#define ATA_REG_LBA4		0x0A
#define ATA_REG_LBA5		0x0B
#define ATA_REG_CONTROL		0x0C
#define ATA_REG_ALTSTATUS	0x0C
#define ATA_REG_DEVADDRESS	0x0D

struct ata_identify {
	uint16_t type;
	uint8_t unused1[52];
	char model[40];
	uint8_t unused2[4];
	uint16_t capabilities;
	uint8_t unused3[20];
	uint32_t max_lba;
	uint8_t unused4[76];
	uint64_t max_lba_ext;
	uint8_t unused5[304];
} __attribute__ ((packed));

struct ata_device {
	uint32_t base;
	uint32_t control;
	uint8_t slave;
	uint8_t is_atapi;
	struct ata_identify identify;
};

struct ata_device* ata_get_device(int);

void ata_init();
void ata_read_sector(struct ata_device*, uint64_t, void*);
void ata_write_sector(struct ata_device*, uint64_t, const void*);

#endif
