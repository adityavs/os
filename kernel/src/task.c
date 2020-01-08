#include "kernel/task.h"

#include <stdio.h>
#include <stdlib.h>

#include "kernel/cpu.h"

struct {
	struct task* head;
	struct task* tail;
} task_list;
struct task* current_task;

void task_init() {
	struct task* task = (struct task*) malloc(sizeof(struct task));
	task->user_stack = (void*) get_rsp();
	task->kernel_stack = NULL;
	task->page_map = (struct page_table*) get_cr3();
	task->next = task;

	task_list.head = task;
	task_list.tail = task;
	current_task = task;
}

void task_add(struct task *task) {
	task_list.tail->next = task;
	task_list.tail = task;
	task_list.tail->next = NULL;
}

void task_reschedule() {
	struct task *prev = current_task;
	current_task = current_task->next;
	if (current_task == NULL)
		current_task = task_list.head;
	context_switch(prev, current_task);
}
