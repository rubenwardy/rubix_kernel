#include "fs.h"
#include "disk.h"
#include "blocks.h"
#include "../utils.h"

typedef struct __attribute__((__packed__)) {
	char path[20];
	u32 block_num;
} INodeIndexEntry;
// sizeof(INodeIndexEntry) = 20 + 4 = 24

#define MAX_INODES 10
INodeIndexEntry inode_index[MAX_INODES];
bool _fs_index_modified;

void fs_init() {
	_fs_index_modified = false;

	fs_disk_init();
	fs_blocks_init();
	memset(&inode_index[0], 0, MAX_INODES * sizeof(INodeIndexEntry));
}

void _fs_writeSuperBlock() {
	u32 blockSize = fs_blocks_getBlockSize();
	size_t size = MAX_INODES * sizeof(INodeIndexEntry);
	if (size > blockSize) {
		printError("[Fs] INodeIndex does not fit into a single block!");
		return;
	}
	char mem[blockSize];
	memcpy(&mem[0], &inode_index[0], size);
	memset(&mem[size], 0, blockSize - size);
	fs_blocks_writeBlock(0, mem, NULL, NULL);
	_fs_index_modified = false;
}

void _fs_handle_readSuperBlock(u32 block_num, char *resp, void *meta) {
	if (resp[0] == '\0') {
		printError("[Fs] Initialising new file system");
		memcpy(inode_index[0].path, "a.txt", 6);
		inode_index[0].block_num = 1;
		_fs_index_modified = true;
		_fs_writeSuperBlock();
	} else {
		printError("[Fs] Found filesystem super block");
		_fs_index_modified = false;

		memcpy(&inode_index[0], resp, MAX_INODES * sizeof(INodeIndexEntry));
	}
}

void fs_on_disk_connected() {
	fs_blocks_readBlock(0, &_fs_handle_readSuperBlock, NULL);
}

INode *fs_get_inode(const char* path) {
	return NULL;
}
