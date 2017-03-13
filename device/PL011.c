#include "PL011.h"

volatile PL011_t* UART0 = ( volatile PL011_t* )( 0x10009000 );
volatile PL011_t* UART1 = ( volatile PL011_t* )( 0x1000A000 );
volatile PL011_t* UART2 = ( volatile PL011_t* )( 0x1000B000 );
volatile PL011_t* UART3 = ( volatile PL011_t* )( 0x1000C000 );

int  xtoi( char x ) {
  if      ( ( x >= '0' ) && ( x <= '9' ) ) {
    return (  0 + ( x - '0' ) );
  }
  else if ( ( x >= 'a' ) && ( x <= 'f' ) ) {
    return ( 10 + ( x - 'a' ) );
  }
  else if ( ( x >= 'A' ) && ( x <= 'F' ) ) {
    return ( 10 + ( x - 'A' ) );
  }

  return -1;
}

char itox( int  x ) {
  if     ( ( x >=  0 ) && ( x <=  9 ) ) {
    return '0' + ( x -  0 );
  }
  else if( ( x >= 10 ) && ( x <= 15 ) ) {
    return 'A' + ( x - 10 );
  }

  return -1;
}

bool    PL011_can_putc( PL011_t* d ) {
  // can putc iff. transmit FIFO is not full
  return !( d->FR & 0x20 );
}

bool    PL011_can_getc( PL011_t* d ) {
  // can getc iff. receive  FIFO is not empty
  return !( d->FR & 0x10 );
}

void    PL011_putc( PL011_t* d, uint8_t x, bool f ) {
  // wait while blocking enabled and transmit FIFO is full
  while( f && ( d->FR & 0x20 ) );
  //            transmit x
  d->DR = x;
}

uint8_t PL011_getc( PL011_t* d,            bool f ) {
  // wait while blocking enabled and receive  FIFO is empty
  while( f && ( d->FR & 0x10 ) );
  //            recieve  r
  return d->DR;
}

void    PL011_puth( PL011_t* d, uint8_t x, bool f ) {
  PL011_putc( d, itox( ( x >> 4 ) & 0xF ), f );
  PL011_putc( d, itox( ( x >> 0 ) & 0xF ), f );
}

uint8_t PL011_geth( PL011_t* d,            bool f ) {
  uint8_t r  = ( xtoi( PL011_getc( d, f ) ) << 4 );
          r |= ( xtoi( PL011_getc( d, f ) ) << 0 );

  return r;
}
