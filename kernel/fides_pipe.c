#include "fides_pipe.h"
#include "utils.h"

#define PIPE_MAX_BUFFER 0x400
#define MAX_PIPES 3

typedef struct {
	size_t refs_read;
	size_t refs_write;
	char buffer[PIPE_MAX_BUFFER];
	size_t buffer_size;
} PipeBuffer;

PipeBuffer pipe_buffers[MAX_PIPES];

void fides_pipe_init() {
	memset(&pipe_buffers[0], 0, sizeof(PipeBuffer) * MAX_PIPES);
}

size_t fides_pipe_read (FiDes *node, char *data, size_t max) {
	PipeBuffer *pipe = &pipe_buffers[node->data];
	size_t to_read = min(pipe->buffer_size, max);
	memcpy(&data[0], &pipe->buffer[0], sizeof(char) * to_read);

	printf("[pipe] Reading from buffer ");
	printNum(node->data);
	printf(" size=");
	printNum(to_read);
	printf(" max-read=");
	printNum(max);
	printf(" buf-size=");
	printNum(pipe->buffer_size);
	printf("\n");

	if (pipe->buffer_size > to_read) {
		memcpy(&pipe->buffer[0], &pipe->buffer[to_read], sizeof(char) * (pipe->buffer_size - to_read));
		pipe->buffer_size -= to_read;
	} else {
		pipe->buffer_size = 0;
	}

	return to_read;
}

size_t fides_pipe_write(FiDes *node, const char *data, size_t len) {
	PipeBuffer *pipe = &pipe_buffers[node->data];
	size_t to_write = min(len, PIPE_MAX_BUFFER - pipe->buffer_size);

	memcpy(&pipe->buffer[pipe->buffer_size], &data[0], sizeof(char) * to_write);
	pipe->buffer_size += to_write;

	return to_write;
}

void fides_pipe_grab(FiDes *node) {
	if (node->write) {
		pipe_buffers[node->data].refs_write++;
	} else {
		pipe_buffers[node->data].refs_read++;
	}
}

void fides_pipe_drop(FiDes *node) {
	if (node->write) {
		pipe_buffers[node->data].refs_write--;
	} else {
		pipe_buffers[node->data].refs_read--;
	}

	if (pipe_buffers[node->data].refs_write + pipe_buffers[node->data].refs_read == 0) {
		printLine("Destroying pipe (no references left)");
		node->drop = 0;
		node->grab = 0;
	} else {
		printLine("Dropped fd to pipe, but references remain");
	}
}

void fides_pipe_create(FiDes *one, FiDes *two) {
	one->read  = &fides_pipe_read;
	one->grab  = &fides_pipe_grab;
	one->drop  = &fides_pipe_drop;
	two->write = &fides_pipe_write;
	two->grab  = &fides_pipe_grab;
	two->drop  = &fides_pipe_drop;

	size_t ptr = 0;
	while (pipe_buffers[ptr].refs_read + pipe_buffers[ptr].refs_write > 0) {
		ptr++;
		if (ptr >= MAX_PIPES) {
			printLine("Unable to create pipe, max reached!");
			return;
		}
	}

	one->data = ptr;
	two->data = ptr;
	pipe_buffers[ptr].refs_read = 1;
	pipe_buffers[ptr].refs_write = 1;
	// memset(&pipe_buffers[ptr].buffer[0], 0, sizeof(char) * PIPE_MAX_BUFFER);
}
