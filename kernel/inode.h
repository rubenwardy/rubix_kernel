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

typedef enum {
	OPERATION_STICKY  = 0b1000,
	OPERATION_READ    = 0b0100,
	OPERATION_WRITE   = 0b0010,
	OPERATION_EXEC    = 0b0001
} Operation;

extern void inode_init();
extern INode *inode_create(u32 user, u32 group);
extern INode *inode_get(u32 id);
extern bool inode_check_perm(INode *node, u32 user, u32 group, Operation op);

#endif
