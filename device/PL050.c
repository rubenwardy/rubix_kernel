#include "PL050.h"

volatile PL050_t* PS20 = ( volatile PL050_t* )( 0x10006000 );
volatile PL050_t* PS21 = ( volatile PL050_t* )( 0x10007000 );

void    PL050_putc( PL050_t* d, uint8_t x ) {
  // wait while transmit register isn't empty 
  while( !( d->STAT & 0x40 ) );
  //            transmit x
  d->DATA = x;
}

uint8_t PL050_getc( PL050_t* d            ) {
  // wait while receive  register isn't  full
  while( !( d->STAT & 0x10 ) );
  //            recieve  r
  return d->DATA;
}
