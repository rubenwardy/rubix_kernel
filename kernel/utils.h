#ifndef UTILS_H
#define UTILS_H

extern void printf(const char *cs);
extern void printLine(const char *cs);
extern void printError(const char *cs);
extern void printNum2(int num);
extern void printNum(int num);

#define min(a,b) (((a) > (b)) ? (b) : (a))
#define max(a,b) (((a) > (b)) ? (a) : (b))

#endif
