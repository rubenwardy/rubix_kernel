#include "fides.h"
#include "utils.h"

#define MAX_INODES 100

FiDes fidess[MAX_INODES];

void fides_init() {
	memset(&fidess[0], 0, sizeof(FiDes) * MAX_INODES);
}

FiDes *fides_create(pid_t pid, u32 fid) {
	int ptr = 0;
	while (fidess[ptr].pid != 0) {
		ptr++;

		if (ptr >= MAX_INODES) {
			printLine("Maximum number of FiDes reached, returning 0!");
			return 0;
		}
	}

	fidess[ptr].pid  = pid;
	fidess[ptr].id   = fid;

	printf("Created fides id=");
	printNum(fidess[ptr].id);
	printf(" for process pid=");
	printNum(pid);
	printf("\n");

	return &fidess[ptr];
}

void fides_duplicate(pid_t pid, FiDes *old) {
	FiDes *fides = fides_create(pid, old->id);
	fides->data  = old->data;
	fides->write = old->write;
	fides->read  = old->read;
	fides->grab  = old->grab;
	fides->drop  = old->drop;
	if (fides->grab) {
		fides->grab(fides);
	}
}

size_t fides_duplicate_all(pid_t oldpid, pid_t pid) {
	size_t num_created = 0;

	for (int i = 0; i < MAX_INODES; i++) {
		if (fidess[i].pid == oldpid) {
			fides_duplicate(pid, &fidess[i]);
			num_created++;
		}
	}

	return num_created;
}

void fides_dropall(pid_t pid) {
	for (int i = 0; i < MAX_INODES; i++) {
		if (fidess[i].pid == pid) {
			if (fidess[i].drop) {
				fidess[i].drop(&fidess[i]);
			}
			fidess[i].pid = 0;
		}
	}
}

FiDes *fides_get(pid_t pid, u32 id) {
	for (int i = 0; i < MAX_INODES; i++) {
		if (fidess[i].pid == pid && fidess[i].id == id) {
			return &fidess[i];
		}
	}

	return 0;
}
