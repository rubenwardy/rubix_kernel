#ifndef INODE_PIPE_H
#define INODE_PIPE_H
#include "inode.h"

extern void inode_pipe_init();
extern void inode_pipe_create(INode *one, INode *two);

#endif
