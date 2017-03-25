#include "console.h"
#include "stdio.h"

int gets(char* x, int n) {
	int i = 0;
	while (i < n) {
		int amt_read = read(STDIN_FILENO, &x[i], n - i);
		i += amt_read;
		if (amt_read == 0 || x[i - 1] == '\r') {
			x[i - 1] = '\0';
			break;
		}

		// TODO: allow UART0 input so I don't have to print this
		write(STDOUT_FILENO, &x[i - amt_read], amt_read);
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
	printf("\n");

	while (1) {
		printf("user1@pc $ ");
		int n = gets(x, 1024);
		x[n] = '\0';

		printf("\n");

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
			printf("%s: command not found\n", x);
		}
	}

	exit(EXIT_SUCCESS);
}
