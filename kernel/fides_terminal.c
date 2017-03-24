#include "fides_terminal.h"
#include "hilevel.h"
#include "utils.h"

#define INPUT_BUFFER_SIZE 1025
char input_buffer[INPUT_BUFFER_SIZE];
size_t read_size = 0;

size_t fides_terminal_read(FiDes *node, char *x, size_t n) {
	if (read_size == 0 && n > 0) {
		printLine("[termr] Unable to read");
		return SIZE_MAX;
	}
	printLine("[termr] Reading...");

	size_t to_read = min(read_size, n);
	if (to_read > 0) {
		printLine("[termr] reading...");
		memcpy(x, &input_buffer[0], to_read * sizeof(char));
		memcpy(&input_buffer[0], &input_buffer[to_read], INPUT_BUFFER_SIZE - to_read * sizeof(char));
		read_size -= to_read;
	} else {
		printLine("[termr] no read");
	}
	return to_read;
}

size_t fides_terminal_write(FiDes *node, const char *x1, size_t n) {
	char *x = (char*)x1;
	for (int i = 0; i < n; i++) {
		PL011_putc(UART1, *x++, true);
	}
}

size_t fides_terminal_write_err(FiDes *node, const char *x1, size_t n) {
	char *x = (char*)x1;
	for (int i = 0; i < n; i++) {
		PL011_putc(UART0, *x++, true);
	}
}

void fides_terminal_create(FiDes *in, FiDes *out, FiDes *err) {
	in->read   = &fides_terminal_read;
	out->write = &fides_terminal_write;
	err->write = &fides_terminal_write_err;
}

void fides_terminal_input(char c) {
	input_buffer[read_size] = c;
	read_size++;
}

int fides_terminal_is_terminal(FiDes *fides) {
	return fides->read == &fides_terminal_read || fides->write == &fides_terminal_write;
}
