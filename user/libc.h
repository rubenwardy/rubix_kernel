#ifndef __LIBC_H
#define __LIBC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Define a type that that captures a Process IDentifier (PID).

typedef int pid_t;

/* The definitions below capture symbolic constants within these classes:
 *
 * 1. system call identifiers (i.e., the constant used by a system call
 *    to specify which action the kernel should take),
 * 2. signal identifiers (as used by the kill system call),
 * 3. status codes for exit,
 * 4. standard file descriptors (e.g., for read and write system calls),
 * 5. platform-specific constants, which may need calibration (wrt. the
 *    underlying hardware QEMU is executed on).
 *
 * They don't *precisely* match the standard C library, but are intended
 * to act as a limited model of similar concepts.
 */

#define SYS_YIELD       ( 0x00 )
#define SYS_WRITE       ( 0x01 )
#define SYS_READ        ( 0x02 )
#define SYS_FORK        ( 0x03 )
#define SYS_EXIT        ( 0x04 )
#define SYS_EXEC        ( 0x05 )
#define SYS_KILL        ( 0x06 )
#define SYS_WAIT        ( 0x07 )
#define SYS_PIPE        ( 0x08 )
#define SYS_CLOSE       ( 0x09 )
#define SYS_DUP2        ( 0x10 )
#define SYS_FD_SETBLOCK ( 0x11 )
#define SYS_SETPRIORITY ( 0x12 )
#define SYS_FOPEN       ( 0x13 )

#define SIG_TERM      ( 0x00 )
#define SIG_QUIT      ( 0x01 )

#define EXIT_SUCCESS  ( 0 )
#define EXIT_FAILURE  ( 1 )

#define  STDIN_FILENO ( 0 )
#define STDOUT_FILENO ( 1 )
#define STDERR_FILENO ( 2 )

// convert ASCII string x into integer r
extern int  atoi( char* x        );
// convert integer x into ASCII string r
extern void itoa( char* r, int x );

// cooperatively yield control of processor, i.e., invoke the scheduler
extern void yield();

// write n bytes from x to   the file descriptor fd; return bytes written
extern int write( int fd, const void* x, size_t n );
// read  n bytes into x from the file descriptor fd; return bytes read
extern int  read( int fd,       void* x, size_t n );

// perform fork, returning 0 iff. child or > 0 iff. parent process
extern int  fork();
// perform exit, i.e., terminate process with status x
extern void exit(       int   x );
// perform exec, i.e., start executing program at address x
extern int exec( char* x );
// perform blocking wait for child's exit code
extern pid_t waitpid( pid_t pid, int *status_code );
inline pid_t wait(int *status_code) { return waitpid(0, status_code); }

extern int pipe(int fd[2]);

extern int popen( char *cmd, char mode );

extern int dup2(int old, int new);

extern int close(int fd);

extern int fd_setblock(int fd, int isblocking);

extern int setpriority(int who, int prio);

extern int fopen(char *path, char mode);

// signal process identified by pid with signal x
extern int  kill( pid_t pid, int x );

#endif
