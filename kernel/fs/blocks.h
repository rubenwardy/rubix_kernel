#ifndef BLOCKS_H
#define BLOCKS_H

#include "../hilevel.h"

typedef void (*BlockOperationCallback)(u32 block_num, char *resp, void *meta);

extern void fs_blocks_init();

extern u32 fs_blocks_getBlockSize();

extern void fs_blocks_readBlock(u32 block_num, BlockOperationCallback callback, void *meta);
extern void fs_blocks_writeBlock(u32 block_num, char *content, BlockOperationCallback callback, void *meta);

extern void fs_blocks_write(u32 address, char *data, size_t n, BlockOperationCallback callback, void *meta);

#endif
