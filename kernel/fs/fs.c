#include "fs.h"
#include "disk.h"
#include "../utils.h"

void _fs_handle_query(char *resp, void *data) {
	printLine("Received query!");
}

void fs_init() {
	fs_disk_init();

	// clear bad bytes
	fs_disk_run_command("", NULL, NULL);

	fs_disk_run_command("00", &_fs_handle_query, NULL);
}

INode *fs_get_inode(const char* path) {
	return NULL;
}
