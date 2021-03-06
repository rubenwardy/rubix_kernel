#ifndef FIDES_H
#define FIDES_H
#include "hilevel.h"

struct FiDes {
	pid_t pid;
	u32 id;
	int refs;
	int data; // Used to store pipe id or inode id

	bool is_blocking;

	size_t (*read)  (struct FiDes *node, char *data, size_t max);
	size_t (*write) (struct FiDes *node, const char *data, size_t len);

	void (*grab) (struct FiDes *node);
	void (*drop) (struct FiDes *node);
};
typedef struct FiDes FiDes;

extern void fides_init();
extern FiDes *fides_create(pid_t pid, u32 fid);
extern void fides_duplicate(pid_t pid, FiDes *old);
extern void fides_duplicateAlias(pid_t pid, u32 old, u32 new);
extern size_t fides_duplicate_all(pid_t oldpid, pid_t pid);
extern int fides_drop(pid_t pid, u32 fid);
extern void fides_dropall(pid_t pid);
extern void fides_dropallOnExec(pid_t pid);
extern FiDes *fides_get(pid_t pid, u32 id);

#endif
