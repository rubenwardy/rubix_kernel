#ifndef INODE_PIPE_H
#define INODE_PIPE_H
#include "fides.h"

extern void fides_pipe_init();
extern void fides_pipe_create(FiDes *one, FiDes *two);
extern int fides_pipe_is_pipe(FiDes *fides);

#endif
