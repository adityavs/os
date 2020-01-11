#ifndef _TASK_H
#define _TASK_H 1

#include <stdbool.h>
#include <stdint.h>

enum task_status {
	TASK_RUNNING, TASK_READY, TASK_IOWAIT, TASK_DONE,
};

struct task {
	void* user_stack;
	void* kernel_stack;
	struct page_table* page_map;
	enum task_status status;
};

extern void context_switch(struct task*, struct task*);

void task_init();
struct task* task_new(void*);
int task_add(struct task*);

void task_yield();
void task_exit();

#endif
