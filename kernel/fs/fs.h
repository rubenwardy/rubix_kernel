#ifndef VFS_H
#define VFS_H

#include "inode.h"
#include "disk.h"

extern void fs_init();
extern void fs_on_disk_connected();
extern INode *fs_get_inode(const char *path);

#endif
