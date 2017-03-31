#include "scheduler.h"
#include "maxheap.h"

MaxHeap heap;

void scheduler_init() {
	maxheap_init(&heap);
}

pid_t scheduler_getNext() {
	maxheap_increaseAll(&heap, 1);
	maxheap_print(&heap);
	MaxHeapItem item = maxheap_extractMax(&heap);
	return item.pid;
}

void scheduler_add(pid_t pid, u32 priority) {
	maxheap_insert(&heap, priority, pid);
}

bool scheduler_remove(pid_t pid) {
	return maxheap_remove(&heap, pid);
}
