#include "console.h"

/* The following functions are special-case versions of a) writing,
* and b) reading a string from the UART (the latter case returning
* once a carriage return character has been read, or an overall
* limit reached).
*/

void puts(char* x) {
	write(STDOUT_FILENO, x, strlen(x));
}

int gets(char* x, int n) {
	int i = 0;
	while (i < n) {
		int amt_read = read(STDIN_FILENO, &x[i], n - i);
		i += amt_read;
		if (amt_read == 0 || x[i - 1] == '\r') {
			x[i] = '\0';
			break;
		}
	}
	return i;
}

/* The behaviour of the console process can be summarised as an
* (infinite) loop over three main steps, namely
*
* 1. write a command prompt then read a command,
* 2. split the command into space-separated tokens using strtok,
* 3. execute whatever steps the command dictates.
*/

void main_console() {
	char x[1024];

	puts("#### CONSOLE ####\n");

	while (1) {
		puts("shell$ ");
		int n = gets(x, 1024);
		x[n] = '\0';

		puts(x);

		char *p = strtok(x, " ");

		if (strcmp(p, "fork") == 0) {
			pid_t pid = fork();

			if (pid == 0) {
				const char *addr = "p4";
				exec((char*)addr);
			}
		} else if (strcmp(p, "kill") == 0) {
			pid_t pid = atoi(strtok(NULL, " "));
			int   s   = atoi(strtok(NULL, " "));

			kill( pid, s );
		} else {
			puts("unknown command\n");
		}
	}

	exit(EXIT_SUCCESS);
}
