#ifndef FIDES_FILE_H
#define FIDES_FILE_H
#include "fides.h"

extern void fides_file_init();
extern void fides_file_create(FiDes *one, char *path, char mode);
extern int fides_file_is_file(FiDes *fides);

#endif
