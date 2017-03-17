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
    // int res = fork();
    // if (res == -1) {
    //     write( STDOUT_FILENO, "error", 5);
    // } else if (res == 0) {
    //     write( STDOUT_FILENO, "child", 5);
    // } else {
    //     write( STDOUT_FILENO, "parent", 6);
    // }
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

  exit( EXIT_SUCCESS );
}
