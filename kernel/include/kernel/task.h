#ifndef _TASK_H
#define _TASK_H 1

#include <stdint.h>

struct task {
	void* user_stack;
	void* kernel_stack;
	struct page_table* page_map;

	struct task *next;
};

static inline uint64_t get_tss_rsp0() {
	extern uint32_t tss;
	return (uint64_t) ((&tss)[2] << 32) | (&tss)[1];
}

void context_switch(struct task*, struct task*);

void task_init();
void task_add(struct task*);
void task_reschedule();

#endif
