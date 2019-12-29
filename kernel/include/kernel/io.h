#ifndef _KERNEL_IO_H
#define _KERNEL_IO_H 1

static inline void out8(uint16_t port, uint8_t val) {
	asm volatile ("outb %0, %1" : : "a" (val), "Nd" (port));
}

static inline void out16(uint16_t port, uint16_t val) {
	asm volatile ("outw %0, %1" : : "a" (val), "Nd" (port));
}

static inline void out32(uint16_t port, uint32_t val) {
	asm volatile ("outl %0, %1" : : "a" (val), "Nd" (port));
}

static inline void outs32(uint16_t port, uint64_t buffer, uint32_t count) {
	asm volatile ("cld; rep; outsl" : : "D" (buffer), "d" (port), "c" (count));
}

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

static inline uint64_t read_cr3() {
	uint64_t ret;
	asm volatile ("movq %%cr3, %0" : "=r" (ret));
	return ret;
}

#endif
