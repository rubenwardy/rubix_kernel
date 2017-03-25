#include "stdio.h"
#include <stdarg.h>
#include <string.h>
#include "libc.h"

void printf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char buffer[1024];
	size_t output_length = 0;

	size_t len = strlen(fmt);
	for (size_t i = 0; i < len; i++) {
		char c = fmt[i];
		if (c == '%') {
			i++;
			if (i >= len) {
				char m[] = "printf: unexpected eos after var";
				write(STDERR_FILENO, m, strlen(m));
				va_end(args);
				return;
			}
			c = fmt[i];

			if (c == 's') {
				char *str = va_arg(args, char*);
				size_t ptr = 0;
				while (str[ptr] != '\0' && output_length < 1024) {
					buffer[output_length++] = str[ptr++];
				}
			} else if (c == 'd' || c == 'i') {
				int x = va_arg(args, int);
				char str[100];
				itoa(&str[0], x);
				size_t ptr = 0;
				while (str[ptr] != '\0' && output_length < 1024) {
					buffer[output_length++] = str[ptr++];
				}
			} else {
				char m[] = "printf: unrecognised print type var";
				write(STDERR_FILENO, m, strlen(m));
				va_end(args);
				return;
			}
		} else if (output_length < 1024) {
			buffer[output_length++] = c;
		}
	}

	va_end(args);

	write(STDOUT_FILENO, buffer, output_length);

	if (output_length > 1024) {
		char m[] = "printf: buffer overflow";
		write(STDOUT_FILENO, m, strlen(m));
	}
}
