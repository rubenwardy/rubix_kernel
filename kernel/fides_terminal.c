#include "fides_terminal.h"
#include "hilevel.h"

size_t fides_terminal_read(FiDes *node, const char *x1, size_t n) {
	char *x = (char*)x1;
	for (int i = 0; i < n; i++ ) {
		*x++ = PL011_getc(UART0, true);
	}
}

size_t fides_terminal_write(FiDes *node, const char *x1, size_t n) {
	char *x = (char*)x1;
	for (int i = 0; i < n; i++) {
		PL011_putc(UART0, *x++, true);
	}
}

void fides_terminal_create(FiDes *out, FiDes *in) {
	in->read   = &fides_terminal_read;
	out->write = &fides_terminal_write;
}
