#include "heaps.h"

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

void heaps_insert(MaxHeap *x, int weight, pid_t data) {
	MaxHeapItem item;
	item.weight = -10000;
	item.data = data;
	x->items[x->num_items++] = item;
	increaseKey(x, x->num_items - 1, weight);
}

pid_t heaps_extractMax(MaxHeap *x) {
	if (x->num_items < 1) {
		return 0;
	}

	pid_t max = x->items[0].data;
	x->items[0] = x->items[x->num_items - 1];
	x->num_items--;
	maxHeapify(x, 1);
	return max;
}
