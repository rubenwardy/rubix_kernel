#include "inode.h"

#define MAX_INODES 3

INode inodes[MAX_INODES];
int inode_id_counter;

void inode_init() {
	memset(&inodes[0], 0, sizeof(INode) * MAX_INODES);
	inode_id_counter = 0;
}

INode *inode_create(u32 user, u32 group) {
	int ptr = 0;
	while (inodes[ptr].id != 0) {
		ptr++;

		if (ptr >= MAX_INODES) {
			return 0;
		}
	}

	inodes[ptr].id = ++inode_id_counter;
	inodes[ptr].user_id = user;
	inodes[ptr].group_id = group;

	return &inodes[ptr];
}

INode *inode_get(u32 id) {
	for (int i = 0; i < MAX_INODES; i++) {
		if (inodes[i].id == id) {
			return &inodes[i];
		}
	}

	return 0;
}

bool inode_check_perm(INode *node, u32 user, u32 group, Operation op) {
	size_t offset = 0;
	if (user == node->user_id) {
		offset = 8;
	} else if (group == node->group_id) {
		offset = 4;
	// } else {
	// 	offset = 0;
	}

	return (node->perms >> offset) & op;
}
