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

size_t numberOfBlocks;
size_t blockSize;

int xstoi(const char *xs, size_t n) {
	int res = 0;
	for (int ptr = n - 1; ptr > 0; ptr -= 2) {
		res = res * 16 + xtoi(xs[ptr - 1]);
		res = res * 16 + xtoi(xs[ptr]);
	}
	return res;
}

void _fs_disk_query();
void fs_disk_init() {
	memset(&current_cmd, 0, sizeof(FsDiskCmd));
	memset(cmds, 0, sizeof(FsDiskCmd) * MAX_QUEUED_COMMANDS);
	num_commands     = 0;
	disk_buffer_size = 0;
	numberOfBlocks   = 0;
	blockSize        = 0;

	// clear bad bytes
	fs_disk_run_command("", NULL, NULL);

	_fs_disk_query();
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

void fs_disk_run_command(char *cmd, DiskCommandHandler handler, void *meta) {
	if (current_cmd.cmd == NULL) {
		// immediately execute command
		current_cmd.cmd     = cmd;
		current_cmd.handler = handler;
		current_cmd.data    = meta;
		_fs_disk_send_command();
	} else {
		printLine("[FsDisk] Queuing command");
		cmds[num_commands].cmd     = cmd;
		cmds[num_commands].handler = handler;
		cmds[num_commands].data    = meta;
		num_commands++;
	}
}

void _fs_on_received_msg(char *msg) {
	if (current_cmd.cmd) {
		printLine("[FsDisk] Received response, processing...");
		if (current_cmd.handler) {
			current_cmd.handler(msg, current_cmd.data);
		}
		current_cmd.cmd = NULL;
	} else {
		printError("[FsDisk] No current command!");
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

void _fs_disk_handle_query(char *resp, void *meta) {
	if (strcmp(resp, "01") == 0) {
		printError("[FsDisk] Error from disk ctr during disk query");
		return;
	}

	strtok(resp, " "); // discard
	char *data = strtok(NULL, " ");
	if (!data) {
		printError("[FsDisk] Error querying disk! Invalid response (no spaces)");
		return;
	}

	size_t len = strlen(data);
	if (len != 16) {
		printError("[FsDisk] Error querying disk! Invalid response (too short)");
		return;
	}

	numberOfBlocks = xstoi(data,     8);
	blockSize      = xstoi(&data[8], 8);

	kprint("[FsDisk] Received disk info (nblocks=");
	printNum(numberOfBlocks);
	kprint(", bsize=");
	printNum(blockSize);
	kprint(")\n");
}

void _fs_disk_query() {
	fs_disk_run_command("00", &_fs_disk_handle_query, NULL);
}
