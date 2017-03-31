#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libc.h"
#include "stdio.h"

uint32_t gcd( uint32_t x, uint32_t y ) {
  if     ( x == y ) {
    return x;
  }
  else if( x >  y ) {
    return gcd( x - y, y );
  }
  else if( x <  y ) {
    return gcd( x, y - x );
  }
}

void main_P4() {
    int fd = fopen("a.txt", 'r');
    if (fd < 0) {
        printf("FDError!\n");
    }

    char buffer[100];
    int n = read(fd, buffer, 100);
    write(STDOUT_FILENO, buffer, n);


    write( STDOUT_FILENO, "P4", 2 );
    exit( EXIT_SUCCESS );
  while( 1 ) {

    uint32_t lo = 1 <<  4;
    uint32_t hi = 1 <<  8;

    for( uint32_t x = lo; x < hi; x++ ) {
      for( uint32_t y = lo; y < hi; y++ ) {
        uint32_t r = gcd( x, y );
      }
    }
  }

  exit( EXIT_SUCCESS );
}
