#ifndef VFS_H
#define VFS_H

#include "inode.h"
#include "disk.h"

extern void fs_init();
extern INode *fs_get_inode(const char *path);

#endif
