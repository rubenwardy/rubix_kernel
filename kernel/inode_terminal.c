#include "inode_terminal.h"
#include "hilevel.h"

size_t inode_terminal_read(INode *node, const char *x1, size_t n) {
	char *x = (char*)x1;
	for (int i = 0; i < n; i++ ) {
		*x++ = PL011_getc(UART0, true);
	}
}

size_t inode_terminal_write(INode *node, const char *x1, size_t n) {
	char *x = (char*)x1;
	for (int i = 0; i < n; i++) {
		PL011_putc(UART0, *x++, true);
	}
}

void inode_terminal_create(INode *node) {
	node->read  = &inode_terminal_read;
	node->write = &inode_terminal_write;
	u8 p = OPERATION_WRITE | OPERATION_READ;
	node->perms = p << 8 |
				p << 4 |
				p << 0;
}
