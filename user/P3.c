#include "P3.h"

int is_prime( uint32_t x ) {
  if ( !( x & 1 ) || ( x < 2 ) ) {
    return ( x == 2 );
  }

  for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
    if( !( x % d ) ) {
      return 0;
    }
  }

  return 1;
}

extern void main_P4();

void main_P3() {
    int fd[2];
    if (pipe(fd) == -1) {
        write( STDOUT_FILENO, "perror", 5);
        exit(1);
        return;
    }

    int res = fork();
    if (res == -1) {
        write( STDOUT_FILENO, "error", 5);
    } else if (res == 0) {
        write( STDOUT_FILENO, "child", 5);
    } else {
        write( STDOUT_FILENO, "parent", 6);
        int code;
        int pid = wait(&code);
        if (pid == res) {
            write( STDOUT_FILENO, "cpid", 4);
        } else {
            write( STDOUT_FILENO, "wpid", 4);
        }

        if (code == 5) {
            write( STDOUT_FILENO, "5", 1);
        } else {
            write( STDOUT_FILENO, "n5", 2);
        }

        exit(EXIT_SUCCESS);
        return;
    }
    write( STDOUT_FILENO, "P3", 2 );

  // int res = exec("p4");
  // if (res == -1) {
  //     write( STDOUT_FILENO, "err", 3 );
  // } else {
  //     write( STDOUT_FILENO, "oth", 3 );
  // }

  for( int i = 0; i < 50; i++ ) {
    write( STDOUT_FILENO, "P3", 2 );

    uint32_t lo = 1 <<  8;
    uint32_t hi = 1 << 16;

    for( uint32_t x = lo; x < hi; x++ ) {
      int r = is_prime( x );
    }
  }

  exit( 5 );
}
