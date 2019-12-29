#include "kernel/tfs.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kernel/ata.h"
#include "kernel/clock.h"

struct ata_device *device;
struct tfs_super super;

inline static void disk_read(uint32_t sector, void *node) {
	ata_read_sector(device, sector, node);
}

inline static void disk_write(uint32_t sector, const void *node) {
	ata_write_sector(device, sector, node);
}

void tfs_init() {
	device = ata_get_device(0);
	disk_read(0, &super);
}

void tfs_format() {
	// Super block
	super.total_blocks = device->identify.max_lba_ext * 1024 / 512;
	super.bitmap_blocks = (super.total_blocks + 4095) / 4096;
	super.bitmap_offset = super.total_blocks - super.bitmap_blocks;
	super.boot_signature = 0xAA55;
	disk_write(0, &super);

	// Root directory
	struct tfs_node root = { 0 };
	root.type = TFS_DIRECTORY;
	root.size = 0;
	root.pointer = 0;
	root.time = get_milliseconds() * 65536;
	root.name[0] = 0;
	root.parent = 0;
	root.next = 0;
	disk_write(2048, &root);

	// Bitmap
	uint8_t buffer[512] = { 0 };
	memset(buffer, 0xFF, 256); // Reserved blocks
	buffer[256] |= 1 << 7; // Root directory block
	disk_write(super.bitmap_offset, &buffer);
}

/*
 * Bitmap
 */
int tfs_bitmap_alloc(int size) {
	int buffer_block = super.bitmap_offset;
	uint8_t buffer[512];

	int index = 0;
	int length = 0;
	for (uint32_t i = 0; i < super.total_blocks; i++) {
		if (i % 4096 == 0)
			disk_read(buffer_block++, &buffer);
		int byte = (i % 4096) / 8;
		if (buffer[byte] == 0xFF)
			continue;
		if (!(buffer[byte] & (1 << (7 - (i % 8))))) {
			if (length == 0)
				index = i;
			if (++length == size)
				break;
		} else {
			index = length = 0;
		}
		if (length == size)
			break;
	}
	if (index == 0 || length != size) {
		printf(" * disk full (%d, %d, %d)\n", index, length, size);
		return -1;
	}

	int index_block = super.bitmap_offset + (index / 4096);
	disk_read(index_block, &buffer);
	for (int i = index; i < index + size; i++) {
		buffer[(i % 4096) / 8] |= (1 << (7 - (i % 8)));
		if ((i + 1) % 4096 == 0 && (i + 1) < (index + size)) {
			disk_write(index_block, &buffer);
			disk_read(++index_block, &buffer);
		}
	}
	disk_write(index_block, &buffer);

	return index;
}

void tfs_bitmap_free(int index, int size) {
	uint8_t buffer[512];

	int index_block = super.bitmap_offset + (index / 4096);
	disk_read(index_block, &buffer);
	for (int i = index; i < index + size; i++) {
		buffer[(i % 4096) / 8] &= ~(1 << (7 - (i % 8)));
		if ((i + 1) % 4096 == 0 && (i + 1) < (index + size)) {
			disk_write(index_block, &buffer);
			disk_read(++index_block, &buffer);
		}
	}
	disk_write(index_block, &buffer);
}

/*
 * Children
 */
int tfs_child_add(uint32_t block, uint32_t child_block, enum tfs_type child_type, const char *child_name) {
	struct tfs_node node;
	disk_read(block, &node);

	if (node.type != TFS_DIRECTORY) {
		printf("Error: attempted to add a child to a non-directory.\n");
		return -1;
	}

	if (node.pointer == 0) {
		node.pointer = child_block;
		node.size = 1;
		disk_write(block, &node);
		return 0;
	}

	int prev_block, curr_block;
	struct tfs_node prev, curr;

	curr_block = node.pointer;
	disk_read(curr_block, &curr);
	if (child_type <= curr.type && strcmp(child_name, curr.name) < 0) {
		node.pointer = child_block;
		node.size++;
		disk_write(block, &node);
		return curr_block;
	}
	for (;;) {
		if (curr.next == 0) {
			curr.next = child_block;
			disk_write(curr_block, &curr);
			node.size++;
			disk_write(block, &node);
			return 0;
		}

		prev_block = curr_block;
		memcpy(&prev, &curr, 512);

		curr_block = curr.next;
		disk_read(curr_block, &curr);
		if (child_type <= curr.type && strcmp(child_name, curr.name) < 0) {
			prev.next = child_block;
			disk_write(prev_block, &prev);
			node.size++;
			disk_write(block, &node);
			return curr_block;
		}
	}
}

void tfs_child_remove(uint32_t block, uint32_t child_block) {
	struct tfs_node node;
	disk_read(block, &node);
	
	if (node.type != TFS_DIRECTORY) {
		printf("Error: attempted to remove a child from a non-directory.\n");
		return;
	}

	if (node.pointer == 0)
		return;

	uint32_t prev_block, curr_block;
	struct tfs_node prev, curr;

	curr_block = node.pointer;
	disk_read(curr_block, &curr);
	if (curr_block == child_block) {
		node.pointer = curr.next;
		node.size--;
		disk_write(block, &node);
		return;
	}
	for (;;) {
		if (curr.next == 0)
			return;

		prev_block = curr_block;
		memcpy(&prev, &curr, 512);

		curr_block = curr.next;
		disk_read(curr_block, &curr);
		if (curr_block == child_block) {
			prev.next = curr.next;
			disk_write(prev_block, &prev);
			node.size--;
			disk_write(block, &node);
			return;
		}
	}
}

/*
 * Calls
 */
int tfs_getnode(const char *path, struct tfs_node *out) {
	int path_offset = 0;
	char name[TFS_NAME_LENGTH];

	int block = 2048;
	struct tfs_node node;

	for (;;) {
		for (int i = 0; i < TFS_NAME_LENGTH; i++) {
			if (path[path_offset + i] == '/' || path[path_offset + i] == 0) {
				path_offset += i;
				name[i] = 0;
				break;
			}
			name[i] = path[path_offset + i];
		}

		for (;;) {
			disk_read(block, &node);
			if (strcmp(name, node.name) == 0)
				break;
			if (node.next == 0)
				return -1;
			block = node.next;
		}

		if (path[path_offset] == 0) {
			if (out != NULL)
				memcpy(out, &node, 512);
			return block;
		}

		path_offset++;
		if (node.type == TFS_DIRECTORY) {
			if (node.pointer == 0)
				return -1;
			block = node.pointer;
		} else {
			printf("Error: '%s' (%s) is not a directory.\n", name, path);
			return -1;
		}
	}
}

int tfs_mknode(const char *path, enum tfs_type type) {
	printf("$ mknode %s %d\n", path, type);
	if (tfs_getnode(path, NULL) != -1) {
		printf("Error: node '%s' already exists.\n", path);
		return -1;
	}

	char *parent_path = strdup(path);
	char *last_slash = strrchr(parent_path, '/');
	if (last_slash) *last_slash = 0;
	struct tfs_node parent;
	int parent_block = tfs_getnode(parent_path, &parent);
	if (parent_block == -1) {
		printf("Error: parent node '%s' doesn't exist.\n", parent_path);
		free(parent_path);
		return -1;
	}
	if (parent.type != TFS_DIRECTORY) {
		printf("Error: parent node '%s' is not a directory.\n", parent_path);
		free(parent_path);
		return -1;
	}
	free(parent_path);

	int block = tfs_bitmap_alloc(1);
	if (block == -1)
		return -1;
	struct tfs_node node = { 0 };
	node.type = type;
	node.pointer = 0;
	node.size = 0;
	node.time = get_milliseconds() * 65536;
	strcpy(node.name, strrchr(path, '/') + 1);
	node.parent = parent_block;
	node.next = tfs_child_add(parent_block, block, node.type, node.name);
	disk_write(block, &node);
	return block;
}

int tfs_rmnode(const char *path) {
	struct tfs_node node;
	int block = tfs_getnode(path, &node);
	if (block == -1) {
		printf("Error: node '%s' doesn't exist.\n", path);
		return -1;
	} else if (block == 2048) {
		printf("Error: can't remove the root node.\n");
		return -1;
	}

	if (node.type == TFS_FILE) {
		tfs_bitmap_free(node.pointer, (node.size + 511) / 512);
	} else if (node.type == TFS_DIRECTORY) {
		if (node.size > 0) {
			printf("Error: directory '%s' is not empty.\n", path);
			return -1;
		}
	}

	tfs_child_remove(node.parent, block);
	tfs_bitmap_free(block, 1);
	return 0;
}

int tfs_write(const char *path, const void *buffer, uint64_t length) {
	struct tfs_node node;
	int block = tfs_getnode(path, &node);
	if (block == -1) {
		printf("Error: node '%s' doesn't exist.\n", path);
		return -1;
	} else if (node.type != TFS_FILE) {
		printf("Error: node '%s' is not a file.\n", path);
		return -1;
	}

	if (node.pointer != 0) {
		tfs_bitmap_free(node.pointer, (node.size + 511) / 512);
		node.pointer = 0;
	}

	int blocks = (length + 511) / 512;
	int pointer = tfs_bitmap_alloc(blocks);
	if (pointer == -1) {
		disk_write(block, &node);
		return -1;
	}

	node.pointer = pointer;
	node.size = length;
	node.time = get_milliseconds() * 65536;
	disk_write(block, &node);

	for (int i = 0; i < blocks; i++) {
		if (i == blocks - 1) {
			char disk_buffer[512] = { 0 };
			memcpy(disk_buffer, buffer + i * 512, length - i * 512);
			disk_write(node.pointer + i, disk_buffer);
		} else {
			disk_write(node.pointer + i, buffer + i * 512);
		}
	}

	return length;
}

int tfs_read(const char *path, void *buffer, uint64_t length) {
	struct tfs_node node;
	int block = tfs_getnode(path, &node);
	if (block == -1) {
		printf("Error: node '%s' doesn't exist.\n", path);
		return -1;
	} else if (node.type != TFS_FILE) {
		printf("Error: node '%s' is not a file.\n", path);
		return -1;
	}

	if (node.pointer == 0)
		return 0;
	if (length > node.size)
		length = node.size;

	int blocks = (length + 511) / 512;
	for (int i = 0; i < blocks; i++) {
		if (i == blocks - 1) {
			char disk_buffer[512] = { 0 };
			disk_read(node.pointer + i, disk_buffer);
			memcpy(buffer + i * 512, disk_buffer, length - i * 512);
		} else {
			disk_read(node.pointer + i, buffer + i * 512);
		}
	}

	return length;
}

/*
 * Debug
 */
void tfs_print_super() {
	struct tfs_super super;
	disk_read(0, &super);
	printf("\033[97mFilesystem superblock:\033[0m\n");
	printf("  total_blocks: %d\n", super.total_blocks);
	printf("  bitmap_blocks: %d\n", super.bitmap_blocks);
	printf("  bitmap_offset: %d\n\n", super.bitmap_offset);
}

void tfs_print_usage() {
	struct tfs_super super;
	disk_read(0, &super);

	uint8_t bitmap[512];
	int used_blocks = 0;
	for (uint32_t i = 0; i < super.bitmap_blocks; i++) {
		disk_read(super.total_blocks - super.bitmap_blocks + i, bitmap);
		for (int j = 0; j < 512; j++) {
			for (int k = 0; k < 8; k++) {
				if (bitmap[j] & (1 << k))
					used_blocks++;
			}
		}
	}

	printf("\033[97mFilesystem usage:\033[0m\n");
	printf("  total sectors: %d\n", super.total_blocks);
	printf("  used sectors: %d\n", used_blocks);
	printf("  used: %d%%\n\n", (used_blocks * 100) / super.total_blocks);
}

void tfs_print_child_node(int, char*, bool);
void tfs_print_node(int block, char *indent) {
	struct tfs_node node;
	disk_read(block, &node);

	printf(node.name);
	if (node.type == TFS_DIRECTORY) {
		printf("/\n");
		int child_block = node.pointer;
		struct tfs_node child;
		while (child_block != 0) {
			disk_read(child_block, &child);
			tfs_print_child_node(child_block, indent, child.next == 0);
			child_block = child.next;
		}
	} else {
		printf(" (%d)\n", node.size);
	}
}
void tfs_print_child_node(int block, char *indent, bool last) {
	printf("\033[90m%s", indent);
	char *id = malloc(strlen(indent) + 3);
	strcpy(id, indent);
	if (last) {
		printf("\\-");
		id[strlen(id)] = ' ';
		id[strlen(id)] = ' ';
		id[strlen(id)] = '\0';
	} else {
		printf("|-");
		id[strlen(id)] = '|';
		id[strlen(id)] = ' ';
		id[strlen(id)] = '\0';
	}
	printf("\033[0m");
	tfs_print_node(block, id);
	free(id);
}
void tfs_print_files() {
	printf("\033[97mFilesystem tree:\033[0m\n");
	tfs_print_node(2048, "");
	printf("\n");
}
