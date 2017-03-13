#include "disk.h"

void addr_puth( PL011_t* d,       uint32_t x,        bool f ) {
  PL011_puth( d, ( x >>  0 ) & 0xFF, f );
  PL011_puth( d, ( x >>  8 ) & 0xFF, f );
  PL011_puth( d, ( x >> 16 ) & 0xFF, f );
  PL011_puth( d, ( x >> 24 ) & 0xFF, f );
}

void data_puth( PL011_t* d, const uint8_t* x, int n, bool f ) {
  for( int i = 0; i < n; i++ ) {
    PL011_puth( d, x[ i ], f );
  }
}

void data_geth( PL011_t* d,       uint8_t* x, int n, bool f ) {
  for( int i = 0; i < n; i++ ) {
    x[ i ] = PL011_geth( d, f );
  }
}

int disk_get_block_num() {
  int n = 2 * sizeof( uint32_t ); uint8_t x[ n ];

  for( int i = 0; i < DISK_RETRY; i++ ) {
      PL011_puth( UART2, 0x00, true );        // write command
      PL011_putc( UART2, '\n', true );        // write EOL

    if( PL011_geth( UART2, true ) == 0x00 ) { // read  command
      PL011_getc( UART2,       true );        // read  separator
       data_geth( UART2, x, n, true );        // read  data
      PL011_getc( UART2,       true );        // read  EOL
      
      return ( ( uint32_t )( x[ 0 ] ) <<  0 ) |
             ( ( uint32_t )( x[ 1 ] ) <<  8 ) |
             ( ( uint32_t )( x[ 2 ] ) << 16 ) |
             ( ( uint32_t )( x[ 3 ] ) << 24 ) ;
    } 
    else {
      PL011_getc( UART2,       true );        // read  EOL
    }
  }

  return DISK_FAILURE;
}

int disk_get_block_len() {
  int n = 2 * sizeof( uint32_t ); uint8_t x[ n ];

  for( int i = 0; i < DISK_RETRY; i++ ) {
      PL011_puth( UART2, 0x00, true );        // write command
      PL011_putc( UART2, '\n', true );        // write EOL

    if( PL011_geth( UART2, true ) == 0x00 ) { // read  command
      PL011_getc( UART2,       true );        // read  separator
       data_geth( UART2, x, n, true );        // read  data
      PL011_getc( UART2,       true );        // read  EOL

      return ( ( uint32_t )( x[ 4 ] ) <<  0 ) |
             ( ( uint32_t )( x[ 5 ] ) <<  8 ) |
             ( ( uint32_t )( x[ 6 ] ) << 16 ) |
             ( ( uint32_t )( x[ 7 ] ) << 24 ) ;
    }
    else {
      PL011_getc( UART2,       true );        // read  EOL
    }
  }

  return DISK_FAILURE;
}

int disk_wr( uint32_t a, const uint8_t* x, int n ) {
  for( int i = 0; i < DISK_RETRY; i++ ) {
      PL011_puth( UART2, 0x01, true );        // write command
      PL011_putc( UART2, ' ',  true );        // write separator
       addr_puth( UART2, a,    true );        // write address
      PL011_putc( UART2, ' ',  true );        // write separator
       data_puth( UART2, x, n, true );        // write data
      PL011_putc( UART2, '\n', true );        // write EOL
  
    if( PL011_geth( UART2, true ) == 0x00 ) { // read  command
      PL011_getc( UART2,       true );        // read  EOL  

      return DISK_SUCCESS;
    }
    else {
      PL011_getc( UART2,       true );        // read  EOL
    }
  }
  
  return DISK_FAILURE;
}

int disk_rd( uint32_t a,       uint8_t* x, int n ) {
  for( int i = 0; i < DISK_RETRY; i++ ) {
      PL011_puth( UART2, 0x02, true );        // write command
      PL011_putc( UART2, ' ',  true );        // write separator
       addr_puth( UART2, a,    true );        // write address
      PL011_putc( UART2, '\n', true );        // write EOL
  
    if( PL011_geth( UART2, true ) == 0x00 ) { // read  command
      PL011_getc( UART2,       true );        // read  separator
       data_geth( UART2, x, n, true );        // read  data
      PL011_getc( UART2,       true );        // read  EOL

      return DISK_SUCCESS;
    }
    else {
      PL011_getc( UART2,       true );        // read  EOL
    }
  }

  return DISK_FAILURE;
}
