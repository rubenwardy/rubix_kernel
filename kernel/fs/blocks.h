#ifndef BLOCKS_H
#define BLOCKS_H

#include "../hilevel.h"

typedef void (*BlockOperationCallback)(char *resp, void *meta);

extern void fs_blocks_init();
extern void fs_blocks_write(u32 address, char *data, size_t n, BlockOperationCallback callback, void *meta);

#endif
