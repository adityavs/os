#ifndef _TASK_H
#define _TASK_H 1

#include <stdbool.h>
#include <stdint.h>

struct task {
	uint64_t pid;

	void* user_stack;
	void* kernel_stack;
	struct page_table* page_map;

	struct task* next;
};

void context_switch(struct task*, struct task*);

void task_init();
struct task* task_new(void*);
void task_schedule(struct task*);
void task_switch(bool);

#endif
