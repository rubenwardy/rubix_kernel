#include "fides.h"
#include "utils.h"

#define MAX_INODES 1000

FiDes fidess[MAX_INODES];

void fides_init() {
	memset(&fidess[0], 0, sizeof(FiDes) * MAX_INODES);
}

FiDes *fides_create(pid_t pid, u32 fid) {
	int ptr = 0;
	while (fidess[ptr].pid != 0) {
		ptr++;

		if (ptr >= MAX_INODES) {
			printError("Maximum number of FiDes reached, returning 0!");
			return 0;
		}
	}

	fidess[ptr].pid  = pid;
	fidess[ptr].id   = fid;
	fidess[ptr].is_blocking = true;

	kprint("Created fides id=");
	printNum(fidess[ptr].id);
	kprint(" for process pid=");
	printNum(pid);
	kprint("\n");

	return &fidess[ptr];
}

void fides_duplicate(pid_t pid, FiDes *old) {
	FiDes *fides = fides_create(pid, old->id);
	fides->is_blocking = old->is_blocking;
	fides->data  = old->data;
	fides->write = old->write;
	fides->read  = old->read;
	fides->grab  = old->grab;
	fides->drop  = old->drop;
	if (fides->grab) {
		fides->grab(fides);
	}
}

void fides_duplicateAlias(pid_t pid, u32 old, u32 new) {
	FiDes *fides = fides_get(pid, old);
	FiDes *fides_new = fides_create(pid, new);
	fides_new->data  = fides->data;
	fides_new->is_blocking = fides->is_blocking;
	fides_new->write = fides->write;
	fides_new->read  = fides->read;
	fides_new->grab  = fides->grab;
	fides_new->drop  = fides->drop;
	if (fides_new->grab) {
		fides_new->grab(fides_new);
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

int fides_drop(pid_t pid, u32 fid) {
	FiDes *fides = fides_get(pid, fid);
	if (!fides) {
		return 0;
	}

	if (fides->drop) {
		fides->drop(fides);
	} else {
		printLine("No drop found!");
	}
	memset(fides, 0, sizeof(FiDes));
	return 1;
}

void fides_dropall(pid_t pid) {
	for (int i = 0; i < MAX_INODES; i++) {
		if (fidess[i].pid == pid) {
			kprint("[Fides] Dropping open fd, id=");
			printNum(fidess[i].id);
			kprint("\n");
			if (fidess[i].drop) {
				fidess[i].drop(&fidess[i]);
			}
			fidess[i].pid = 0;
		}
	}
}

void fides_dropallOnExec(pid_t pid) {
	for (int i = 0; i < MAX_INODES; i++) {
		// TODO: add close_on_exec flag
		if (fidess[i].pid == pid && fidess[i].id > 2) {
			kprint("[Fides] Dropping open fd on exec, id=");
			printNum(fidess[i].id);
			kprint("\n");
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

	printError("Failed to find fides");
	return 0;
}
