#include "kernel/task.h"

#include <stdio.h>
#include <stdlib.h>

#include "kernel/cpu.h"
#include "kernel/memory.h"
#include "kernel/panic.h"

struct {
	int size;
	int capacity;
	struct task **tasks;
} task_list;
int current_task;

void task_init() {
	task_list.size = 0;
	task_list.capacity = 8;
	task_list.tasks = (struct task**) calloc(8 * sizeof(struct task*));

	struct task* task = (struct task*) malloc(sizeof(struct task));
	task->user_stack = (void*) get_rsp();
	task->kernel_stack = NULL;
	task->page_map = (struct page_table*) get_cr3();
	task->status = TASK_RUNNING;
	current_task = task_add(task);
}

struct task* task_new(void* entry) {
	struct task *task = (struct task*) malloc(sizeof(struct task));
	task->page_map = virtual_new();
	task->kernel_stack = virtual_alloc(task->page_map, 1, 0) + PAGE_SIZE;
	task->user_stack = virtual_alloc(task->page_map, 1, 1) + PAGE_SIZE;
	uint64_t prev_cr3 = get_cr3();
	set_cr3((uint64_t) task->page_map);
	*((uint64_t*) (task->user_stack -= 8)) = (uint64_t) entry;	// rip
	*((uint64_t*) (task->user_stack -= 8)) = (uint64_t) 0;		// rbp
	*((uint64_t*) (task->user_stack -= 8)) = (uint64_t) 0;		// rflags
	set_cr3(prev_cr3);
	task->status = TASK_READY;
	return task;
}

int task_add(struct task *task) {
	for (int pid = 0; pid < task_list.capacity; pid++) {
		if (task_list.tasks[pid] == NULL) {
			task_list.tasks[pid] = task;
			return pid;
		}
	}
	panic("TODO: make task vector reallocatable\n");
	return -1;
}

void task_yield() {
	int next_task = -1;
	for (int i = 1; i < task_list.capacity; i++) {
		int pid = (current_task + i) % task_list.capacity;
		struct task* task = task_list.tasks[pid];
		if (task == NULL)
			continue;
		if (task->status == TASK_READY) {
			next_task = pid;
			break;
		} else if (task->status == TASK_DONE) {
			if (task->page_map != NULL) {
				if (task->user_stack != NULL)
					virtual_free(task->page_map, task->user_stack - PAGE_SIZE, 1);
				if (task->kernel_stack != NULL)
					virtual_free(task->page_map, task->kernel_stack - PAGE_SIZE, 1);
				virtual_delete(task->page_map);
			}
			free(task);
			task_list.tasks[pid] = NULL;
		}
	}
	if (next_task == -1)
		return;

	printf("\033[90m* Switching tasks %d -> %d\n\033[0m", current_task, next_task);
	struct task* curr = task_list.tasks[current_task];
	struct task* next = task_list.tasks[next_task];
	if (curr->status == TASK_RUNNING)
		curr->status = TASK_READY;
	next->status = TASK_RUNNING;
	current_task = next_task;
	context_switch(curr, next);
}

void task_exit() {
	printf("\033[90m* SIGEXIT from task %d\n\033[0m", current_task);
	task_list.tasks[current_task]->status = TASK_DONE;
	task_yield();
}
