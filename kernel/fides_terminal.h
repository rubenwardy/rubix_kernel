#ifndef INODE_TERMINAL_H
#define INODE_TERMINAL_H

#include "fides.h"

extern void fides_terminal_create(FiDes *in, FiDes *out, FiDes *err);
extern void fides_terminal_input(char c);
extern int fides_terminal_is_terminal(FiDes *fides);

#endif
