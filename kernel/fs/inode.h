#ifndef INODE_H
#define INODE_H
#include "../hilevel.h"

struct __attribute__((__packed__)) INode {
	u32 id;
	u32 user_id;
	u32 group_id;

	u32 block_num;
	u8 size;

	// BIT flags:
	//   U    G    E
	// srwx srwx srwx
	u16 perms;
};
typedef struct INode INode;

typedef enum {
	INODE_PERM_SETBIT  = 0b1000,
	INODE_PERM_READ    = 0b0100,
	INODE_PERM_WRITE   = 0b0010,
	INODE_PERM_EXECUTE = 0b0001,
} INodePerm;

extern void inode_set_perms(INode *inode, u8 u, u8 g, u8 e);

#endif
