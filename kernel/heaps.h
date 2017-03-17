#ifndef HEAPS_H
#define HEAPS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>

struct MaxHeapNode {
	struct MaxHeapNode *left;
	struct MaxHeapNode *right;
	void *data;
	int weight;
};
typedef struct MaxHeapNode MaxHeapNode;

typedef MaxHeapNode * MaxHeap;

void heap_set(MaxHeap heap, int weight, void *data) {
	
}

void maxHeapify(MaxHeapNode *x) {
	MaxHeapNode *left  = x->left;
	MaxHeapNode *right = x->right;

	int x_w = x->weight;
	int left_w = (left) ? left->weight : -1;
	int right_w = (right) ? right->weight : -1;

	if (left_w > x_w && left_w > right_w) {
		// right > left and root
		void *x_data = x->data;
		x->data = left->data;
		x->weight = left->weight;
		left->data = x_data;
		left->weight = x_w;
		maxHeapify(x->left);
	} else if (right_w > x_w) {
		// right > left and root
		void *x_data = x->data;
		int x_w = x->weight;
		x->data = right->data;
		x->weight = right->weight;
		right->data = x_data;
		right->weight = x_w;
		maxHeapify(x->right);
	} else {
		// root is biggest
	}
}


MaxHeapNode *insertIntoTree(MaxHeapNode *tree, MaxHeapNode *node) {

}

#endif
