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

#define MAX_INODE_FETCH_OPERATIONS 20

typedef struct {
	u32 block_num;
	INodeFetchCallback callback;
	void *meta;
} INodeFetchOperation;
INodeFetchOperation inode_fetch_operations[MAX_INODE_FETCH_OPERATIONS];

void fs_init() {
	_fs_index_modified = false;

	for (size_t i = 0; i < MAX_INODE_FETCH_OPERATIONS; i++) {
		inode_fetch_operations[i].block_num = SIZE_MAX;
	}

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

void _fs_writeINode(u32 block_num, INode *inode) {
	u32 blockSize = fs_blocks_getBlockSize();
	size_t size = sizeof(INode);
	if (size > blockSize) {
		printError("[Fs] INode does not fit into a single block!");
		return;
	}
	char mem[blockSize];
	memcpy(&mem[0], inode, size);
	memset(&mem[size], 0, blockSize - size);
	fs_blocks_writeBlock(block_num, mem, NULL, NULL);
	_fs_index_modified = false;
}

void _fs_handle_readSuperBlock(u32 block_num, char *resp, void *meta) {
	if (resp[0] == '\0') {
		printError("[Fs] Initialising new file system");
		memcpy(inode_index[0].path, "a.txt", 6);
		inode_index[0].block_num = 1;
		_fs_index_modified = true;
		_fs_writeSuperBlock();

		INode inode;
		inode.id        = 1;
		inode.user_id   = 1;
		inode.group_id  = 1;
		inode.block_num = 2;
		inode.size      = 12;
		u8 p = INODE_PERM_READ | INODE_PERM_WRITE;
		inode_set_perms(&inode, p, p, p);

		_fs_writeINode(1, &inode);

		u32 blockSize = fs_blocks_getBlockSize();
		char mem[blockSize];
		char *dat = "File content";
		memcpy(&mem[0], dat, strlen(dat) * sizeof(char));
		memset(&mem[strlen(dat)], 0, blockSize - 12);
		fs_blocks_writeBlock(2, mem, NULL, NULL);
	} else {
		printError("[Fs] Found filesystem super block");
		_fs_index_modified = false;

		memcpy(&inode_index[0], resp, MAX_INODES * sizeof(INodeIndexEntry));
	}

	fs_fetchINode("a.txt", NULL, NULL);
}

void fs_on_disk_connected() {
	fs_blocks_readBlock(0, &_fs_handle_readSuperBlock, NULL);
}

void _fs_handle_readINode(u32 block_num, char *resp, void *meta) {
	if (!meta) {
		printError("[Fs] Unable to handle readINode as no meta was passed");
	}

	INode *inode = (INode*)resp;
	INodeFetchOperation *rmeta = (INodeFetchOperation*)meta;
	if (rmeta->callback) {
		rmeta->callback(inode, rmeta->meta);
	}
	rmeta->block_num = SIZE_MAX;
}

INodeFetchOperation *_fs_allocateINodeFetchOperation(u32 block_num) {
	for (size_t i = 0; i < MAX_INODE_FETCH_OPERATIONS; i++) {
		if (inode_fetch_operations[i].block_num == SIZE_MAX) {
			inode_fetch_operations[i].block_num = block_num;
			return &inode_fetch_operations[i];
		}
	}

	printError("[Fs] Out of inode fetch operation meta space!");

	return NULL;
}

bool fs_fetchINode(const char *path, INodeFetchCallback callback, void *meta) {
	for (int i = 0; i < MAX_INODES; i++) {
		if (strcmp(inode_index[i].path, path) == 0) {
			INodeFetchOperation *rmeta = _fs_allocateINodeFetchOperation(inode_index[i].block_num);
			if (!rmeta) {
				printError("[Fs] Unable to read as allocateINodeFetchOperation returned NULL");
				return false;
			}
			rmeta->callback = callback;
			rmeta->meta = meta;

			fs_blocks_readBlock(inode_index[i].block_num, &_fs_handle_readINode, (void*)rmeta);
			return true;
		}
	}
	return false;
}
