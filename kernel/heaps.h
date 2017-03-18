#ifndef HEAPS_H
#define HEAPS_H

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

extern void heaps_init(MaxHeap *x);
extern void heaps_insert(MaxHeap *x, int weight, pid_t pid);
extern bool heaps_remove(MaxHeap *x, pid_t pid);
extern MaxHeapItem heaps_extractMax(MaxHeap *x);
extern void heaps_increaseAll(MaxHeap *x, int v);
extern void heaps_print(MaxHeap *x);


#endif
