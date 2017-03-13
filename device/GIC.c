#include "GIC.h"

volatile GICC_t* GICC0 = ( volatile GICC_t* )( 0x1E000000 );
volatile GICD_t* GICD0 = ( volatile GICD_t* )( 0x1E001000 );
volatile GICC_t* GICC1 = ( volatile GICC_t* )( 0x1E010000 );
volatile GICD_t* GICD1 = ( volatile GICD_t* )( 0x1E011000 );
volatile GICC_t* GICC2 = ( volatile GICC_t* )( 0x1E020000 );
volatile GICD_t* GICD2 = ( volatile GICD_t* )( 0x1E021000 );
volatile GICC_t* GICC3 = ( volatile GICC_t* )( 0x1E030000 );
volatile GICD_t* GICD3 = ( volatile GICD_t* )( 0x1E031000 );
