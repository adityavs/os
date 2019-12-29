#include "kernel/interrupts.h"

#include <stdio.h>
#include <string.h>

#include "kernel/io.h"
#include "kernel/clock.h"
#include "kernel/tty.h"

struct idt_entry idt_entries[256] = { 0 };
struct idt_descriptor idt_descriptor;

isr_handler_t *isr_handlers[256] = { 0 };

void interrupts_init() {
	// Setup descriptor
	idt_descriptor.limit = (uint16_t) (256 * sizeof(struct idt_entry));
	idt_descriptor.base = (uint64_t) &idt_entries;
	__asm__ ("lidt %0" : : "m"(idt_descriptor));

	// Interrupt Service Routines
	extern void *isr_vector;
	for (int i = 0; i < 48; i++) {
		uint64_t base = (uint64_t) &isr_vector + 10 * i;
		idt_entries[i].base_low = base & 0xFFFF;
		idt_entries[i].base_mid = (base >> 16) & 0xFFFF;
		idt_entries[i].base_high = (base >> 32) & 0xFFFFFFFF;
		idt_entries[i].selector = 0x08;
		idt_entries[i].flags = 0x8E;
	}

	// Remap the PIC to 0x20..0x2F
	out8(0x20, 0x11);
	out8(0xA0, 0x11);
	out8(0x21, 0x20);
	out8(0xA1, 0x28);
	out8(0x21, 0x04);
	out8(0xA1, 0x02);
	out8(0x21, 0x01);
	out8(0xA1, 0x01);
	out8(0x21, 0x00);
	out8(0xA1, 0x00);
}

void interrupts_set_handler(int i, isr_handler_t *func) {
	isr_handlers[i] = func;
}

char *exception_messages[] = {
	"Integer Divide-by-Zero Exception",				// 0
	"Debug Exception",								// 1
	"Non-Maskable-Interrupt",						// 2
	"Breakpoint Exception (INT 3)",					// 3
	"Overflow Exception (INTO instruction)",		// 4
	"Bound-Range Exception (BOUND instruction)",	// 5
	"Invalid-Opcode Exception",						// 6
	"Device-Not-Available Exception",				// 7
	"Double-Fault Exception",						// 8
	"Coprocessor-Segment-Overrun Exception",		// 9
	"Invalid-TSS Exception",						// 10
	"Segment-Not-Present Exception",				// 11
	"Stack Exception",								// 12
	"General-Protection Exception",					// 13
	"Page-Fault Exception",							// 14
	"(Reserved)",									// 15
	"x87 Floating-Point Exception",					// 16
	"Alignment-Check Exception",					// 17
	"Machine-Check Exception",						// 18
	"SIMD Floating-Point Exception",				// 19
	"Unknown Exception",							// 20
	"Unknown Exception",							// 21
	"Unknown Exception",							// 22
	"Unknown Exception",							// 23
	"Unknown Exception",							// 24
	"Unknown Exception",							// 25
	"Unknown Exception",							// 26
	"Unknown Exception",							// 27
	"Unknown Exception",							// 28
	"Unknown Exception",							// 29
	"Unknown Exception",							// 30
	"Unknown Exception",							// 31
};

void isr_handler(struct isr_stack s) {
	if (s.interrupt >= 32) {
		if (s.interrupt >= 40)
			out8(0xA0, 0x20);
		out8(0x20, 0x20);

		isr_handler_t *handler = isr_handlers[s.interrupt];
		if (handler != 0)
			handler(s);
		else
			printf("\033[97m* \033[0mUnhandled interrupt (0x%x).\n", s.interrupt);
	} else {
		struct time t;
		get_time(&t);

		printf("\n\033[91mKERNEL PANIC\033[0m\n");
		printf("\033[97m* \033[0m%02d:%02d:%02d, %d milliseconds since boot.\n", t.hour,
				t.minute, t.second, get_milliseconds());
		printf("\033[97m* \033[0m%s (INT:%d ERR:%d)\n", exception_messages[s.interrupt],
				s.interrupt, s.error_code);
		printf("\033[97m* \033[0mContext:\n");
		printf("    RAX:%16x RDI:%16x CR0:%16x\n", s.rax, s.rdi, s.cr0);
		printf("    RBX:%16x RSI:%16x CR2:%16x\n", s.rbx, s.rsi, s.cr2);
		printf("    RCX:%16x RSP:%16x CR3:%16x\n", s.rcx, s.rsp, s.cr3);
		printf("    RDX:%16x RBP:%16x CR4:%16x\n", s.rdx, s.rbp, s.cr4);
		printf("    RIP:%16x  CS:%6x SS:%6x FLG:%16x\n", s.rip, s.cs, s.ss, s.flags);
		printf("\033[97m* \033[0mSystem halted.\n");

		asm ("cli");
		for (;;)
			asm ("hlt");
	}
}
