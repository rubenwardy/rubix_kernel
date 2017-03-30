#include "fs.h"
#include "disk.h"
#include "../utils.h"

void fs_init() {
	fs_disk_init();
}

INode *fs_get_inode(const char* path) {
	return NULL;
}
