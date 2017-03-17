#include "P5.h"

uint32_t weight( uint32_t x ) {
  x = ( x & 0x55555555 ) + ( ( x >>  1 ) & 0x55555555 );
  x = ( x & 0x33333333 ) + ( ( x >>  2 ) & 0x33333333 );
  x = ( x & 0x0F0F0F0F ) + ( ( x >>  4 ) & 0x0F0F0F0F );
  x = ( x & 0x00FF00FF ) + ( ( x >>  8 ) & 0x00FF00FF );
  x = ( x & 0x0000FFFF ) + ( ( x >> 16 ) & 0x0000FFFF );

  return x;
}

void main_P5() {
  while( 1 ) {
    write( STDOUT_FILENO, "P5", 2 );

    uint32_t lo = 1 <<  8;
    uint32_t hi = 1 << 24;

    for( uint32_t x = lo; x < hi; x++ ) {
      uint32_t r = weight( x );
    }
  }

  exit( EXIT_SUCCESS );
}
