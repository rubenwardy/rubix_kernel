#include "blocks.h"
#include "disk.h"
#include "../utils.h"

size_t numberOfBlocks;
size_t blockSize;

#define BLOCK_CACHE_SIZE 1024
#define MAX_BLOCKS_IN_CACHE 64
char _fs_cache[BLOCK_CACHE_SIZE];

typedef struct {
	u32 block_num;
	bool loaded;
	bool modified;
} BlockCacheIndexEntry;

BlockCacheIndexEntry block_cache_index[MAX_BLOCKS_IN_CACHE];
size_t blocks_in_cache;

typedef void (*BulkBlockReadOperationCallback)(int n, u32 address_num[n], char *data[n]);

char *_fs_blocks_fetchFromCache(u32 block_num, BlockCacheIndexEntry *entry) {
	if (numberOfBlocks == 0 || blockSize == 0) {
		printError("[FsBlocks] Unable to write when disk hasn't been initialised yet!");
		return NULL;
	}

	size_t max_blocks_in_cache = BLOCK_CACHE_SIZE / blockSize;
	for (size_t i = 0; i < max_blocks_in_cache; i++) {
		if (block_cache_index[i].block_num == block_num) {
			kprint("[FsBlocks] Found ");
			printNum(block_num);
			kprint(" in the cache at ");
			printNum(i);
			if (entry) {
				*entry = block_cache_index[i];
			}
			if (!block_cache_index[i].loaded) {
				kprint(" (although it's just reserved)\n");
			} else {
				kprint("\n");
			}

			return &_fs_cache[i * blockSize];
		}
	}

	kprint("[FsBlocks] Block ");
	printNum(block_num);
	printLine(" not in cache!");

	return NULL;
}

bool _fs_blocks_insertIntoCache(u32 block_num, char *data) {
	if (numberOfBlocks == 0 || blockSize == 0) {
		printError("[FsBlocks] Unable to write when disk hasn't been initialised yet!");
		return false;
	}

	size_t max_blocks_in_cache = BLOCK_CACHE_SIZE / blockSize;
	for (size_t i = 0; i < max_blocks_in_cache; i++) {
		if (block_cache_index[i].block_num == SIZE_MAX) {
			kprint("[FsBlocks] Inserting ");
			printNum(block_num);
			kprint(" into cache at ");
			printNum(i);
			kprint("\n");

			block_cache_index[i].block_num = block_num;
			block_cache_index[i].loaded    = data != NULL;
			block_cache_index[i].modified  = false;
			if (data) {
				memcpy(&_fs_cache[i * blockSize], data, blockSize);
			}
			return true;
		}
	}

	printError("[FsBlocks] Unable to insert block into cache, no more spaces!");
	return false;
}

void _fs_blocks_fetchBlocks(size_t n, u32 block_nums[n], BulkBlockReadOperationCallback callback) {

}

void _fs_blocks_handle_query(char *resp, void *meta) {
	if (strcmp(resp, "01") == 0) {
		printError("[FsBlocks] Error from disk ctr during disk query");
		return;
	}

	strtok(resp, " "); // discard
	char *data = strtok(NULL, " ");
	if (!data) {
		printError("[FsBlocks] Error querying disk! Invalid response (no spaces)");
		return;
	}

	size_t len = strlen(data);
	if (len != 16) {
		printError("[FsBlocks] Error querying disk! Invalid response (too short)");
		return;
	}

	numberOfBlocks = xstoi(data,     8);
	blockSize      = xstoi(&data[8], 8);

	kprint("[FsBlocks] Received disk info (nblocks=");
	printNum(numberOfBlocks);
	kprint(", bsize=");
	printNum(blockSize);
	kprint(")\n");

	_fs_blocks_insertIntoCache(0x00, "\0\1\0\1\0\1\0\1\0\1\0\1\0\1\0\1");
	fs_blocks_write(0x05, "x", 1, NULL, NULL);
}

void fs_blocks_init() {
	numberOfBlocks  = 0;
	blockSize       = 0;
	blocks_in_cache = 0;

	for (size_t i = 0; i < MAX_BLOCKS_IN_CACHE; i++) {
		block_cache_index[i].block_num = SIZE_MAX;
	}

	fs_disk_run_command("00", &_fs_blocks_handle_query, NULL);
}

void fs_blocks_write(u32 address, char *data, size_t n, BlockOperationCallback callback, void *meta) {
	if (numberOfBlocks == 0 || blockSize == 0) {
		printError("[FsBlocks] Unable to write when disk hasn't been initialised yet!");
		return;
	}

	u32 block_num = address / blockSize;
	BlockCacheIndexEntry entry = false;
	char *info = _fs_blocks_fetchFromCache(block_num, &entry);
	if (info && is_loaded) {
		for (int i = 0; i < n; i++) {
			info[address - block_num * blockSize + i] = data[i];

			// TODO: into multiple blocks
		}
	} else {
		// TODO: read into cache
	}

}
