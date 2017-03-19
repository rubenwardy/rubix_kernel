#include "inode_pipe.h"

#define PIPE_MAX_BUFFER 0x400
#define MAX_PIPES 3

typedef struct {
	u32 inode_id;
	const char *buffer[PIPE_MAX_BUFFER];
	size_t buffer_size;
} PipeBuffer;

PipeBuffer pipe_buffers[MAX_PIPES];

void inode_pipe_init() {
	memset(&pipe_buffers[0], 0, sizeof(PipeBuffer) * MAX_PIPES);
}

size_t inode_pipe_read (INode *node, const char *data, size_t max) {
	return 0;
}

size_t inode_pipe_write(INode *node, const char *data, size_t len) {
	return len;
}

void inode_pipe_create(INode *one, INode *two) {
	one->perms =
			OPERATION_READ << 8 |
			OPERATION_READ << 4 |
			OPERATION_READ << 0;

	one->read = &inode_pipe_read;

	two->perms =
			OPERATION_WRITE << 8 |
			OPERATION_WRITE << 4 |
			OPERATION_WRITE << 0;

	two->write = &inode_pipe_write;
}
