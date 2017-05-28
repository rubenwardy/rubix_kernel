#ifndef DISK_H
#define DISK_H

#include "../../hilevel.h"

typedef void (*DiskCommandHandler)(char *resp, void *meta);

extern int xstoi(const char *xs, size_t n);

extern void fs_disk_init();
extern void fs_disk_run_command(char *cmd, DiskCommandHandler handler, void *meta);
extern void fs_disk_on_interrupt();

#endif
