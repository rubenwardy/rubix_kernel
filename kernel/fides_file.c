#include "fides_file.h"
#include "blockedqueue.h"
#include "utils.h"
#include "fs/fs.h"
#include "fs/blocks.h"

typedef struct {
	u32 id;
	INode inode;
	int ptr;
	char data[256];
} FidesFileEntry;

#define MAX_FIDES_FILES 10
FidesFileEntry _fides_files[MAX_FIDES_FILES];
int fides_files_count;

void fides_file_init() {
	fides_files_count = 0;
	memset(&_fides_files[0], 0, sizeof(FidesFileEntry) * MAX_FIDES_FILES);
}

FidesFileEntry *_fides_file_allocateEntry() {
	for (size_t i = 0; i < MAX_FIDES_FILES; i++) {
		if (_fides_files[i].id == 0) {
			_fides_files[i].id = ++fides_files_count;
			_fides_files[i].inode.id = 0;
			return &_fides_files[i];
		}
	}

	return NULL;
}

size_t fides_file_read(FiDes *node, char *data, size_t max) {
	return 0;
}

size_t fides_file_write(FiDes *node, const char *data, size_t len) {
	return 0;
}

void fides_file_grab(FiDes *node) {
	// TODO: unimplemented
}

void fides_file_drop(FiDes *node) {
	// TODO: unimplemented
}

void _fides_file_handle_read_data(u32 block_num, char *resp, void *meta) {
	FidesFileEntry *entry = (FidesFileEntry*)meta;
	if (!entry) {
		printError("[FidesFile] Unable to get entry linked to inode! rdata");
		return;
	}

	memcpy(&entry->data[0], resp, fs_blocks_getBlockSize());
}

void _fides_file_handle_fetch_inode(INode *inode, void *meta) {
	FidesFileEntry *entry = (FidesFileEntry*)meta;
	if (!entry) {
		printError("[FidesFile] Unable to get entry linked to inode! rinode");
		return;
	}

	printError("Fetched INode!");

	memcpy(&entry->inode, inode, sizeof(INode));

	fs_blocks_readBlock(inode->block_num, &_fides_file_handle_read_data, meta);
}

void fides_file_create(FiDes *one, char *path, char mode) {
	if (mode == 'r') {
		one->read  = &fides_file_read;
	} else {
		one->write = &fides_file_write;
	}
	one->grab  = &fides_file_grab;
	one->drop  = &fides_file_drop;

	FidesFileEntry *entry = _fides_file_allocateEntry();
	one->data = entry->id;

	fs_fetchINode(path, &_fides_file_handle_fetch_inode, (void*)entry);
}

int fides_file_is_file(FiDes *fides) {
	return fides->read == &fides_file_read || fides->write == &fides_file_write;
}
