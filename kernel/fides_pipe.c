#include "fides_pipe.h"

#define PIPE_MAX_BUFFER 0x400
#define MAX_PIPES 3

typedef struct {
	size_t refs;
	const char *buffer[PIPE_MAX_BUFFER];
	size_t buffer_size;
} PipeBuffer;

PipeBuffer pipe_buffers[MAX_PIPES];

void fides_pipe_init() {
	memset(&pipe_buffers[0], 0, sizeof(PipeBuffer) * MAX_PIPES);
}

size_t fides_pipe_read (FiDes *node, const char *data, size_t max) {
	return 0;
}

size_t fides_pipe_write(FiDes *node, const char *data, size_t len) {
	return len;
}

void fides_pipe_grab(FiDes *node) {
	pipe_buffers[node->data].refs++;
}

void fides_pipe_drop(FiDes *node) {
	pipe_buffers[node->data].refs--;

	if (pipe_buffers[node->data].refs == 0) {
		node->drop = 0;
		node->grab = 0;
	}
}

void fides_pipe_create(FiDes *one, FiDes *two) {
	one->read = &fides_pipe_read;
	two->write = &fides_pipe_write;

	size_t ptr = 0;
	while (pipe_buffers[ptr].refs > 0) {
		ptr++;
		if (ptr >= MAX_PIPES) {
			printLine("Unable to create pipe, max reached!");
			return;
		}
	}

	one->data = ptr;
	two->data = ptr;
	pipe_buffers[ptr].refs = 1;
	// memset(&pipe_buffers[ptr].buffer[0], 0, sizeof(char) * PIPE_MAX_BUFFER);
}
