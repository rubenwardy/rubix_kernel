#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>

extern size_t kprint(const char *cs);
extern void kprintmod(const char *mod);
extern void printLine(const char *mod, const char *cs);
extern void printError(const char *mod, const char *cs);
extern void printNum2(int num);
extern void printNum(int num);

#define min(a,b) (((a) > (b)) ? (b) : (a))
#define max(a,b) (((a) > (b)) ? (a) : (b))

#endif
