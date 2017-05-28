#include "disk.h"
#include "../../hilevel.h"
#include "../../utils.h"

#define MAX_QUEUED_COMMANDS 10
#define MAX_CMD 550

typedef struct {
	char cmd[MAX_CMD];
	DiskCommandHandler handler;
	void *data;
} FsDiskCmd;

size_t num_commands;
FsDiskCmd cmds[MAX_QUEUED_COMMANDS];
FsDiskCmd current_cmd;

char disk_msg_buffer[2048];
size_t disk_buffer_size;

int xstoi(const char *xs, size_t n) {
	int res = 0;
	for (int ptr = n - 1; ptr > 0; ptr -= 2) {
		res = res * 16 + xtoi(xs[ptr - 1]);
		res = res * 16 + xtoi(xs[ptr]);
	}
	return res;
}

void fs_disk_init() {
	memset(&current_cmd, 0, sizeof(FsDiskCmd));
	memset(cmds, 0, sizeof(FsDiskCmd) * MAX_QUEUED_COMMANDS);
	num_commands     = 0;
	disk_buffer_size = 0;

	// clear bad bytes
	fs_disk_run_command("", NULL, NULL);
}

void _fs_disk_send_command() {
	printLine("fs:disk", "Sending command");

	size_t ptr = 0;
	while (current_cmd.cmd[ptr] != '\0') {
		PL011_putc(UART2, current_cmd.cmd[ptr++], true);
	}
	PL011_putc(UART2, '\n', true);
}

void _fs_disk_start_from_queue() {
	if (cmds[0].cmd[0] != '\0') {
		printLine("fs:disk", "Popping command from queue");

		memcpy(current_cmd.cmd, cmds[0].cmd, MAX_CMD * sizeof(char));
		current_cmd.handler = cmds[0].handler;
		current_cmd.data    = cmds[0].data;
		_fs_disk_send_command();

		for (size_t i = 1; i < num_commands; i++) {
			memcpy(&cmds[i-1], &cmds[i], sizeof(FsDiskCmd));
		}
		num_commands--;
		cmds[num_commands].cmd[0] = '\0';
	}
}

void fs_disk_run_command(char *cmd, DiskCommandHandler handler, void *meta) {
	if (cmd[0] == '\0') {
		cmd = " ";
	}
	if (current_cmd.cmd[0] == '\0') {
		// immediately execute command
		memcpy(&current_cmd.cmd[0], cmd, strlen(cmd));
		current_cmd.cmd[strlen(cmd)] = '\0';
		current_cmd.handler = handler;
		current_cmd.data    = meta;
		_fs_disk_send_command();
	} else {
		printLine("fs:disk", "Queuing command");

		memcpy(&cmds[num_commands].cmd[0], cmd, strlen(cmd) * sizeof(char));
		cmds[num_commands].cmd[strlen(cmd)] = '\0';
		cmds[num_commands].handler = handler;
		cmds[num_commands].data    = meta;
		num_commands++;
	}
}

void _fs_on_received_msg(char *msg) {
	if (current_cmd.cmd[0] != '\0') {
		printLine("fs:disk", "Received response, processing...");
		if (current_cmd.handler) {
			current_cmd.handler(msg, current_cmd.data);
		}
		current_cmd.cmd[0] = '\0';
	} else {
		printError("fs:disk", "No current command!");
	}
	_fs_disk_start_from_queue();
}

void fs_disk_on_interrupt() {
	while (PL011_can_getc(UART2)) {
		char c = PL011_getc(UART2, true);
		if (c == '\r' || c == '\n') {
			if (disk_buffer_size > 0) {
				disk_msg_buffer[disk_buffer_size++] = '\0';
				_fs_on_received_msg(disk_msg_buffer);
				disk_buffer_size = 0;
			}
		} else {
			disk_msg_buffer[disk_buffer_size++] = c;
		}
	}
}
