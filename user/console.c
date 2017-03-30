#include "console.h"
#include "stdio.h"

int gets(char* x, int n) {
	int i = 0;
	while (i < n) {
		int amt_read = read(STDIN_FILENO, &x[i], n - i);
		i += amt_read;
		if (amt_read == 0) {
			x[0] = '\0';
			break;
		} else if (x[i - 2] == '\r') {
			x[i - 2] = '\0';
			i -= 2;
			break;
		} else if (x[i - 1] == '\r' || x[i - 1] == '\n') {
			x[i - 1] = '\0';
			i -= 1;
			break;
		}

		// write(STDOUT_FILENO, value, amt_read);
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
	printf("\n");

	while (1) {
		printf("user1@pc $ ");
		char x[1024];
		int n = gets(x, 1024);
		x[n] = '\0';

		printf("\n");

		char *p = strtok(x, " ");

		if (strcmp(p, "fork") == 0) {
			const char *cmd_name = strtok(NULL, " ");
			const char *priority = strtok(NULL, " ");
			int pid = fork();
			if (pid == 0) {
				if (exec((char*)cmd_name) == -1) {
					exit(1);
				}
			} else if (pid == -1) {
				printf("Unable to fork to start process\n");
			} else if (priority != NULL) {
				printf("Set priority to %d\n", atoi(priority));
				setpriority(pid, atoi(priority));
			}
		} else if (strcmp(p, "kill") == 0) {
			pid_t pid = atoi(strtok(NULL, " "));
			int   s   = atoi(strtok(NULL, " "));

			kill( pid, s );
		} else {
			int pid = fork();
			if (pid == 0) {
				exec(p);
				exit(127);
			} else if (pid == -1) {
				printf("Unable to fork to start process\n");
			} else {
				int status = 0;
				waitpid(pid, &status);

				if (status == 127) {
					printf("%s: command not found\n", x);
				} else {
					printf("Program exited with code %d\n", status);
				}
			}
		}
	}

	exit(EXIT_SUCCESS);
}
