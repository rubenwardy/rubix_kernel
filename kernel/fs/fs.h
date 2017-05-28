#ifndef VFS_H
#define VFS_H

#include "inode.h"
#include "pydisk/disk.h"

typedef void (*INodeFetchCallback)(INode *inode, void *meta);

extern void fs_init();
extern void fs_on_disk_connected();
extern bool fs_fetchINode(const char *path, INodeFetchCallback callback, void *meta);

#endif
