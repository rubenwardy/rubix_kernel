#include "heaps.h"
#include "utils.h"

size_t parent(size_t i) {
	return i / 2;
}

size_t left(size_t i) {
	return 2 * i;
}

size_t right(size_t i) {
	return 2 * i + 1;
}

#define A(v) x->items[(v)].weight

void maxHeapify(MaxHeap *x, size_t i) {
	size_t l = left(i);
	size_t r = right(i);

	size_t largest = i;
	if (l < x->num_items && A(l) > A(i)) {
		largest = l;
	}

	if (r < x->num_items && A(r) > A(largest)) {
		largest = r;
	}

	if (largest != i) {
		MaxHeapItem tmp = x->items[largest];
		x->items[largest] = x->items[i];
		x->items[i] = tmp;
		maxHeapify(x, largest);
	}
}

void increaseKey(MaxHeap *x, size_t i, int weight) {
	if (weight < x->items[i].weight) {
		return;
	}

	x->items[i].weight = weight;
	while (i > 0 && x->items[parent(i)].weight < x->items[i].weight) {
		MaxHeapItem tmp = x->items[parent(i)];
		x->items[parent(i)] = x->items[i];
		x->items[i] = tmp;
		i = parent(i);
	}
}

void heaps_init(MaxHeap *x) {
	x->num_items = 0;
}

void heaps_insert(MaxHeap *x, int weight, pid_t pid) {
	MaxHeapItem item;
	item.weight = -10000;
	item.pid = pid;
	x->items[x->num_items++] = item;
	increaseKey(x, x->num_items - 1, weight);
}

bool heaps_remove(MaxHeap *x, pid_t pid) {
	for (size_t i = 0; i < x->num_items; i++) {
		if (x->items[i].pid == pid) {
			x->num_items--;
			if (i < x->num_items - 1) {
				x->items[i] = x->items[x->num_items];
				maxHeapify(x, i);
			}
			return true;
		}
	}

	return false;
}

MaxHeapItem heaps_extractMax(MaxHeap *x) {
	if (x->num_items < 1) {
		MaxHeapItem item = { 0, 0 };
		return item;
	}

	MaxHeapItem max = x->items[0];
	x->items[0] = x->items[x->num_items - 1];
	x->num_items--;
	maxHeapify(x, 0);
	return max;
}

void heaps_increaseAll(MaxHeap *x, int v) {
	for (int i = 0; i < x->num_items; i++) {
		x->items[i].weight += v;
	}
}

void heaps_print(MaxHeap *x) {
	for (int i = 0; i < x->num_items; i++) {
		printNum(x->items[i].pid);
		printf("(");
		printNum(x->items[i].weight);
		printf(") ");
	}
	printf("\n");
}
