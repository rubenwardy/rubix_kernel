#include "inode.h"

void inode_set_perms(INode *inode, u8 u, u8 g, u8 e) {
	inode->perms = (((u << 4) | g) << 4) | e;
}
