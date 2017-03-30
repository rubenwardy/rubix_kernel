#include "disk.h"
#include "../hilevel.h"
#include "../utils.h"

#define MAX_QUEUED_COMMANDS 10

typedef struct {
	char *cmd;
	DiskCommandHandler handler;
	void *data;
} FsDiskCmd;

size_t num_commands;
FsDiskCmd cmds[MAX_QUEUED_COMMANDS];
FsDiskCmd current_cmd;

char disk_msg_buffer[100];
size_t disk_buffer_size;

void fs_disk_init() {
	memset(&current_cmd, 0, sizeof(FsDiskCmd));
	memset(cmds, 0, sizeof(FsDiskCmd) * MAX_QUEUED_COMMANDS);
	num_commands = 0;
	disk_buffer_size = 0;
}

void _fs_disk_send_command() {
	printLine("[FsDisk] Sending command");

	size_t ptr = 0;
	while (current_cmd.cmd[ptr] != '\0') {
		PL011_putc(UART2, current_cmd.cmd[ptr++], true);
	}
	PL011_putc(UART2, '\n', true);
}

void _fs_disk_start_from_queue() {
	if (cmds[0].cmd != NULL) {
		printLine("[FsDisk] Popping command from queue");

		current_cmd.cmd     = cmds[0].cmd;
		current_cmd.handler = cmds[0].handler;
		current_cmd.data    = cmds[0].data;
		_fs_disk_send_command();

		for (size_t i = 1; i < num_commands; i++) {
			memcpy(&cmds[i-1], &cmds[i], sizeof(FsDiskCmd));
		}
		num_commands--;
		cmds[num_commands].cmd = NULL;
	}
}

void fs_disk_run_command(char *cmd, DiskCommandHandler handler, void *data) {
	if (current_cmd.cmd == NULL) {
		// immediately execute command
		current_cmd.cmd     = cmd;
		current_cmd.handler = handler;
		current_cmd.data    = data;
		_fs_disk_send_command();
	} else {
		printLine("[FsDisk] Queuing command");
		cmds[num_commands].cmd     = cmd;
		cmds[num_commands].handler = handler;
		cmds[num_commands].data    = data;
		num_commands++;
	}
}

void _fs_on_received_msg(char *msg) {
	if (current_cmd.cmd) {
		printLine("Received response, processing...");
		if (current_cmd.handler) {
			current_cmd.handler(msg, current_cmd.data);
		}
		current_cmd.cmd = NULL;
	} else {
		printError("No current command!");
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
