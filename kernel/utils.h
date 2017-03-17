#ifndef UTILS_H
#define UTILS_H
#include "PL011.h"

void printf(const char *cs) {
	size_t ptr = 0;
	while (cs[ptr] != '\0') {
		PL011_putc(UART0, cs[ptr], true);
		ptr++;
	}
}

void printLine(const char *cs) {
	printf(cs);
	printf("\n\0");
}

void printNum(int num) {
	if (num == 0) {
		PL011_putc(UART0, '0', true);
	} else {
		int next = num / 10;
		if (next != 0) {
			printNum(next);
		}
		int digit = num % 10;
		PL011_putc(UART0, digit + '0', true);
	}
}

#endif
