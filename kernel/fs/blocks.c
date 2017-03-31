#include "blocks.h"
#include "disk.h"
#include "../utils.h"

size_t numberOfBlocks;
size_t blockSize;

#define BLOCK_CACHE_SIZE 4096
#define MAX_BLOCK_SIZE 256
#define MAX_BLOCKS_IN_CACHE 8
char _fs_cache[BLOCK_CACHE_SIZE];

typedef struct {
	u32 block_num;
	bool loaded;
	bool modified;
} BlockCacheIndexEntry;

BlockCacheIndexEntry block_cache_index[MAX_BLOCKS_IN_CACHE];
size_t blocks_in_cache;

typedef struct {
	u32 block_num;
	BlockOperationCallback callback;
	void *meta;
} BlockOperationMeta;
#define MAX_BLOCK_OPERATIONS 100
BlockOperationMeta block_operation_meta[MAX_BLOCK_OPERATIONS];

typedef void (*BulkBlockReadOperationCallback)(int n, u32 address_num[n], char *data[n]);

char *_fs_blocks_fetchFromCache(u32 block_num, BlockCacheIndexEntry **entry) {
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
				*entry = &block_cache_index[i];
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

char *_fs_blocks_insertIntoCache(u32 block_num, char *data, BlockCacheIndexEntry **entry) {
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
			if (entry) {
				*entry = &block_cache_index[i];
			}
			return &_fs_cache[i * blockSize];
		}
	}

	printError("[FsBlocks] Unable to insert block into cache, no more spaces!");
	return NULL;
}

char *_fs_blocks_fetchFromCacheOrCreate(u32 block_num, BlockCacheIndexEntry **entry) {
	char *ret = _fs_blocks_fetchFromCache(block_num, entry);
	if (ret) {
		return ret;
	}

	return _fs_blocks_insertIntoCache(block_num, NULL, entry);
}

BlockOperationMeta *_fs_blocks_allocateBlockOperationMeta(u32 block_num) {
	for (size_t i = 0; i < MAX_BLOCK_OPERATIONS; i++) {
		if (block_operation_meta[i].block_num == SIZE_MAX) {
			block_operation_meta[i].block_num = block_num;
			return &block_operation_meta[i];
		}
	}

	printError("[FsBlocks] Out of block operation meta space!");

	return NULL;
}

void _fs_blocks_fetchBlocks(size_t n, u32 block_nums[n], BulkBlockReadOperationCallback callback) {

}

void _fs_tmp_read_cb(u32 block_num, char *data, void *meta) {
	printError("Read block!");
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

	if (blockSize > MAX_BLOCK_SIZE) {
		printError("[FsBlock] Block size exceeds maximum!");
		numberOfBlocks = 0;
		blockSize = 0;
		return;
	}

	// fs_blocks_writeBlock(0, "\0\1\0\1\0\1\0\1\0\1\0\1\0\1\0\1", NULL, NULL);
	fs_blocks_readBlock(0, _fs_tmp_read_cb, NULL);
}

void fs_blocks_init() {
	numberOfBlocks  = 0;
	blockSize       = 0;
	blocks_in_cache = 0;

	for (size_t i = 0; i < MAX_BLOCKS_IN_CACHE; i++) {
		block_cache_index[i].block_num = SIZE_MAX;
	}

	for (size_t i = 0; i < MAX_BLOCK_OPERATIONS; i++) {
		block_operation_meta[i].block_num = SIZE_MAX;
	}

	fs_disk_run_command("00", &_fs_blocks_handle_query, NULL);
}

void _fs_blocks_handleRead(char *resp, void *meta) {
	if (numberOfBlocks == 0 || blockSize == 0) {
		printError("[FsBlocks] Unable to handle read when disk hasn't been initialised yet!");
		return;
	}

	if (!meta) {
		printError("[FsBlocks] No meta proved by cmd?");
		return;
	}

	BlockOperationMeta *bmeta = (BlockOperationMeta*)meta;

	if (strcmp(resp, "01") == 0) {
		bmeta->block_num = SIZE_MAX;
		printError("[FsBlocks] Error from disk ctr during disk read");
		return;
	}

	char data[MAX_BLOCK_SIZE+1];
	for (u32 i = 0; i < blockSize; i++) {
		char hex1 = resp[3 + i*2];
		char hex2 = resp[4 + i*2];
		data[i] = (xtoi(hex1) << 4) | xtoi(hex2);
	}
	data[blockSize] = '\0';

	if (bmeta->callback) {
		bmeta->callback(bmeta->block_num, data, bmeta->meta);
	}

	bmeta->block_num = SIZE_MAX;

	printError("[FsBlocks] Read finished!");
}

void fs_blocks_readBlock(u32 block_num, BlockOperationCallback callback, void *meta) {
	if (numberOfBlocks == 0 || blockSize == 0) {
		printError("[FsBlocks] Unable to read when disk hasn't been initialised yet!");
		return;
	}

	BlockCacheIndexEntry *entry = NULL;
	char *data = _fs_blocks_fetchFromCacheOrCreate(block_num, &entry);
	if (!data) {
		printError("[FsBlocks] Unable to create cache item!");
		return;
	} else if (!entry) {
		printError("[FsBlocks] Entry is NULL");
		return;
	}

	if (entry->loaded) {
		printError("[FsBlocks] Warning: returning block from cache");
		if (callback) {
			callback(block_num, data, meta);
		}
		return;
	}

	char cmd[20] = "02 ";
	size_t ptr = 3;

	for (int i = 0; i < 4; i++) {
		char byte = (block_num >> (i * 8)) & 0xFF;
		cmd[ptr++] = itox(byte >> 4);
		cmd[ptr++] = itox(byte & 0x0F);
	}
	cmd[ptr] = '\0';

	BlockOperationMeta *bmeta = _fs_blocks_allocateBlockOperationMeta(block_num);
	if (!bmeta) {
		printError("[FsBlocks] Unable to write as allocateBlockOperationMeta returned NULL");
		return;
	}

	bmeta->callback = callback;
	bmeta->meta = meta;

	fs_disk_run_command(cmd, &_fs_blocks_handleRead, (void*)bmeta);
}

void _fs_blocks_handleWrite(char *resp, void *meta) {
	if (!meta) {
		printError("[FsBlocks] No meta proved by cmd?");
		return;
	}

	BlockOperationMeta *bmeta = (BlockOperationMeta*)meta;

	if (strcmp(resp, "01") == 0) {
		bmeta->block_num = SIZE_MAX;
		printError("[FsBlocks] Error from disk ctr during disk write");
		return;
	}

	if (bmeta->callback) {
		bmeta->callback(bmeta->block_num, NULL, bmeta->meta);
	}

	bmeta->block_num = SIZE_MAX;

	printError("[FsBlocks] Write finished!");
}

void fs_blocks_writeBlock(u32 block_num, char *content, BlockOperationCallback callback, void *meta) {
	if (numberOfBlocks == 0 || blockSize == 0) {
		printError("[FsBlocks] Unable to write when disk hasn't been initialised yet!");
		return;
	}

	BlockCacheIndexEntry *entry = NULL;
	char *data = _fs_blocks_fetchFromCacheOrCreate(block_num, &entry);
	if (!data) {
		printError("[FsBlocks] Unable to create cache item!");
		return;
	} else if (!entry) {
		printError("[FsBlocks] Entry is NULL");
		return;
	}

	memcpy(data, content, blockSize * sizeof(char));
	entry->loaded = true; // TODO: is this correct?

	char cmd[MAX_BLOCK_SIZE*2 + 20] = "01 ";
	size_t ptr = 3;

	for (int i = 0; i < 4; i++) {
		char byte = (block_num >> (i * 8)) & 0xFF;
		cmd[ptr++] = itox(byte >> 4);
		cmd[ptr++] = itox(byte & 0x0F);
	}

	cmd[ptr++] = ' ';

	for (int i = 0; i < blockSize; i++) {
		char byte = content[i];
		cmd[ptr++] = itox(byte >> 4);
		cmd[ptr++] = itox(byte & 0x0F);
	}
	cmd[ptr] = '\0';

	BlockOperationMeta *bmeta = _fs_blocks_allocateBlockOperationMeta(block_num);
	if (!bmeta) {
		printError("[FsBlocks] Unable to write as allocateBlockOperationMeta returned NULL");
		return;
	}

	bmeta->callback = callback;
	bmeta->meta = meta;

	fs_disk_run_command(cmd, &_fs_blocks_handleWrite, (void*)bmeta);
}

void fs_blocks_write(u32 address, char *data, size_t n, BlockOperationCallback callback, void *meta) {
	if (numberOfBlocks == 0 || blockSize == 0) {
		printError("[FsBlocks] Unable to write when disk hasn't been initialised yet!");
		return;
	}

	u32 block_num = address / blockSize;
	BlockCacheIndexEntry *entry = NULL;
	char *info = _fs_blocks_fetchFromCache(block_num, &entry);
	if (info && entry && entry->loaded) {
		for (int i = 0; i < n; i++) {
			info[address - block_num * blockSize + i] = data[i];

			// TODO: into multiple blocks
		}
	} else {
		// TODO: read into cache
	}

}
