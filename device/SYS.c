#include "SYS.h"

volatile SYSCONF_t* SYSCONF  = ( volatile SYSCONF_t* )( 0x10000000 );

volatile  uint32_t* SYSCTRL0 = ( volatile  uint32_t* )( 0x10001000 );
volatile  uint32_t* SYSCTRL1 = ( volatile  uint32_t* )( 0x1001A000 );
