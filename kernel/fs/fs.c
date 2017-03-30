#include "fs.h"
#include "disk.h"
#include "../utils.h"

int xstoi(const char *xs, size_t n) {
	int res = 0;
	for (int ptr = n - 1; ptr > 0; ptr -= 2) {
		res = res * 16 + xtoi(xs[ptr - 1]);
		res = res * 16 + xtoi(xs[ptr]);
	}
	return res;
}

size_t numberOfBlocks;
size_t blockSize;

void _fs_handle_query(char *resp, void *meta) {
	if (strcmp(resp, "01") == 0) {
		printError("[FS] Error from disk ctr during disk query");
		return;
	}

	strtok(resp, " "); // discard
	char *data = strtok(NULL, " ");
	if (!data) {
		printError("[FS] Error querying disk! Invalid response (no spaces)");
		return;
	}

	size_t len = strlen(data);
	if (len != 16) {
		printError("[FS] Error querying disk! Invalid response (too short)");
		return;
	}

	numberOfBlocks = xstoi(data,     8);
	blockSize      = xstoi(&data[8], 8);

	kprint("[FS] Received disk info (nblocks=");
	printNum(numberOfBlocks);
	kprint(", bsize=");
	printNum(blockSize);
	kprint(")\n");
}

void fs_init() {
	fs_disk_init();

	numberOfBlocks = 0;
	blockSize = 0;

	// clear bad bytes
	fs_disk_run_command("", NULL, NULL);

	fs_disk_run_command("00", &_fs_handle_query, NULL);
}

INode *fs_get_inode(const char* path) {
	return NULL;
}
