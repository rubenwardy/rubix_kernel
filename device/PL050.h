#ifndef __PL050_H
#define __PL050_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RSVD(x,y,z) uint8_t reserved##x[ z - y + 1 ];

/* The ARM PrimeCell PS2 Keyboard/Mouse Interface (PL050) is documented at
 *
 * http://infocenter.arm.com/help/topic/com.arm.doc.ddi0143c/index.html
 * 
 * In particular, Section 3 explains the programmer's model, i.e., how to 
 * interact with it: this includes 
 * 
 * - Section 3.2, which summarises the device register layout in Table 3.1
 *   (including an offset from the device base address, in the memory map,
 *   for each register), and
 * - Section 3.3, which summarises the internal structure of each device
 *   register.
 * 
 * Note that the field identifiers used here follow the documentation in a
 * general sense, but with a some minor alterations to improve clarity and
 * consistency.
 */

typedef volatile struct {
  uint32_t CR;               // 0x0000          : control
  uint32_t STAT;             // 0x0004          : status
  uint32_t DATA;             // 0x0008          : data
  uint32_t CLKDIV;           // 0x000C          : clock divisor
  uint32_t IR;               // 0x0010          : interrupt status
  RSVD( 0, 0x0008, 0x0017 ); // 0x0014...0x003C : reserved
  RSVD( 1, 0x001C, 0x001F ); // 0x0040...0x009C : reserved
  RSVD( 2, 0x004C, 0x0FDF ); // 0x00A0...0x00FF : reserved
} PL050_t;

/* Per Table 4.2 (for example: the information is in several places) of
 * 
 * http://infocenter.arm.com/help/topic/com.arm.doc.dui0417d/index.html
 * 
 * we know the registers are mapped to fixed addresses in memory, so we
 * can just define a (structured) pointer to each one to support access.
 */

extern volatile PL050_t* PS20; // keyboard
extern volatile PL050_t* PS21; //    mouse

// transmit raw      byte x via PL050 instance d
extern void    PL050_putc( PL050_t* d, uint8_t x );
// recieve  raw      byte r via PL050 instance d
extern uint8_t PL050_getc( PL050_t* d            );

#endif
