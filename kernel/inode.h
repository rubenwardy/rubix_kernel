#ifndef INODE_H
#define INODE_H
#include "hilevel.h"

struct INode {
	u32 id;
	u32 user_id;
	u32 group_id;

	// BIT flags:
	//   U    G    E
	// srwx srwx srwx
	u16 perms;

	size_t (*read)    (struct INode *node, const char *data, size_t max);
	size_t (*write)   (struct INode *node, const char *data, size_t len);
	int (*execute) (struct INode *node);
};
typedef struct INode INode;

#endif
