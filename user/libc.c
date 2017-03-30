#include "libc.h"

int  atoi( char* x        ) {
  char* p = x; bool s = false; int r = 0;

  if     ( *p == '-' ) {
    s =  true; p++;
  }
  else if( *p == '+' ) {
    s = false; p++;
  }

  for( int i = 0; *p != '\x00'; i++, p++ ) {
    r = s ? ( r * 10 ) - ( *p - '0' ) :
            ( r * 10 ) + ( *p - '0' ) ;
  }

  return r;
}

void itoa( char* r, int x ) {
  char* p = r; int t, n;

  if( x < 0 ) {
    p++; t = -x; n = 1;
  }
  else {
         t = +x; n = 1;
  }

  while( t >= n ) {
    p++; n *= 10;
  }

  *p-- = '\x00';

  do {
    *p-- = '0' + ( t % 10 ); t /= 10;
  } while( t );

  if( x < 0 ) {
    *p-- = '-';
  }

  return;
}

void yield() {
  asm volatile( "svc %0     \n" // make system call SYS_YIELD
              :
              : "I" (SYS_YIELD)
              : );

  return;
}

int write( int fd, const void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_WRITE
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_WRITE), "r" (fd), "r" (x), "r" (n)
              : "r0", "r1", "r2" );

  return r;
}

int  read( int fd,       void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_READ
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_READ),  "r" (fd), "r" (x), "r" (n)
              : "r0", "r1", "r2" );

  return r;
}

int fork() {
  int r;

  asm volatile( "svc %1     \n" // make system call SYS_FORK
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_FORK)
              : "r0");

  return r;
}

void exit( int x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  x
                "svc %0     \n" // make system call SYS_EXIT
              :
              : "I" (SYS_EXIT), "r" (x)
              : "r0" );

  return;
}

int exec( char* x ) {
  int r;
  asm volatile( "mov r0, %2 \n" // assign r0 = x
                "svc %1     \n" // make system call SYS_EXEC
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_EXEC), "r" (x)
              : "r0" );

  return r;
}

pid_t waitpid( pid_t pid, int *status_code ) {
      pid_t r;
      asm volatile( "mov r0, %2 \n" // assign r0 = pid
                    "mov r1, %3 \n" // assign r1 = status code
                    "svc %1     \n" // make system call SYS_WAIT
                    "mov %0, r0 \n" // assign r  = r0
                  : "=r" (r)
                  : "I" (SYS_WAIT), "r" (pid), "r" (status_code)
                  : "r0", "r1" );

      return r;
}

int pipe(int fd[2]) {
      pid_t r;
      asm volatile( "mov r0, %2 \n" // assign r0 = pid
                    "svc %1     \n" // make system call SYS_PIPE
                    "mov %0, r0 \n" // assign r  = r0
                  : "=r" (r)
                  : "I" (SYS_PIPE), "r" (fd)
                  : "r0" );

      return r;
}

extern int popen( char *cmd, char mode ) {
    if (mode != 'r' && mode != 'w') {
        return NULL;
    }

    int fd[2];
    if (pipe(fd) == -1) {
        return NULL;
    }

    int res = fork();
    if (res == -1) {
        close(fd[0]);
        close(fd[1]);
        return NULL;
    } else if (res == 0) {
        // is child
        if (mode == 'r') {
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            close(fd[1]);
        } else {
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            close(fd[1]);
        }

        exec(cmd);
    } else {
        // is parent
        if (mode == 'r') {
            return fd[0];
        } else {
            return fd[1];
        }
    }
}

int dup2(int old, int new) {
      int r;
      asm volatile( "mov r0, %2 \n" // assign r0 = pid
                    "mov r1, %3 \n" // assign r0 = pid
                    "svc %1     \n" // make system call SYS_DUP2
                    "mov %0, r0 \n" // assign r  = r0
                  : "=r" (r)
                  : "I" (SYS_DUP2), "r" (old), "r" (new)
                  : "r0", "r1" );

      return r;
}

int close(int fd) {
      pid_t r;
      asm volatile( "mov r0, %2 \n" // assign r0 = pid
                    "svc %1     \n" // make system call SYS_CLOSE
                    "mov %0, r0 \n" // assign r  = r0
                  : "=r" (r)
                  : "I" (SYS_CLOSE), "r" (fd)
                  : "r0" );

      return r;
}

int fd_setblock(int fd, int isblocking) {
      int r;
      asm volatile( "mov r0, %2 \n" // assign r0 = pid
                    "mov r1, %3 \n" // assign r0 = pid
                    "svc %1     \n" // make system call SYS_CLOSE
                    "mov %0, r0 \n" // assign r  = r0
                  : "=r" (r)
                  : "I" (SYS_FD_SETBLOCK), "r" (fd), "r" (isblocking)
                  : "r0" );

      return r;
}

int setpriority(int who, int prio) {
      int r;
      asm volatile( "mov r0, %2 \n" // assign r0 = pid
                    "mov r1, %3 \n" // assign r0 = pid
                    "svc %1     \n" // make system call SYS_CLOSE
                    "mov %0, r0 \n" // assign r  = r0
                  : "=r" (r)
                  : "I" (SYS_SETPRIORITY), "r" (who), "r" (prio)
                  : "r0" );

      return r;
}

int kill( int pid, int x ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 =  pid
                "mov r1, %3 \n" // assign r1 =    x
                "svc %1     \n" // make system call SYS_KILL
                "mov %0, r0 \n" // assign r0 =    r
              : "=r" (r)
              : "I" (SYS_KILL), "r" (pid), "r" (x)
              : "r0", "r1" );

  return r;
}
