#include "scheduler.h"
#include "heaps.h"

MaxHeap heap;

void scheduler_init() {
	heaps_init(&heap);
}

pid_t scheduler_getNext() {
	heaps_increaseAll(&heap, 1);
	heaps_print(&heap);
	MaxHeapItem item = heaps_extractMax(&heap);
	return item.pid;
}

void scheduler_add(pid_t pid, u32 priority) {
	heaps_insert(&heap, priority, pid);
}

bool scheduler_remove(pid_t pid) {
	return heaps_remove(&heap, pid);
}
