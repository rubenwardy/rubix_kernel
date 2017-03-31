#ifndef MAXHEAP_H
#define MAXHEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "hilevel.h"

struct MaxHeapItem {
	pid_t pid;
	int weight;
};
typedef struct MaxHeapItem MaxHeapItem;

struct MaxHeap {
	MaxHeapItem items[MAX_PROCESSES];
	int num_items;
};
typedef struct MaxHeap MaxHeap;

extern void maxheap_init(MaxHeap *x);
extern void maxheap_insert(MaxHeap *x, int weight, pid_t pid);
extern bool maxheap_remove(MaxHeap *x, pid_t pid);
extern MaxHeapItem maxheap_extractMax(MaxHeap *x);
extern void maxheap_increaseAll(MaxHeap *x, int v);
extern void maxheap_print(MaxHeap *x);


#endif
