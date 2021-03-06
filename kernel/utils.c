#include "utils.h"
#include "PL011.h"

size_t kprint(const char *cs) {
	size_t ptr = 0;
	while (cs[ptr] != '\0') {
		if (cs[ptr] != '\r') {
			PL011_putc(UART0, cs[ptr], true);
		}
		ptr++;
	}
	return ptr;
}


extern void kprintmod(const char *mod) {
	kprint("[");
	size_t res = kprint(mod);

	size_t spaces = 0;
	if (res <= 10) {
		spaces = 10 - res;
	}

	for (int i = 0; i < spaces; i++) {
		kprint(" ");
	}
	kprint("]  ");
}

void printLine(const char *mod, const char *cs) {
	kprintmod(mod);
	kprint(cs);
	kprint("\n");
}

void printError(const char *mod, const char *cs) {
	kprint("\e[31m");
	kprintmod(mod);
	kprint(cs);
	kprint("\e[0m\n");
}

void printNum2(int num) {
	int digit = num % 10;
	int next = num / 10;
	if (next > 0) {
		printNum2(next);
	}
	PL011_putc(UART0, digit + '0', true);
}

void printNum(int num) {
	if (num < 0) {
		PL011_putc(UART0, '-', true);
		num *= -1;
	}

	if (num == 0) {
		PL011_putc(UART0, '0', true);
	} else {
		printNum2(num);
	}
}
