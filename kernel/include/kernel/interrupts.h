#ifndef _KERNEL_INTERRUPTS_H
#define _KERNEL_INTERRUPTS_H 1

#include <stdint.h>

struct idt_entry {
	uint16_t base_low;
	uint16_t selector;
	uint8_t ist;
	uint8_t flags;
	uint16_t base_mid;
	uint32_t base_high;
	uint32_t zero;
} __attribute__ ((packed));

struct idt_descriptor {
	uint16_t limit;
	uint64_t base;
} __attribute__ ((packed));

struct isr_stack {
	uint64_t cr0, cr2, cr3, cr4;
	uint64_t rdi, rsi, rbp;
	uint64_t rax, rbx, rcx, rdx;
	uint64_t interrupt, error_code;
	uint64_t rip, cs, flags, rsp, ss;
} __attribute__ ((packed));

typedef void isr_handler_t(struct isr_stack);

void interrupts_init();
void interrupts_set_handler(int, isr_handler_t*);

#endif
