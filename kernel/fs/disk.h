#ifndef DISK_H
#define DISK_H

typedef void (*DiskCommandHandler)(char *resp, void *data);

extern void fs_disk_init();
extern void fs_disk_run_command(char *cmd, DiskCommandHandler handler, void *data);
extern void fs_disk_on_interrupt();

#endif
