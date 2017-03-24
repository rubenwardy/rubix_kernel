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
    int pid = fork();
    if (pid == 0) {
        write(STDOUT_FILENO, "child", 5);
        while(1) {
            yield();
        }
    } else if (pid == -1) {
        write( STDOUT_FILENO, "err", 3);
    } else {
        yield();
        yield();
        yield();
        yield();
        yield();
        int res = kill(pid, 9);
        if (res == 0) {
            write( STDOUT_FILENO, "ok", 2);
        } else {
            write( STDOUT_FILENO, "err", 3);
        }
    }

    // int fd = popen("p4", 'r');
    //
    // char buffer[100];
    // size_t size = read(fd, buffer, 100);
    // if (size == 0) {
    //     write( STDOUT_FILENO, "eof", 3);
    // } else {
    //     write( STDOUT_FILENO, "remaining", 3);
    // }
    // close(fd);
    // write(STDOUT_FILENO, buffer, size);

    // exit(EXIT_SUCCESS);
  //
  //   int fd[2];
  //   if (pipe(fd) == -1) {
  //       write( STDOUT_FILENO, "perror", 5);
  //       exit(1);
  //       return;
  //   }
  //
  //   int res = fork();
  //   if (res == -1) {
  //       write( STDOUT_FILENO, "error", 5);
  //   } else if (res == 0) {
  //       write( STDOUT_FILENO, "child", 5);
  //       close(fd[0]);
  //       write(fd[1], "hello", 5);
  //       close(fd[1]);
  //   } else {
  //       write( STDOUT_FILENO, "parent", 6);
  //
  //       close(fd[1]);
  //       char buffer[100];
  //       size_t size = read(fd[0], buffer, 100);
  //       if (size == 0) {
  //           write( STDOUT_FILENO, "eof", 3);
  //       } else {
  //           write( STDOUT_FILENO, "remaining", 3);
  //       }
  //       close(fd[0]);
  //       write(STDOUT_FILENO, buffer, size);
  //
  //       int code;
  //       int pid = wait(&code);
  //       if (pid == res) {
  //           write( STDOUT_FILENO, "cpid", 4);
  //       } else {
  //           write( STDOUT_FILENO, "wpid", 4);
  //       }
  //
  //       if (code == 5) {
  //           write( STDOUT_FILENO, "5", 1);
  //       } else {
  //           write( STDOUT_FILENO, "n5", 2);
  //       }
  //
  //       exit(EXIT_SUCCESS);
  //       return;
  //   }
  //   write( STDOUT_FILENO, "P3", 2 );
  //
  // // int res = exec("p4");
  // // if (res == -1) {
  // //     write( STDOUT_FILENO, "err", 3 );
  // // } else {
  // //     write( STDOUT_FILENO, "oth", 3 );
  // // }

  for( int i = 0; i < 10; i++ ) {
    write( STDOUT_FILENO, "P3", 2 );

    uint32_t lo = 1 <<  8;
    uint32_t hi = 1 << 16;

    for( uint32_t x = lo; x < hi; x++ ) {
      int r = is_prime( x );
    }
  }
  //
  exit( 5 );
}
