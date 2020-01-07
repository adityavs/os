#include "kernel/elf.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "kernel/tfs.h"

uint64_t elf_load(struct page_table *p4, const char *path) {
	struct tfs_node node;
	if (tfs_getnode(path, &node) == -1) {
		printf("Couldn't open '%s'\n", path);
		return 0;
	}

	char *buffer = (char*) malloc(node.size);
	tfs_read(path, buffer, node.size);

	struct elf_header *ehdr = (struct elf_header*) buffer;
	struct elf_program_header *phdr = (struct elf_program_header*)
		((size_t) ehdr + ehdr->phoff);
	for (size_t i = 0; i < ehdr->phnum; i++) {
		struct elf_program_header *program = &phdr[i];
		if (program->type == PT_LOAD) {
			virtual_alloc_to(p4, (program->vaddr / PAGE_SIZE) * PAGE_SIZE,
					(program->memsz + PAGE_SIZE - 1) / PAGE_SIZE, 1);
			if (program->filesz)
				memcpy((void*) program->vaddr, buffer + program->offset, program->filesz);
		}
	}

	free(buffer);
	return ehdr->entry;
}
