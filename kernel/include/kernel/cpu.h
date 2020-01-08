#ifndef _KERNEL_CPU_H
#define _KERNEL_CPU_H 1

/*
 * Output
 */
static inline void out8(uint16_t port, uint8_t data) {
	asm volatile ("outb %0, %1" : : "a" (data), "Nd" (port));
}

static inline void out16(uint16_t port, uint16_t data) {
	asm volatile ("outw %0, %1" : : "a" (data), "Nd" (port));
}

static inline void out32(uint16_t port, uint32_t data) {
	asm volatile ("outl %0, %1" : : "a" (data), "Nd" (port));
}

static inline void outs32(uint16_t port, uint64_t buffer, uint32_t count) {
	asm volatile ("cld; rep; outsl" : : "D" (buffer), "d" (port), "c" (count));
}

/*
 * Input
 */
static inline uint8_t in8(uint16_t port) {
	uint8_t ret;
	asm volatile ("inb %1, %0" : "=a" (ret) : "Nd" (port));
	return ret;
}

static inline uint16_t in16(uint16_t port) {
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "Nd" (port));
	return ret;
}

static inline uint32_t in32(uint16_t port) {
	uint32_t ret;
	asm volatile ("inl %1, %0" : "=a" (ret) : "Nd" (port));
	return ret;
}

static inline void ins32(uint16_t port, uint64_t buffer, uint32_t count) {
	asm volatile ("cld; rep; insl" : : "D" (buffer), "d" (port), "c" (count));
}

/*
 * Helpers
 */
static inline void sysret(uint64_t rsp, uint64_t rcx) {
	asm volatile ("mov %0, %%rsp; rex.w sysret" : : "r" (rsp), "c" (rcx));
}

/*
 * Registers
 */
static inline void set_cr3(uint64_t cr3) {
	asm volatile ("mov %0, %%cr3" : : "r" (cr3));
}

static inline uint64_t get_cr3() {
	uint64_t cr3;
	asm volatile ("mov %%cr3, %0" : "=r" (cr3));
	return cr3;
}

static inline uint64_t get_rsp() {
	uint64_t rsp;
	asm volatile ("mov %%rsp, %0" : "=r" (rsp));
	return rsp;
}

/*
 * Model specific registers
 */
#define MSR_EFER 0xC0000080
#define MSR_STAR 0xC0000081
#define MSR_LSTAR 0xC0000082
#define MSR_CSTAR 0xC0000083
#define MSR_SFMASK 0xC0000084

static inline uint64_t rdmsr(uint64_t msr) {
	uint32_t low, high;
	asm volatile ("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));
	return ((uint64_t) high << 32) | low;
}

static inline void wrmsr(uint64_t msr, uint64_t data) {
	uint32_t low = data & 0xFFFFFFFF;
	uint32_t high = data >> 32;
	asm volatile ("wrmsr" : : "c" (msr), "a" (low), "d" (high));
}

#endif
