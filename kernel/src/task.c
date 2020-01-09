#include "kernel/task.h"

#include <stdio.h>
#include <stdlib.h>

#include "kernel/cpu.h"
#include "kernel/memory.h"
#include "kernel/panic.h"

uint64_t next_pid = 0;

struct task* run_queue;
struct task* delete_queue;

struct task* current_task;

void task_init() {
	struct task* task = (struct task*) malloc(sizeof(struct task));
	task->pid = next_pid++;
	task->user_stack = (void*) get_rsp();
	task->kernel_stack = NULL;
	task->page_map = (struct page_table*) get_cr3();
	task->next = NULL;

	run_queue = NULL;
	delete_queue = NULL;
	current_task = task;
}

struct task* task_new(void* entry) {
	struct task *task = (struct task*) malloc(sizeof(struct task));
	task->pid = next_pid++;
	printf("\033[90m* Creating new task (pid=%d, entry=0x%x\n\033[0m", task->pid, entry);
	task->page_map = virtual_new();
	task->kernel_stack = virtual_alloc(task->page_map, 1, 0) + PAGE_SIZE;
	task->user_stack = virtual_alloc(task->page_map, 1, 1) + PAGE_SIZE;
	uint64_t prev_cr3 = get_cr3();
	set_cr3((uint64_t) task->page_map);
	*((uint64_t*) (task->user_stack -= 8)) = (uint64_t) entry;	// rip
	*((uint64_t*) (task->user_stack -= 8)) = (uint64_t) 0;		// rbp
	*((uint64_t*) (task->user_stack -= 8)) = (uint64_t) 0;		// rflags
	set_cr3(prev_cr3);
	return task;
}

void task_delete(struct task* task) {
	if (task == current_task)
		panic("attempted to delete current task.");
	printf("\033[90m* Deleting task (pid=%d)\n\033[0m", task->pid);
	if (task->page_map != NULL) {
		if (task->user_stack != NULL)
			virtual_free(task->page_map, task->user_stack - PAGE_SIZE, 1);
		if (task->kernel_stack != NULL)
			virtual_free(task->page_map, task->kernel_stack - PAGE_SIZE, 1);
		virtual_delete(task->page_map);
	}
	if (task != NULL)
		free(task);
}

void task_schedule(struct task *task) {
//	printf("\033[90m* Scheduling task (pid=%d)\n\033[0m", task->pid);
	if (run_queue == NULL) {
		run_queue = task;
		run_queue->next = NULL;
		return;
	}
	struct task* iter = run_queue;
	while (iter->next != NULL)
		iter = iter->next;
	iter->next = task;
	task->next = NULL;
}

void task_switch(bool reschedule) {
	while (delete_queue != NULL) {
		struct task *next = delete_queue->next;
		task_delete(delete_queue);
		delete_queue = next;
	}
	if (run_queue == NULL)
		return;
	struct task* prev = current_task;
	current_task = run_queue;
	printf("\033[90m* Switching from task (pid=%d, %sreschedule) to task (pid=%d)\n\033[0m", prev->pid, reschedule ? "" : "no ", current_task->pid);
	run_queue = run_queue->next;
	if (reschedule) {
		task_schedule(prev);
	} else {
		prev->next = delete_queue;
		delete_queue = prev;
	}
	context_switch(prev, current_task);
}
